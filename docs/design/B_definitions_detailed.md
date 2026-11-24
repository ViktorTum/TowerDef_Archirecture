---
title: "Tower Defence Engine — Блок B. Definitions"
subtitle: "Подробное описание классов (namespace td::defs)"
---

# 0. Назначение блока и правила чтения

**Блок B (Definitions)** — модель конфигурации игры: всё, что разработчик описывает в редакторе и что хранится в файлах проекта. В терминологии документа это «типы» (reusable configuration definitions), а не экземпляры в сессии.

**Что блок делает:** хранит данные и выполняет *чистые* вычисления над ними (применить модификатор, посчитать итоговые параметры апгрейда, посчитать эффективное здоровье по пресету сложности, проверить условие открытия узла кампании).

**Что блок не делает:** не проверяет допустимость значений (блок C), не читает и не пишет файлы (блок D), не исполняет механики (блок E) и не знает о симуляции (блоки G–I).

**Зависит от:** блок A. **Статус:** согласован. Согласованные решения:

- Параметры башен, ловушек и эффектов хранятся в универсальной модели `ParameterSet` («имя → значение»), поверх которой даётся типизированный вид `TowerStats` / `TrapStats`.
- Допущения по пробелам документа (раздел 10).

## 0.1. Обозначения

Как в описании блока A: `+` public, `-` private, `{static}`, `{abstract}`, `{const}`. Связи: **композиция** (часть не живёт без целого), **агрегация**, **наследование**, **ассоциация** (использует). Колонка «UC» показывает, какие юзкейсы класс обслуживает.

## 0.2. Жизненный цикл объектов конфигурации

| Стадия | Состояние | Как выражено в коде |
|---|---|---|
| Редактирование | Объект изменяем, в реестр проекта не добавлен (UC-4.11: «остаётся в состоянии редактирования») | Обычный объект с методами-сеттерами, используется блоком L |
| Опубликован | Объект лежит в реестре проекта | `std::shared_ptr<const T>`: изменить через такую ссылку нельзя |
| Снимок уровня | Копия, взятая на момент запуска уровня | `LevelConfigSnapshot` (раздел 9), неизменяем |

Так мы не заводим отдельный класс «черновик» для каждого типа, но публикация даёт гарантию неизменяемости на уровне компилятора.

## 0.3. Доменные алиасы идентификаторов

Все — `ConfigId<Tag>` из блока A с собственной меткой:

| Алиас | Что называет |
|---|---|
| `StructureTypeId` | тип башни, ловушки или барьера |
| `EnemyTypeId` | тип врага |
| `DamageTypeId` | категория урона |
| `MapId` | карта (уровень) |
| `WaveId` | волна |
| `SpawnId`, `GoalId` | точка появления, цель |
| `AirRouteId` | воздушный маршрут |
| `UpgradeNodeId` | узел дерева апгрейдов |
| `UnlockId` | награда-разблокировка кампании |
| `PresetId` | пресет сложности |
| `CampaignId`, `CampaignNodeId` | кампания, узел сюжетной карты |
| `RewardId` | награда узла |
| `AssetId` | графический ассет |
| `ProjectId` | проект |

## 0.4. Состав блока (54 типа)

| Подпакет | Классы |
|---|---|
| B.1 Параметры | `ParameterType`\*, `ParameterValue`, `ParameterSet`, `ModifierOp`\*, `ParameterModifier`, `ValueRange`, `ParameterSpec`, `ParameterSchema` |
| B.2 Постройки | `StructureKind`\*, `PlacementZone`\*, `PlacementRules`, `StructureType`, `TowerType`, `TrapType`, `BarrierType`, `TowerStats`, `TrapStats` |
| B.3 Апгрейды | `UpgradeNode`, `UpgradeTree` |
| B.4 Враги и урон | `MovementType`\*, `EnemyType`, `GroundEnemyType`, `AirEnemyType`, `ShieldInteraction`\*, `StackingMode`\*, `EffectSpec`, `DamageType` |
| B.5 Карта | `SpawnPoint`, `GoalDefinition`, `SpawnGoalPair`, `AirRoute`, `WinCondition`\*, `LossCondition`\*, `LevelRules`, `MapDefinition` |
| B.6 Волны | `ReleaseMode`\*, `SpawnGroup`, `WaveDefinition`, `WaveSchedule` |
| B.7 Сложность | `DifficultyPreset` |
| B.8 Кампания | `UnlockKind`\*, `UnlockDefinition`, `ConditionMode`\*, `UnlockCondition`, `RewardMode`\*, `RewardDefinition`, `CampaignNode`, `StoryLink`, `CampaignDefinition` |
| B.9 Проект | `AssetDefinition`, `MechanicReference`, `ProjectConfig`, `GameProject`, `LevelConfigSnapshot` |

\* — перечисление (12 штук).

## 0.5. Карта связей (основное)

```
StructureType <|-- TowerType, TrapType, BarrierType
StructureType *-- PlacementRules, ParameterSet (baseParameters), UpgradeTree
UpgradeTree *-- UpgradeNode *-- ParameterModifier
TowerStats, TrapStats ..> ParameterSet            (типизированный вид)
ParameterSchema *-- ParameterSpec *-- ValueRange

EnemyType <|-- GroundEnemyType, AirEnemyType
DamageType *-- EffectSpec (0..1)

MapDefinition *-- SpawnPoint, GoalDefinition, AirRoute, SpawnGoalPair, LevelRules
WaveSchedule *-- WaveDefinition *-- SpawnGroup

CampaignDefinition *-- CampaignNode *-- UnlockCondition, RewardDefinition
CampaignDefinition *-- StoryLink

GameProject *-- ProjectConfig, реестры всех определений, MechanicReference
LevelConfigSnapshot o-- (копии нужных определений)
```

\newpage

# 1. B.1 Параметры: универсальная модель «имя → значение»

**Зачем этот подпакет.** Апгрейды (UC-5) меняют параметры башни *по имени* (`{param, op, value}`), а плагины (A.8, уровень 3) приносят механики со *своими* параметрами. Жёсткие поля класса не позволили бы ни того, ни другого без правки ядра. Поэтому параметры хранятся как именованный набор, а типизированный доступ дают виды `TowerStats` и `TrapStats` (раздел 2).

## 1.1. `ParameterType` (enum)

`Number` (вещественное), `Integer`, `Boolean`, `Text`. Нужен `ParameterSpec`, чтобы описать ожидаемый тип поля.

## 1.2. `ParameterValue`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-4.6, UC-5.3 |

```
- value_ : std::variant<Scalar, int64_t, bool, std::string>
+ ofNumber(v : Scalar) / ofInteger(v : int64_t) / ofBoolean(v : bool) / ofText(v : std::string)   {static}
+ type() : ParameterType                         {const}
+ asNumber() : std::optional<Scalar>             {const}   «целое преобразуется в число»
+ asInteger() : std::optional<int64_t>           {const}
+ asBoolean() : std::optional<bool>              {const}
+ asText() : std::optional<std::string>          {const}
+ operator==                                     {const}
```

**Почему так.** Id механик (`"projectile"`, `"aoe"`) хранятся как `Text`: так апгрейд может «выбрать другую механику» операцией `Set` (A.8, уровень 2).

## 1.3. `ParameterSet`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-4.3–4.8, UC-5.3, 5.6 |

**Ответственность.** Набор параметров «имя → значение». Неизменяемый: «изменение» создаёт копию.

```
- values_ : std::map<std::string, ParameterValue>      «упорядочен по имени ⇒ детерминированный обход и хеш»
+ has(name : std::string_view) : bool                  {const}
+ find(name : std::string_view) : const ParameterValue*  {const}   «nullptr, если нет»
+ number(name) : Result<Scalar>                        {const}   «ошибка: нет параметра или другой тип»
+ integer(name) : Result<int64_t>                      {const}
+ boolean(name) : Result<bool>                         {const}
+ text(name) : Result<std::string>                     {const}
+ with(name : std::string, value : ParameterValue) : ParameterSet   {const}
+ names() : std::vector<std::string>                   {const}
+ size() : size_t       + operator==                   {const}
```

## 1.4. `ModifierOp` (enum) и `ParameterModifier`

`ModifierOp`: `Add`, `Mul`, `Set` (UC-5.3, A.8).

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-5.3, 5.6 |

```
- param_ : std::string
- op_ : ModifierOp
- value_ : ParameterValue
+ ParameterModifier(param : std::string, op : ModifierOp, value : ParameterValue)
+ param() / op() / value()                              {const}
+ apply(source : ParameterSet) : Result<ParameterSet>   {const}
```

**Правила `apply`.**

- `Add` и `Mul` требуют, чтобы параметр существовал и был числовым; иначе ошибка `unknownParameter` или `typeMismatch` (UC-5.3: «referenced parameters must exist»).
- `Set` заменяет значение либо создаёт параметр. Это позволяет апгрейду, меняющему механику стрельбы, добавить недостающие параметры новой механики (UC-5.7).
- Модификаторы применяются последовательно в порядке списка (UC-5.6).

## 1.5. `ValueRange`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-4.7, 6.7, 15.3, A.6 |

**Ответственность.** Числовой интервал с флагами включения границ. Нужен потому, что в A.6 встречаются разные виды границ: «> 0» (открытая слева), «0.5..20» (замкнутый), «строго между 0 и 0.5» (открытый с двух сторон), «≥ 0».

```
- lo_, hi_ : std::optional<Scalar>        «нет значения = без ограничения»
- loInclusive_, hiInclusive_ : bool
+ closed(lo, hi) / open(lo, hi) / atLeast(lo) / greaterThan(lo) / atMost(hi)   {static}
+ contains(value : Scalar) : bool         {const}
+ describe() : std::string                {const}   «"(0, 0.5)" — текст для сообщения об ошибке»
```

## 1.6. `ParameterSpec`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-4.6–4.7, UC-5.7, UC-6.7 |

```
- name_ : std::string
- type_ : ParameterType
- range_ : std::optional<ValueRange>       «для числовых»
- allowedTexts_ : std::vector<std::string>  «для перечислимых: refresh|stack, first|last|…»
- required_ : bool
- defaultValue_ : std::optional<ParameterValue>
+ accessors                                {const}
```

## 1.7. `ParameterSchema`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-4.6, UC-5.7, A.8 |

**Ответственность.** Набор `ParameterSpec`. Схему объявляет базовый вид постройки (таблица A.6) и *каждая механика* (A.8: «registered by id with parameter schema»). Итоговая схема башни — объединение базовой и схем выбранных механик. Саму проверку `ParameterSet` по схеме выполняет блок C.

```
- specs_ : std::map<std::string, ParameterSpec>   «композиция»
+ add(spec : ParameterSpec) : Result<void>         «ошибка при дубликате имени»
+ find(name : std::string_view) : const ParameterSpec*   {const}
+ merge(other : ParameterSchema) : Result<ParameterSchema> {const}   «конфликт типов или диапазонов — ошибка»
+ specs() : std::vector<ParameterSpec>             {const}
+ requiredNames() : std::vector<std::string>       {const}
```

\newpage

# 2. B.2 Постройки

**Идея.** Башня, ловушка и барьер — три вида *построек* (в документе всё лежит в одном «tower registry», вид задаётся `structureKind`). Общее вынесено в абстрактный `StructureType`, различия выражены наследованием.

## 2.1. `StructureKind` и `PlacementZone` (enum)

`StructureKind`: `Tower`, `Trap`, `Barrier`. `PlacementZone`: `BuildZone`, `Road`.

## 2.2. `PlacementRules`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-3.5, 3.7–3.9, UC-4.8–4.9, UC-14.2, 14.6, A.5 |

**Ответственность.** Где можно строить и как постройка влияет на движение (блок `placement` из A.6).

```
- allowedZones_ : std::set<PlacementZone>
- blocksGroundMovement_ : bool = false
- destructible_ : bool = false
- health_ : std::optional<Scalar>             «обязательно, если destructible»
+ allows(zone : PlacementZone) : bool         {const}
+ blocksMovement() : bool                     {const}
+ isDestructibleBlocker() : bool              {const}   «blocks && destructible: остаётся в «потенциальном» графе (UC-14.6)»
+ isIndestructibleBlocker() : bool            {const}   «исключается и из потенциального графа»
+ canBeBuiltOnRoad() : bool                   {const}   «Road ∈ allowedZones»
+ accessors / setters (для редактора)
```

**Почему так.** Запросы `isDestructibleBlocker` и `isIndestructibleBlocker` инкапсулируют различие, на котором построен алгоритм маршрутов (A.5): блок F не разбирает флаги вручную.

## 2.3. `StructureType` (абстрактный)

| | |
|---|---|
| **Вид** | абстрактный класс |
| **UC** | UC-4 (все шаги), UC-3.4–3.6, 3.13 |

```
- id_ : StructureTypeId
- cost_ : int64_t                                     «>= 0»
- placement_ : PlacementRules                         «композиция»
- sellable_ : bool = true
- sellRefund_ : int64_t = 0                           «0..cost»
- graphicAssetId_ : std::optional<AssetId>
- baseParameters_ : ParameterSet                      «композиция»
- upgrades_ : UpgradeTree                             «композиция»
+ kind() : StructureKind                              {abstract, const}
+ id() / cost() / placement() / sellable() / sellRefund() / graphicAssetId() / baseParameters() / upgrades()   {const}
+ setters (для редактора, в опубликованном объекте недоступны через const-ссылку)
```

## 2.4. `TowerType`, `TrapType`, `BarrierType`

Наследники `StructureType`.

| Класс | `kind()` | Что отличает | UC |
|---|---|---|---|
| `TowerType` | `Tower` | Параметры: damage, damageType, range, attackSpeed, fireMechanic, damageMechanic, targeting и параметры механик. Метод `baseStats() : Result<TowerStats>`. Только башня ищет цели и стреляет | 4.3 |
| `TrapType` | `Trap` | Параметры: damage, damageType, damageMechanic, activationMechanic (`onEnter`). Нет range и attackSpeed. Метод `baseStats() : Result<TrapStats>` | 4.4 |
| `BarrierType` | `Barrier` | Нет параметров урона и стрельбы. Здоровье задаётся в `placement` | 4.5 |

**Почему наследование.** Требования «только башня стреляет», «у барьера нет урона» выражены типами: у `BarrierType` просто нет методов `baseStats()` и полей урона. Тип постройки определяет и то, какие команды ей применимы (UC-3.14: targeting и enable/disable только для башни).

## 2.5. `TowerStats` и `TrapStats`

| | |
|---|---|
| **Вид** | value object — типизированный «вид» на `ParameterSet` |
| **UC** | UC-1.11–1.14, UC-4, UC-5.6–5.7 |

**Зачем.** Игра читает `damage` и `range` как числа с типами, а не по строковым именам. Этот же вид строится и для базовых параметров башни, и для параметров *после апгрейдов*, поэтому код симуляции не различает «базовая» и «улучшенная» башню.

```
TowerStats
- parameters_ : ParameterSet
+ from(parameters : ParameterSet) : Result<TowerStats>    {static}   «ошибка, если обязательные поля отсутствуют»
+ damage() : Scalar                         {const}
+ damageTypeId() : DamageTypeId             {const}
+ range() : Scalar                          {const}
+ attackSpeed() : Scalar                    {const}
+ fireMechanicId() : MechanicId             {const}
+ damageMechanicId() : MechanicId           {const}
+ targetingId() : MechanicId                {const}
+ attackIntervalTicks(dt : FixedTimeStep) : Tick   {const}   «intervalToTicks(1 / attackSpeed)»
+ parameters() : const ParameterSet&        {const}   «для механик со своими параметрами (projectileSpeed, aoeRadius…)»

TrapStats
+ from(parameters : ParameterSet) : Result<TrapStats>   {static}
+ damage() / damageTypeId() / damageMechanicId() / activationMechanicId() / parameters()   {const}
```

\newpage

# 3. B.3 Апгрейды

## 3.1. `UpgradeNode`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-5.2–5.5, UC-3.11–3.12 |

```
- nodeId_ : UpgradeNodeId
- parentId_ : std::optional<UpgradeNodeId>      «нет у первых узлов — родитель корень»
- price_ : int64_t                              «>= 0»
- modifiers_ : std::vector<ParameterModifier>   «порядок существенен»
- unlockId_ : std::optional<UnlockId>           «нет = доступен всегда, когда выполнены родитель и цена»
+ accessors                                     {const}
```

## 3.2. `UpgradeTree`

| | |
|---|---|
| **Вид** | класс-коллекция |
| **UC** | UC-5.1–5.6, UC-3.11–3.12 |

**Ответственность.** Дерево апгрейдов одной постройки с неявным корнем «базовое состояние» (UC-5.1). Вычисляет итоговые параметры и знает структуру веток.

```
- nodes_ : std::map<UpgradeNodeId, UpgradeNode>     «упорядочен по id ⇒ детерминизм»
+ rootId() : UpgradeNodeId                          {static}   «фиксированный id корня»
+ addNode(node : UpgradeNode) : Result<void>        «ошибка при дубликате id; циклы и «висячие» узлы ищет блок C»
+ find(id : UpgradeNodeId) : const UpgradeNode*     {const}
+ children(id : UpgradeNodeId) : std::vector<UpgradeNodeId>  {const}
+ siblings(id : UpgradeNodeId) : std::vector<UpgradeNodeId>  {const}   «взаимоисключающие альтернативы (UC-5.4)»
+ isDirectChild(parent, child : UpgradeNodeId) : bool        {const}   «проверка «следующий узел — прямой потомок» (UC-3.11)»
+ path(id : UpgradeNodeId) : Result<std::vector<UpgradeNodeId>> {const}   «от корня до узла»
+ resolve(base : ParameterSet, id : UpgradeNodeId) : Result<ParameterSet> {const}
+ size() : size_t                                   {const}
```

**Как работает `resolve`.** Берёт путь от корня, последовательно применяет модификаторы каждого узла (внутри узла — в порядке списка) к базовым параметрам (UC-5.6). Ошибка любого шага возвращается наверх: частично применённого результата не бывает.

**Почему здесь.** «Братья ли эти узлы?» и «каков итог?» — вопросы самого дерева (Information Expert). Это снимает логику ветвления с блока команд H.

\newpage

# 4. B.4 Враги и урон

## 4.1. `MovementType`, `ShieldInteraction`, `StackingMode` (enum)

`MovementType`: `Ground`, `Air`. `ShieldInteraction`: `Normal`, `Ignore`, `Double` (A.2). `StackingMode`: `Refresh`, `Stack` (A.2, DoT).

## 4.2. `EnemyType` (абстрактный)

| | |
|---|---|
| **Вид** | абстрактный класс |
| **UC** | UC-6.1–6.7, UC-1, UC-14 |

```
- id_ : EnemyTypeId
- health_ : int64_t                                       «>= 1»
- shield_ : int64_t                                       «>= 0»
- speed_ : Scalar                                         «0.1..10 тайлов/с»
- collisionRadius_ : Scalar                               «(0, 0.5) тайла»
- resistances_ : std::map<DamageTypeId, Scalar>           «[-1, 1]; отсутствие = defaultResistance»
- reward_ : int64_t                                       «>= 0»
- goalDamage_ : int64_t                                   «>= 1»
- graphicAssetId_ : std::optional<AssetId>
+ movementType() : MovementType                           {abstract, const}
+ resistanceTo(damageType : DamageType) : Scalar          {const}   «свой override либо defaultResistance (UC-6.3)»
+ collisionShape(position : Vec2) : Circle                {const}   «круг для столкновений»
+ accessors / setters
```

## 4.3. `GroundEnemyType`, `AirEnemyType`

| Класс | `movementType()` | Дополнительно | UC |
|---|---|---|---|
| `GroundEnemyType` | `Ground` | `obstacleAttackDamage` (> 0), `obstacleAttackInterval` (> 0 с); метод `obstacleAttackIntervalTicks(dt)` | 6.5, 14.8 |
| `AirEnemyType` | `Air` | нет параметров атаки препятствий | 6.4 |

**Почему наследование.** Требование A.6 «`obstacleAttack*` обязательны для ground» выражено типом: у воздушного врага этих полей просто нет, и ошибку «забыли указать» допустить нельзя. В отличие от `if (movementType == ground)` по всему коду.

## 4.4. `EffectSpec`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-1.4, UC-1.14, A.2 |

**Ответственность.** Описание статуса «при попадании» (burn, slow).

```
- effectId_ : MechanicId                       «реализация IEffect из блока E»
- durationSeconds_ : Scalar
- tickIntervalSeconds_ : std::optional<Scalar> «для периодических эффектов»
- stacking_ : StackingMode
- parameters_ : ParameterSet                   «параметры конкретной реализации (процент замедления и т.п.)»
+ accessors                                    {const}
```

## 4.5. `DamageType`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-1.14, UC-6.3, A.2, A.4 |

```
- id_ : DamageTypeId
- defaultResistance_ : Scalar                  «[-1, 1]»
- shieldInteraction_ : ShieldInteraction
- effect_ : std::optional<EffectSpec>          «композиция»
+ accessors                                    {const}
```

\newpage

# 5. B.5 Карта

**Граница ответственности.** `MapDefinition` — это то, что записано в файле карты: дискретные клетки и маркеры. Граф дорог, соседство и поиск пути строит блок F *по* этому определению.

## 5.1. `SpawnPoint`, `GoalDefinition`, `SpawnGoalPair`

| Класс | Поля | UC |
|---|---|---|
| `SpawnPoint` | `spawnId : SpawnId`, `cell : HexCoord` | UC-7.3 |
| `GoalDefinition` | `goalId : GoalId`, `cell : HexCoord`, `startingHealth : int64_t (>= 1)` | UC-7.3, 7.8 |
| `SpawnGoalPair` | `spawnId`, `goalId`; сравнение `==`, `<` | UC-7.9, UC-14 |

## 5.2. `AirRoute`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-7.6, 7.9–7.10, UC-8.3, UC-14.1 |

```
- routeId_ : AirRouteId
- spawnId_ : SpawnId
- goalId_ : GoalId
- waypoints_ : std::vector<Vec2>         «упорядоченная цепочка непрерывных точек»
+ start() : Vec2   + end() : Vec2        {const}   «первая и последняя точки»
+ accessors                              {const}
```

## 5.3. `WinCondition`, `LossCondition` (enum) и `LevelRules`

`WinCondition`: `WavesCleared`, `SurviveUntilTick`. `LossCondition`: `AnyGoalDestroyed`, `AllGoalsDestroyed` (UC-7.8, A.6).

```
LevelRules
- winCondition_ : WinCondition
- lossCondition_ : LossCondition
- targetTick_ : std::optional<Tick>     «обязателен при SurviveUntilTick, >= 1»
+ accessors                             {const}
```

## 5.4. `MapDefinition`

| | |
|---|---|
| **Вид** | класс-агрегат |
| **UC** | UC-7 (все шаги), UC-8.9, UC-3.5, UC-14 |

```
- mapId_ : MapId
- width_, height_ : int32_t                          «> 0»
- roadCells_ : std::set<HexCoord>                    «проходимые клетки дорог (результат «кисти»)»
- buildZoneCells_ : std::set<HexCoord>
- spawns_ : Registry<SpawnId, SpawnPoint>            «композиция»
- goals_ : Registry<GoalId, GoalDefinition>
- airRoutes_ : Registry<AirRouteId, AirRoute>
- groundPairs_ : std::set<SpawnGoalPair>
- rules_ : LevelRules
- allowedPresetIds_ : std::set<PresetId>             «UC-8.9, UC-15.5»
+ id() / width() / height() / rules() / ...          {const}
+ isRoad(cell : HexCoord) : bool                     {const}
+ isBuildZone(cell : HexCoord) : bool                {const}
+ zoneOf(cell : HexCoord) : std::optional<PlacementZone> {const}
+ isInside(cell : HexCoord) : bool                   {const}   «внутри width × height»
+ findSpawn / findGoal / findAirRoute                {const}
+ allowsPreset(id : PresetId) : bool                 {const}
+ setters (для редактора)
```

**Почему так.** Дороги и зоны хранятся как множества клеток (как их и рисует редактор: кисть закрашивает клетки). Преобразование в граф дорог — ответственность блока F, а не данных.

\newpage

# 6. B.6 Волны

## 6.1. `ReleaseMode` (enum) и `SpawnGroup`

`ReleaseMode`: `Automatic`, `Manual` (UC-8.5–8.6).

```
SpawnGroup
- enemyId_ : EnemyTypeId
- count_ : int64_t                      «>= 1»
- intervalSeconds_ : Scalar             «> 0»
- spawnId_ : SpawnId
- goalId_ : GoalId
- airRouteId_ : std::optional<AirRouteId>   «обязателен для воздушной группы (UC-8.3)»
+ accessors                             {const}
```

## 6.2. `WaveDefinition`

```
- waveId_ : WaveId
- releaseMode_ : ReleaseMode
- delaySeconds_ : Scalar                «>= 0; для Automatic: после окончания спавна предыдущей, для первой — от старта уровня»
- groups_ : std::vector<SpawnGroup>     «порядок существенен: влияет на порядок спавна в одном тике (UC-8.8)»
+ accessors                             {const}
```

## 6.3. `WaveSchedule`

| | |
|---|---|
| **Вид** | класс-коллекция |
| **UC** | UC-8.1, 8.8, 8.10–8.11, UC-1.3, UC-3.15 |

```
- mapId_ : MapId
- waves_ : std::vector<WaveDefinition>              «порядок = порядок волн (UC-8.1)»
+ mapId() : MapId                                   {const}
+ addWave(wave : WaveDefinition) : Result<void>     «ошибка при дубликате waveId»
+ waves() : const std::vector<WaveDefinition>&      {const}
+ find(id : WaveId) : const WaveDefinition*         {const}
+ indexOf(id : WaveId) : std::optional<size_t>      {const}
+ size() : size_t                                   {const}
```

**Почему отдельно от карты.** UC-8.11: расписание «хранится независимо от кода движка» и привязано к карте по `mapId`; одна карта может иметь разные расписания в будущих расширениях, а правка волн не трогает файл карты.

# 7. B.7 Сложность

## 7.1. `DifficultyPreset`

| | |
|---|---|
| **Вид** | value object с чистыми методами расчёта |
| **UC** | UC-15.1–15.11, UC-11.2, 11.4, UC-8.9 |

```
- presetId_ : PresetId
- name_ : std::string
- enemyHealthMultiplier_ : Scalar          «finite, > 0»
- enemyCountMultiplier_ : Scalar
- spawnIntervalMultiplier_ : Scalar
- startingCoinsMultiplier_ : Scalar
+ effectiveHealth(baseHealth : int64_t) : int64_t            {const}   «max(1, ceil(base × m))»
+ effectiveCount(baseCount : int64_t) : int64_t              {const}   «max(1, ceil(base × m))»
+ startingCoins(baseCoins : int64_t) : int64_t               {const}   «floor(base × m)»
+ spawnIntervalTicks(baseSeconds : Scalar, dt : FixedTimeStep) : Tick {const}   «max(1, ceil(base × m / dt))»
+ accessors
```

**Почему формулы здесь.** UC-15.7–15.9 задают точные формулы. Они зависят только от данных пресета, поэтому живут рядом с ним (Information Expert), а не в движке. Округление выполняется через `NumericUtils::ceilTolerant` / `floorTolerant`, иначе, например, `floor(0.29 × 100)` в двоичном double даёт 28 вместо 29.

\newpage

# 8. B.8 Кампания

## 8.1. `UnlockKind` (enum) и `UnlockDefinition`

`UnlockKind`: `Tower`, `Upgrade`.

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-5.5, UC-9.6–9.8, UC-16.7–16.10, UC-3.4 |

```
- unlockId_ : UnlockId
- kind_ : UnlockKind
- towerTypeId_ : std::optional<StructureTypeId>    «обязателен, если kind = Tower»
+ accessors                                         {const}
```

**Как связаны разблокировки с содержимым.**

- `Tower`: определение указывает, *какую постройку* открывает награда. Постройка, на которую не ссылается ни один `UnlockDefinition`, доступна всегда (допущение 2, раздел 10).
- `Upgrade`: связь идёт в обратную сторону. Узел апгрейда содержит `unlockId` (UC-5.5), поэтому определению цель не нужна.

## 8.2. `ConditionMode` (enum) и `UnlockCondition`

`ConditionMode`: `AnyOf`, `AllOf` (UC-9.5).

```
UnlockCondition
- mode_ : ConditionMode
- requiredNodes_ : std::set<CampaignNodeId>
+ isSatisfied(completed : std::set<CampaignNodeId>) : bool   {const}
+ requiredNodes() : const std::set<CampaignNodeId>&          {const}
```

`AllOf` с пустым списком всегда истинно (используется стартовым узлом); `AnyOf` с пустым списком всегда ложно. Условие «хотя бы один пройденный предшественник по направленной связи» (UC-16.2) относится к состоянию кампании и проверяется в блоке J.

## 8.3. `RewardMode` (enum) и `RewardDefinition`

`RewardMode`: `Fixed`, `Choice` (UC-9.7).

```
RewardDefinition
- rewardId_ : RewardId
- mode_ : RewardMode
- options_ : std::vector<UnlockId>       «Fixed: ровно один; Choice: явный перечень»
+ isChoice() : bool                      {const}
+ accessors                              {const}
```

## 8.4. `CampaignNode`, `StoryLink`

| Класс | Поля | UC |
|---|---|---|
| `CampaignNode` | `levelNodeId : CampaignNodeId`, `mapId : MapId`, `position : Vec2` (позиция на сюжетной карте), `condition : UnlockCondition`, `rewards : std::vector<RewardDefinition>` | UC-9.2, 9.5–9.7 |
| `StoryLink` | `from : CampaignNodeId`, `to : CampaignNodeId` | UC-9.3 |

## 8.5. `CampaignDefinition`

| | |
|---|---|
| **Вид** | класс-агрегат |
| **UC** | UC-9, UC-11.1, UC-12.7, UC-16 |

```
- campaignId_ : CampaignId
- name_ : std::string
- startNodeId_ : CampaignNodeId
- nodes_ : Registry<CampaignNodeId, CampaignNode>      «композиция»
- links_ : std::vector<StoryLink>
- allowedPresetIds_ : std::set<PresetId>               «UC-15.5: пресеты можно назначить карте ИЛИ кампании; список карты имеет приоритет»
+ find(id : CampaignNodeId) : const CampaignNode*      {const}
+ successors(id : CampaignNodeId) : std::vector<CampaignNodeId>     {const}
+ predecessors(id : CampaignNodeId) : std::vector<CampaignNodeId>   {const}
+ hasLink(from, to : CampaignNodeId) : bool            {const}
+ nodes() / links() / startNodeId() / allowedPresetIds()  {const}
```

**Почему так.** Запросы `successors` и `predecessors` нужны и валидатору (достижимость, циклы), и блоку J (какие узлы открыть после победы): граф знает свои связи сам.

\newpage

# 9. B.9 Проект и снимок конфигурации уровня

## 9.1. `AssetDefinition`

`assetId : AssetId`, `relativePath : std::string` — запись реестра ассетов проекта (UC-4.10, UC-6.6). Неизвестный `assetId` проверяет блок C.

## 9.2. `MechanicReference`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-2.5, UC-13.1, UC-17.1, A.8 |

```
- mechanicId_ : MechanicId
- version_ : Version
+ accessors / operator== / operator<     {const}
```

**Зачем.** Проект перечисляет, какие механики ему нужны. Та же пара «id + версия» записывается в реплей, и при воспроизведении версия должна совпасть точно (UC-13.1).

## 9.3. `ProjectConfig`

| | |
|---|---|
| **Вид** | value object |
| **UC** | UC-2.1–2.6, UC-15.5 |

```
- projectId_ : ProjectId
- displayName_ : std::string
- schemaVersion_ : Version
- timeStep_ : FixedTimeStep                 «композиция»
- baseStartingCoins_ : int64_t              «>= 0»
- defaultPresetId_ : std::optional<PresetId>  «задаётся после создания пресетов (UC-15.5)»
+ accessors / setters
```

## 9.4. `GameProject`

| | |
|---|---|
| **Вид** | класс-агрегат, корень всей конфигурации |
| **UC** | UC-2.5, UC-4–9, UC-15, UC-10.2 |

```
- config_ : ProjectConfig
- structures_ : Registry<StructureTypeId, std::shared_ptr<const StructureType>>
- enemies_ : Registry<EnemyTypeId, std::shared_ptr<const EnemyType>>
- damageTypes_ : Registry<DamageTypeId, DamageType>
- maps_ : Registry<MapId, MapDefinition>
- waveSchedules_ : Registry<MapId, WaveSchedule>
- presets_ : Registry<PresetId, DifficultyPreset>
- campaigns_ : Registry<CampaignId, CampaignDefinition>
- unlocks_ : Registry<UnlockId, UnlockDefinition>
- assets_ : Registry<AssetId, AssetDefinition>
- requiredMechanics_ : std::vector<MechanicReference>
+ config() / structures() / enemies() / damageTypes() / maps() / waveSchedules() / presets() / campaigns() / unlocks() / assets() / requiredMechanics()   {const}
+ mutable access для редактора (блок L)
```

**Почему так.** Реестр — по одному на вид определений, как в UC-2.5. Постройки и враги хранятся как `shared_ptr<const T>` (полиморфные неизменяемые объекты, см. 0.2).

## 9.5. `LevelConfigSnapshot`

| | |
|---|---|
| **Вид** | неизменяемый value object |
| **UC** | UC-11.3–11.5, UC-13.3, UC-17.1–17.2, UC-15.10 |

**Ответственность.** Полная копия конфигурации, нужной для *одного прогона уровня*. Движок (блок I) читает снимок, а не проект: проект можно править, прогон от этого не меняется.

```
- timeStep_ : FixedTimeStep
- startingCoins_ : int64_t                         «эффективное: уже с учётом пресета»
- requiredMechanics_ : std::vector<MechanicReference>
- structures_ : Registry<StructureTypeId, std::shared_ptr<const StructureType>>
- enemies_ : Registry<EnemyTypeId, std::shared_ptr<const EnemyType>>
- damageTypes_ : Registry<DamageTypeId, DamageType>
- map_ : MapDefinition                             «копия»
- waves_ : WaveSchedule                            «копия»
- appliedPreset_ : DifficultyPreset                «копия: правка пресета позже не изменит старую запись (UC-15.10)»
- unlocks_ : Registry<UnlockId, UnlockDefinition>
+ accessors                                        {const}
```

**Почему так.**

- UC-17.2: реплей не должен ссылаться на редактируемые файлы проекта: снимок и есть самодостаточный источник.
- Сборку снимка из `GameProject` выполняет блок J (`LevelConfigBuilder`); сериализацию — блок D.

\newpage

# 10. Допущения и пробелы документа (принятые решения)

| № | Пробел в юзкейсах | Принятое решение | Что исправить в документе |
|---|---|---|---|
| 1 | UC-2.5 не перечисляет реестр наград, но UC-5.5 и UC-9.8 ссылаются на «campaign rewards registry» | Добавлены `UnlockDefinition` и реестр `unlocks` | Добавить реестр наград в список UC-2.5 |
| 2 | UC-3.4 говорит «башня заблокирована кампанией», но не определяет, как башня становится заблокированной | Башня, на которую не ссылается ни один `UnlockDefinition`, доступна всегда | Явно описать правило в UC-3.4 или UC-9.6 |
| 3 | UC-2.5 упоминает реестр «mechanics», но механики — это код | Проект хранит список требуемых механик (`MechanicReference`: id + версия); реализации лежат в блоке E | Уточнить формулировку UC-2.5 |
| 4 | UC-5.2: «unique nodeId» — неясно, уникален ли id в пределах башни или проекта | Принято: уникален в пределах дерева одной башни; награда типа `Upgrade` ссылается на узел через `unlockId` в самом узле | Уточнить область уникальности в UC-5.2 |

# 11. Сводные таблицы

## 11.1. Покрытие юзкейсов блоком B

| UC | Что даёт блок B |
|---|---|
| UC-1 | `TowerStats`, `TrapStats`, `EnemyType`, `DamageType`, `EffectSpec`, `WaveSchedule`, `LevelRules` |
| UC-2 | `ProjectConfig`, `GameProject`, `MechanicReference` |
| UC-3 | `StructureType`, `PlacementRules`, `UpgradeTree`, `UnlockDefinition`, `WaveSchedule` (manual start) |
| UC-4 | `StructureType`, `TowerType`, `TrapType`, `BarrierType`, `PlacementRules`, `ParameterSet`, `ParameterSchema`, `AssetDefinition` |
| UC-5 | `UpgradeNode`, `UpgradeTree`, `ParameterModifier`, `ModifierOp` |
| UC-6 | `EnemyType`, `GroundEnemyType`, `AirEnemyType`, `DamageType` |
| UC-7 | `MapDefinition`, `SpawnPoint`, `GoalDefinition`, `SpawnGoalPair`, `AirRoute`, `LevelRules` |
| UC-8 | `WaveSchedule`, `WaveDefinition`, `SpawnGroup`, `ReleaseMode` |
| UC-9 | `CampaignDefinition`, `CampaignNode`, `StoryLink`, `UnlockCondition`, `RewardDefinition` |
| UC-10 | `GameProject` (проверяется блоком C), `LevelConfigSnapshot` |
| UC-11 | `LevelConfigSnapshot`, `DifficultyPreset`, `CampaignDefinition` |
| UC-12 | `CampaignDefinition`, `UnlockCondition` |
| UC-13 | `LevelConfigSnapshot`, `MechanicReference` |
| UC-14 | `PlacementRules` (запросы о блокировке), `MapDefinition`, `AirRoute`, `GroundEnemyType` |
| UC-15 | `DifficultyPreset`, `ProjectConfig.defaultPresetId`, `MapDefinition.allowedPresetIds` |
| UC-16 | `CampaignDefinition`, `UnlockCondition`, `RewardDefinition`, `UnlockDefinition` |
| UC-17 | `LevelConfigSnapshot`, `MechanicReference` |

## 11.2. Что намеренно не входит в блок B

| Не входит | Где будет |
|---|---|
| Проверка диапазонов и ссылок (A.6) | Блок C |
| JSON-сериализация | Блок D |
| Реализации механик, каталог типов событий | Блок E |
| Граф дорог, поиск пути, маршруты | Блок F |
| Экземпляры башен, врагов, снарядов | Блок G |
| Сборка `LevelConfigSnapshot` из проекта | Блок J |

## 11.3. Дополнение к блоку B (после согласования)

UC-15.5 разрешает назначать `allowedPresetIds` карте *или кампании*, а в первом варианте блока B список был только у карты. В `CampaignDefinition` добавлено поле `allowedPresetIds`. Правило выбора (предлагается): список карты имеет приоритет; если он пуст, берётся список кампании; если пусты оба — ошибка валидации (блок C).

## 11.4. Дополнение к блоку A

При проработке `DifficultyPreset` обнаружено, что в `NumericUtils` (блок A) нужен метод `floorTolerant` — парный к `ceilTolerant`. Формула UC-15.8 требует `floor(baseStartingCoins × multiplier)`, а в double `0.29 × 100 = 28.999999999999996`; без допуска результат был бы 28 вместо 29. Блок A обновлён: `NumericUtils` получил метод `floorTolerant`.
