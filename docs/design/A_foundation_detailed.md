---
title: "Tower Defence Engine — Блок A. Foundation"
subtitle: "Подробное описание классов (namespace td::core)"
---

# 0. Назначение блока и правила чтения

**Блок A (Foundation)** — самый нижний слой. Он содержит базовые типы, из которых строятся все остальные блоки: идентификаторы, время, числа, геометрия, гекс-координаты, случайность, версии, хеши, ошибки, реестр и конверт события.

**Что блок не делает:** ничего не знает о башнях, врагах, картах, Godot и файловой системе. Зависит только от стандартной библиотеки C++ (C++17 и выше).

**Статус:** согласован с заказчиком. Решения по блоку: `ConfigId<Tag>` оставляем шаблоном; `overlapsCell` остаётся внутри `HexLayout`; числовой тип изолирован алиасом `Scalar` (вопрос double / fixed-point остаётся открытым).

## 0.1. Обозначения

Члены классов записаны в UML-нотации для переноса в Visual Paradigm:

- `+` public, `-` private, `#` protected; `{static}` — статический член; `{abstract}` — абстрактная операция; `{const}` — не меняет состояние.
- Связи: **композиция** (часть не живёт без целого), **агрегация**, **ассоциация / зависимость** (использует), **реализация** (класс реализует интерфейс), **шаблонная привязка** (alias).
- Колонка «UC» показывает, какие юзкейсы класс обслуживает.

## 0.2. Принципы, которые блок обеспечивает остальным

| Принцип | Как проявляется в блоке A |
|---|---|
| Инкапсуляция инвариантов | Невалидный `ConfigId`, `FixedTimeStep` или `Version` создать нельзя: конструкторы закрыты, создание через фабрики, возвращающие `Result` |
| Неизменяемость (immutability) | Все value objects неизменяемы; изменение = новый объект. Это снимает целый класс ошибок в симуляции |
| DIP (инверсия зависимостей) | Эмитент событий знает только `IEventSink`; плагины получают случайность только через `IRandomGenerator` |
| OCP (открыт для расширения) | Новые события и механики не меняют блок A: `EventPayload` хранит произвольные поля, `Registry` принимает любые значения |
| DRY | Один шаблон `Registry` на все реестры; один `MonotonicCounter` на три вида нумерации; один `FixedTimeStep` на все переводы «секунды → тики» |
| Детерминизм | Упорядоченные контейнеры, канонический порядок направлений, собственный ГСЧ и собственный SHA-256; тип чисел спрятан за `Scalar` |

## 0.3. Состав блока (26 классов)

| Группа | Классы |
|---|---|
| Идентификаторы и счётчики | `ConfigId<Tag>`, `InstanceId`, `MonotonicCounter` |
| Время и числа | `Tick`, `FixedTimeStep`, `NumericUtils` (+ `Scalar`) |
| Геометрия и гекс-сетка | `Vec2`, `Circle`, `Segment`, `HexDirection`, `HexCoord`, `HexLayout` |
| Случайность | `IRandomGenerator`, `DeterministicRandom` |
| Версии и хеши | `Version`, `Digest`, `Sha256` |
| Ошибки и коллекции | `Diagnostic`, `DiagnosticList`, `Result<T>`, `Registry<TId, TValue>` |
| События (контракт A.9) | `EventPayload`, `EventDraft`, `Event`, `IEventSink`, `NullEventSink` |

## 0.4. Карта связей между классами блока

```
ConfigId<Tag> <-- MechanicId, EventTypeId (alias)       InstanceId <-- MonotonicCounter (выдаёт числа)
Tick <-- FixedTimeStep (результат перевода секунд)       NumericUtils <-- FixedTimeStep, Vec2 (использует)

Vec2 <>-- Circle (центр)           Vec2 <>-- Segment (2 конца)
HexDirection <-- HexCoord (соседи)  HexCoord, Vec2, Circle <-- HexLayout (использует)

IRandomGenerator <|.. DeterministicRandom
Sha256 --> Digest (создаёт)

Diagnostic <>-- DiagnosticList      Diagnostic <-- Result<T>      Result<T> <-- Registry (add возвращает)

EventPayload <>-- EventDraft <>-- Event       Tick <>-- Event
EventTypeId, MechanicId, InstanceId  <-- EventDraft (поля)
IEventSink ..> EventDraft           IEventSink <|.. NullEventSink
```

(`<>--` композиция, `<|..` реализация интерфейса, `-->` создаёт, `..>` использует.)

\newpage

# 1. Идентификаторы и счётчики

## 1.1. `ConfigId<Tag>`

| | |
|---|---|
| **Вид** | шаблонный value object (неизменяемый) |
| **Файл** | `core/ConfigId.h` |
| **UC** | UC-2, 4, 6, 7, 8, 9, 15 (везде, где «id matching `[a-z0-9_]+`») |

**Ответственность.** Хранит строковое имя сущности конфигурации (`"fire_tower"`, `"goblin"`, `"level_1"`) и гарантирует, что имя соответствует шаблону `[a-z0-9_]+`.

**Что такое `Tag`.** Пустая структура-метка, существующая только на этапе компиляции (`struct TowerTag {}`). Она делает `ConfigId<TowerTag>` и `ConfigId<EnemyTag>` разными типами: перепутать их нельзя, ошибка будет на этапе компиляции, а не в рантайме.

```
- value_ : std::string
- ConfigId(value : std::string)                              «закрытый конструктор»
+ tryParse(text : std::string_view) : Result<ConfigId<Tag>>  {static}
+ str() : const std::string&                                 {const}
+ operator==(other) : bool       + operator<(other) : bool   {const}
std::hash<ConfigId<Tag>>         «специализация для unordered_map»
```

**Инварианты.** Непустая строка; символы только `a–z`, `0–9`, `_`; объект неизменяем после создания.

**Алиасы, объявленные в блоке A** (сквозные, нужны уже в `Event`): `MechanicId = ConfigId<MechanicTag>`, `EventTypeId = ConfigId<EventTypeTag>`. Доменные алиасы (`TowerTypeId`, `EnemyTypeId`, `MapId`, `WaveId`, `PresetId`, `CampaignId` и т.д.) объявляются в блоке B рядом с соответствующими типами.

**Почему так.** Формат id упоминается почти в каждом редакторном юзкейсе. Если проверять строку в десятке валидаторов, где-нибудь её забудут проверить. Здесь проверка одна, и невалидный id в системе не существует. Закрытый конструктор не даёт обойти проверку.

## 1.2. `InstanceId`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/InstanceId.h` |
| **UC** | UC-1 (ничьи, порядок событий), UC-14 (порядок перемаршрутизации), A.3 (порядок целей AoE) |

**Ответственность.** Числовой идентификатор конкретного экземпляра в рантайме (`enemy#17`, `tower#3`). Уникален в пределах игровой сессии, не переиспользуется. Не путать с `ConfigId`: тот называет *тип*, этот — *экземпляр*.

```
- value_ : uint64_t
+ InstanceId(value : uint64_t)                   «explicit; значение 0 зарезервировано как «нет»; нумерация с 1»
+ value() : uint64_t                             {const}
+ operator==, operator<                          {const}
std::hash<InstanceId>
```

**Почему так.**

- Правило ничьих «наименьший стабильный id» (A.1) требует порядка, независимого от адресов в памяти.
- Ссылки между сущностями (снаряд → цель) хранятся по id, а не по указателю: если цель умерла, поиск по id безопасно возвращает «нет такого».
- Выдаёт id блок G (`EntityIdAllocator`), используя `MonotonicCounter`. Сам класс только хранит значение.

## 1.3. `MonotonicCounter`

| | |
|---|---|
| **Вид** | класс с состоянием |
| **Файл** | `core/MonotonicCounter.h` |
| **UC** | UC-3 (номер команды), UC-1 / A.9 (номер события в тике), выдача `InstanceId` |

**Ответственность.** Выдаёт строго возрастающие номера: 1, 2, 3, …

```
- next_ : uint64_t
+ MonotonicCounter(start : uint64_t = 1)
+ take() : uint64_t              «возвращает текущее значение и увеличивает счётчик»
+ peek() : uint64_t              {const}
+ restore(value : uint64_t)      «только для восстановления из снимка / реплея»
```

**Почему так.** Три разные нумерации (команды, события, экземпляры) ведут себя одинаково, поэтому реализованы одним классом. `restore` явно отделён от `take`, чтобы счётчик нельзя было случайно «отмотать» в обычной работе.

\newpage

# 2. Время и числа

## 2.1. `Tick`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Tick.h` |
| **UC** | UC-1 (шаг 1), UC-8 (спавн по тикам), UC-17 (запись команд по тикам) |

**Ответственность.** Целое число тиков симуляции. Используется и как *индекс* (тик №412), и как *длительность* (задержка 40 тиков). Отдельный тип не даёт смешать тики с секундами или произвольными целыми.

```
- index_ : int64_t                             «>= 0»
+ Tick(index : int64_t = 0)                   «explicit»
+ value() : int64_t                           {const}
+ next() : Tick                               {const}   «index + 1»
+ operator+(Tick), operator-(Tick) : Tick     {const}
+ operator==, <, <=, >, >=                    {const}
```

**Почему так.** Один тип для индекса и длительности — сознательное упрощение (в стиле «единица времени симуляции»). Если проверяющему понадобится строгое разделение, его можно ввести без влияния на остальные блоки.

## 2.2. `FixedTimeStep`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/FixedTimeStep.h` |
| **UC** | UC-1 (шаг 1: dt), UC-2 (настройка dt), UC-8 (шаг 7: перевод времени в тики), UC-15 (шаг 9) |

**Ответственность.** Хранит фиксированный шаг симуляции `dt` (в секундах) и является *единственным* местом перевода секунд в тики и обратно.

```
- dtSeconds_ : Scalar                                          «finite, > 0»
+ create(dtSeconds : Scalar) : Result<FixedTimeStep>           {static}
+ seconds() : Scalar                                           {const}
+ delayToTicks(seconds : Scalar) : Result<Tick>                {const}   «seconds >= 0; ceil(seconds/dt); 0 допустим»
+ intervalToTicks(seconds : Scalar) : Result<Tick>             {const}   «seconds > 0; max(1, ceil(seconds/dt))»
+ ticksToSeconds(ticks : Tick) : Scalar                        {const}
```

**Почему два метода перевода.** Правила в документе различаются:

- **задержка** (`delaySeconds`, UC-8.5, 8.7) может быть нулевой;
- **интервал** (`intervalSeconds`, UC-8.7, UC-15.9) всегда занимает минимум один тик.

**Округление с допуском.** `0.75 / 0.05` в двоичном double равно 15.000000000000002, и обычный `ceil` вернул бы 16 вместо 15. Поэтому внутри используется `NumericUtils::ceilTolerant`. Без этого тики спавна «ползли» бы на машинах с разной арифметикой.

## 2.3. `NumericUtils` и алиас `Scalar`

| | |
|---|---|
| **Вид** | утилитный класс (только статические функции, конструктор удалён) + `using Scalar = double;` |
| **Файл** | `core/Numeric.h` |
| **UC** | UC-4–8, 15 (правила A.6: «finite», диапазоны); UC-8 / 15 (округление) |

```
using Scalar = double                                                    «ЕДИНСТВЕННАЯ точка выбора числового типа»

+ isFinite(value : Scalar) : bool                                        {static}
+ inClosedRange(value, lo, hi : Scalar) : bool                           {static}   «lo <= value <= hi»
+ ceilTolerant(value : Scalar, relEps : Scalar = 1e-9) : int64_t         {static}
+ floorTolerant(value : Scalar, relEps : Scalar = 1e-9) : int64_t        {static}   «floor(0.29×100) = 29, а не 28»
+ approxEqual(a, b : Scalar, relEps : Scalar = 1e-9) : bool              {static}
```

**Почему так.** Вопрос «double или fixed-point» (пункт 4 согласования) остаётся открытым. Весь код блоков выше пишется через `Scalar`, поэтому смена на fixed-point потребует правок в блоке A (`Vec2`, `Circle`, `Segment`, `HexLayout`, `DeterministicRandom`, `Sha256::updateScalar`) и не затронет остальные блоки.

\newpage

# 3. Геометрия и гекс-сетка

## 3.1. `Vec2`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Vec2.h` |
| **UC** | UC-1 (шаги 6–8, 13: непрерывные позиции), A.5 |

**Ответственность.** Точка или вектор в непрерывной плоскости карты. Единица длины — тайл (расстояние между центрами соседних гексов).

```
- x_, y_ : Scalar
+ Vec2(x, y : Scalar)
+ x(), y() : Scalar                                  {const}
+ operator+, -, *(Scalar), /(Scalar) : Vec2          {const}
+ dot(other : Vec2) : Scalar                         {const}
+ lengthSquared() : Scalar                           {const}
+ length() : Scalar                                  {const}
+ distanceTo(other : Vec2) : Scalar                  {const}
+ normalized() : Vec2                                {const}   «нулевой вектор → нулевой вектор»
+ moveTowards(target : Vec2, maxStep : Scalar) : Vec2 {const}  «не проскакивает цель»
+ operator==                                         {const}
```

**Почему так.** `moveTowards` нужен движению врагов (UC-1.8: «скорость × dt, остаток дистанции переносится на следующий сегмент маршрута»): он возвращает новую точку и не перелетает цель. Точное равенство `==` допустимо: сравнение «примерно» в симуляции не используется (детерминизм).

## 3.2. `Circle`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Circle.h` |
| **UC** | UC-1 (шаги 11–14), UC-3 (шаг 7), A.1 |

**Ответственность.** Окружность: центр + радиус. Моделирует `collisionRadius` врага, `hitRadius` снаряда, `aoeRadius`, дальность башни.

```
- center_ : Vec2   «композиция»
- radius_ : Scalar «> 0»
+ Circle(center : Vec2, radius : Scalar)
+ center() : Vec2     + radius() : Scalar               {const}
+ contains(point : Vec2) : bool                         {const}   «distance <= radius»
+ overlaps(other : Circle) : bool                       {const}   «distance <= r1 + r2»
```

## 3.3. `Segment`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Segment.h` |
| **UC** | UC-1 (шаг 13: проверка пути снаряда), A.1 («swept segment») |

**Ответственность.** Отрезок между двумя точками. Нужен, чтобы быстрый снаряд не «пролетал сквозь» цель между тиками (tunneling): проверяется весь отрезок пути за тик.

```
- from_, to_ : Vec2   «композиция»
+ Segment(from, to : Vec2)
+ from(), to() : Vec2                                   {const}
+ length() : Scalar                                     {const}
+ closestPointTo(point : Vec2) : Vec2                   {const}
+ intersects(circle : Circle) : bool                    {const}   «distance(center, closestPoint) <= radius»
```

## 3.4. `HexDirection`

| | |
|---|---|
| **Вид** | перечисление (`enum class : uint8_t`) |
| **Файл** | `core/HexDirection.h` |
| **UC** | UC-14 (шаг 4: детерминированные ничьи при поиске пути) |

```
«enumeration» HexDirection { D0, D1, D2, D3, D4, D5 }
+ allDirections() : std::array<HexDirection, 6>   {static}   «канонический порядок обхода»
+ offset(d : HexDirection) : (dq : int, dr : int) {static}   «осевые смещения: (+1,0) (+1,-1) (0,-1) (-1,0) (-1,+1) (0,+1)»
```

**Почему так.** Порядок обхода соседей фиксирован в одном месте. Поиск пути (блок F) всегда рассматривает соседей в этом порядке, поэтому при равной длине выбирается один и тот же маршрут на любой машине.

## 3.5. `HexCoord`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/HexCoord.h` |
| **UC** | UC-3 (шаг 5), UC-7 (шаги 1–2), UC-14, A.5 |

**Ответственность.** Осевая координата клетки гекс-сетки `(q, r)`. Это дискретная часть мира: «какая клетка».

```
- q_, r_ : int32_t
+ HexCoord(q, r : int32_t)
+ q(), r() : int32_t                                      {const}
+ neighbor(d : HexDirection) : HexCoord                   {const}
+ neighbors() : std::array<HexCoord, 6>                   {const}   «в каноническом порядке»
+ distanceTo(other : HexCoord) : int32_t                  {const}   «(|dq| + |dr| + |dq+dr|) / 2»
+ operator==, operator<                                   {const}   «< лексикографически по (q, r)»
std::hash<HexCoord>
```

**Почему так.** Хранение в осевых координатах совпадает с документом (A.5). Целочисленная арифметика не зависит от выбора `Scalar`.

## 3.6. `HexLayout`

| | |
|---|---|
| **Вид** | класс-преобразователь (без изменяемого состояния после создания) |
| **Файл** | `core/HexLayout.h` |
| **UC** | UC-1 (шаг 8: вход в клетку ловушки), UC-3 (шаг 7: враг в клетке), UC-14 (привязка врага к дороге), A.5 |

**Ответственность.** Мост между дискретным и непрерывным миром: переводит `HexCoord` ↔ `Vec2` и отвечает на геометрические вопросы о клетке. Ориентация гексов (остроконечные вверх) задана *только здесь*.

**Как связаны карта и непрерывное движение.** Карта хранит дискретные факты («клетка — дорога», «тут стоит ловушка»). Враги и снаряды имеют непрерывные координаты `Vec2`. `HexLayout` отвечает на вопросы вида «в какой клетке сейчас эта точка?» и «где центр этой клетки?».

```
- spacing_ : Scalar   «расстояние между центрами соседей = 1 тайл»
+ HexLayout()                                                     «pointy-top; spacing = 1»
+ toWorld(cell : HexCoord) : Vec2                                 {const}   «x = q + r/2;  y = (√3/2)·r»
+ fromWorld(point : Vec2) : HexCoord                              {const}   «кубическое округление; границы — по фиксированному правилу»
+ cellContains(cell : HexCoord, point : Vec2) : bool              {const}
+ overlapsCell(cell : HexCoord, circle : Circle) : bool           {const}   «круг врага пересекает клетку»
+ cellVertices(cell : HexCoord) : std::array<Vec2, 6>             {const}   «для рендера и расчёта пересечений»
```

**Почему так.** Остальной код не должен знать формул гекс-геометрии. Если рендеру нужна другая ориентация, меняется только этот класс. `overlapsCell` реализован здесь (решение согласовано: отдельный класс контура клетки не нужен).

\newpage

# 4. Случайность

## 4.1. `IRandomGenerator`

| | |
|---|---|
| **Вид** | интерфейс |
| **Файл** | `core/IRandomGenerator.h` |
| **UC** | UC-11 (шаг 4: выбор seed), UC-13 (шаг 3: seed из реплея), UC-17 (шаг 1: запись seed) |

```
«interface» IRandomGenerator
+ nextUInt64() : uint64_t                       {abstract}
+ nextDouble() : Scalar                         {abstract}   «[0, 1)»
+ nextInt(lo, hi : int64_t) : int64_t           {abstract}   «включительно, без смещения»
+ state() : std::vector<uint64_t>               {abstract, const}
+ restore(state : std::vector<uint64_t>)        {abstract}
```

**Почему так.** Механики-плагины (A.8, уровень 3) получают случайность только через этот интерфейс: источник неслучайности один, и его состояние можно записать в реплей. `state()/restore()` нужны перемотке (UC-13, шаг 8).

## 4.2. `DeterministicRandom`

| | |
|---|---|
| **Вид** | реализация интерфейса |
| **Файл** | `core/DeterministicRandom.h` |
| **UC** | UC-11, 13, 17 |

```
«implements» IRandomGenerator
- s_ : std::array<uint64_t, 4>            «состояние xoshiro256**»
+ DeterministicRandom(seed : uint64_t)    «состояние инициализируется через SplitMix64»
+ nextUInt64 / nextDouble / nextInt / state / restore   «реализация интерфейса»
```

**Почему так.** Распределения стандартной библиотеки (`std::uniform_int_distribution` и др.) *не гарантируют* одинаковых результатов в разных компиляторах. Для реплеев нужен собственный генератор с полностью определённым поведением: `nextDouble` = `(x >> 11) · 2⁻⁵³`, `nextInt` использует отбрасывание (rejection sampling), чтобы не было смещения.

**Замечание для проверки.** В текущих юзкейсах ядро случайность не использует (все правила детерминированы), но seed требуется UC-11/13/17, а плагины могут использовать генератор (например, критические удары).

\newpage

# 5. Версии и хеши

## 5.1. `Version`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Version.h` |
| **UC** | UC-2 (шаг 4: schemaVersion), UC-12 (шаг 2), UC-13 (шаг 1: simulationRulesVersion, версии плагинов), UC-17 (шаг 1) |

```
- major_, minor_, patch_ : uint32_t
+ Version(major, minor, patch : uint32_t)
+ parse(text : std::string_view) : Result<Version>     {static}   «"1.2.3"»
+ toString() : std::string                             {const}
+ operator==, <                                        {const}
```

**Почему так.** Три разные версии (схемы файла, правил симуляции, плагина) имеют один формат, поэтому используется один тип. Правила совместимости («поддерживается ли схема», «версия плагина совпадает точно») реализуются в блоках D и K, а не здесь.

## 5.2. `Digest`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Digest.h` |
| **UC** | UC-12 (campaignConfigHash), UC-13 (шаги 1, 9), UC-17 (шаги 5–6) |

```
- bytes_ : std::array<uint8_t, 32>
+ fromHex(text : std::string_view) : Result<Digest>   {static}
+ toHex() : std::string                                {const}
+ operator==, operator!=                               {const}
```

## 5.3. `Sha256`

| | |
|---|---|
| **Вид** | класс с состоянием (инкрементальный хешер) |
| **Файл** | `core/Sha256.h` |
| **UC** | UC-12, 13, 17 |

```
- state_, buffer_, length_
+ update(data : const void*, size : size_t)
+ updateUInt64(v : uint64_t)      + updateInt64(v : int64_t)
+ updateString(s : std::string_view)      «с префиксом длины — без неоднозначности склейки»
+ updateScalar(v : Scalar)       «канонические биты: -0 → +0, единый NaN»
+ finalize() : Digest            «после вызова объект не используется до reset()»
+ reset()
+ oneShot(data : const void*, size : size_t) : Digest   {static}
```

**Почему так.** Собственная реализация SHA-256 (~100 строк) убирает зависимость от внешних библиотек и гарантирует одинаковые байты на всех платформах. Числа хешируются по каноническому битовому представлению, иначе `-0.0` и `0.0` дали бы разные суммы.

\newpage

# 6. Ошибки и коллекции

## 6.1. `Diagnostic`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/Diagnostic.h` |
| **UC** | UC-2 (шаг 2, 7), UC-4 (шаг 7), UC-6 (шаг 7), UC-7 (шаг 11), UC-8 (шаг 10), UC-12 (шаг 8), UC-13 (шаг 2), A.6 (именованные предупреждения) |

**Ответственность.** Одно сообщение о проблеме: *что* неверно, *где* и насколько серьёзно. Документ повсюду требует, чтобы ошибка называла поле, волну, группу, путь или критерий.

```
«enumeration» Severity { Error, Warning }

- severity_ : Severity
- code_ : std::string            «машинный код: "outOfRange", "unreachableGoal", "unknownId"…»
- fieldPath_ : std::string       «"waves[2].groups[0].count"»
- message_ : std::string
+ error(code, fieldPath, message) : Diagnostic     {static}
+ warning(code, fieldPath, message) : Diagnostic   {static}
+ severity(), code(), fieldPath(), message()       {const}
```

## 6.2. `DiagnosticList`

| | |
|---|---|
| **Вид** | коллекция |
| **Файл** | `core/DiagnosticList.h` |
| **UC** | UC-2–10, 15 (валидаторы), UC-12, 13 (проверки загрузки) |

```
- items_ : std::vector<Diagnostic>        «агрегация, порядок добавления сохраняется»
+ add(d : Diagnostic)
+ merge(other : DiagnosticList)
+ hasErrors() : bool                      {const}
+ errors() : std::vector<Diagnostic>      {const}
+ warnings() : std::vector<Diagnostic>    {const}
+ empty() : bool    + size() : size_t     {const}
+ begin() / end()                         «итерация»
```

**Почему так.** Валидатор должен показывать разработчику *все* ошибки за один проход, а не только первую. Warnings не блокируют сохранение (A.6: неизвестное необязательное поле игнорируется с именованным предупреждением).

## 6.3. `Result<T>`

| | |
|---|---|
| **Вид** | шаблон |
| **Файл** | `core/Result.h` |
| **UC** | все (ожидаемые ошибки без исключений) |

```
- storage_ : std::variant<T, Diagnostic>
+ success(value : T) : Result<T>          {static}
+ failure(error : Diagnostic) : Result<T> {static}
+ isOk() : bool                            {const}
+ value() : const T&                       {const}   «предусловие: isOk()»
+ error() : const Diagnostic&              {const}   «предусловие: !isOk()»
+ valueOr(fallback : T) : T                {const}
«специализация» Result<void>
```

**Почему так.** Невалидный ввод (файл разработчика, команда игрока) — ожидаемая ситуация, а не исключение. `Result` заставляет вызывающего обработать ошибку. Исключения остаются для нарушения контракта (программная ошибка).

## 6.4. `Registry<TId, TValue>`

| | |
|---|---|
| **Вид** | шаблон |
| **Файл** | `core/Registry.h` |
| **UC** | UC-2 (шаг 5: 8 реестров проекта), UC-4–9, UC-15, A.8 (реестры механик) |

**Ответственность.** Хранит значения по уникальному ключу и обходит их в стабильном порядке. Это базовый «словарь с правилами» для всех реестров системы; о его применении подробнее — в блоках B и E.

```
- items_ : std::map<TId, TValue>               «упорядочен по ключу ⇒ детерминированный обход»
+ add(id : TId, value : TValue) : Result<void> «ошибка при дубликате id»
+ replace(id : TId, value : TValue) : Result<void>
+ remove(id : TId) : bool
+ find(id : const TId&) : const TValue*        {const}   «nullptr, если нет»
+ contains(id : const TId&) : bool             {const}
+ ids() : std::vector<TId>                     {const}
+ size() : size_t    + begin() / end()         {const}
```

**Требование к `TId`:** определён `operator<` (подходят `ConfigId<Tag>`, `HexCoord`, `InstanceId`).

**Почему так.** `std::unordered_map` обходится в непредсказуемом порядке, а порядок влияет на симуляцию и сериализацию. `std::map` даёт один и тот же порядок на любой машине. `replace` и `remove` нужны редактору (разработчик правит определения); в игровой сессии реестры только читаются.

\newpage

# 7. События (контракт A.9)

Событие — запись о том, что произошло в симуляции (выстрел, урон, смена маршрута). События нужны для трассировки в тест-режиме и диагностики; реплей воспроизводится по командам, а не по событиям (UC-13). Каталог типов событий определён в блоке E, журнал событий — в блоке I; здесь только общий «конверт».

**Поток события:**

```
подсистема (урон, стрельба…)  →  создаёт EventDraft  →  IEventSink.emit()  →  журнал (блок I) присваивает tick и sequence  →  Event
```

## 7.1. `EventPayload`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/EventPayload.h` |
| **UC** | UC-1 (шаг 17), UC-10 (шаг 8: числа DamageApplied), A.9 (поле `data`) |

**Ответственность.** Упорядоченный набор полей события «ключ → значение». Позволяет любому событию (в том числе из плагина) нести свои данные, не создавая новых классов в ядре.

```
using Value = std::variant<bool, int64_t, Scalar, std::string, InstanceId>

- entries_ : std::vector<std::pair<std::string, Value>>   «порядок вставки сохраняется»
+ set(key : std::string, value : Value)                   «повторный ключ заменяет значение»
+ has(key : std::string_view) : bool                      {const}
+ get<T>(key : std::string_view) : std::optional<T>       {const}
+ entries() : const std::vector<…>&                       {const}
```

**Почему так.** Упорядоченность важна для детерминированной сериализации и хеша трассы.

## 7.2. `EventDraft`

| | |
|---|---|
| **Вид** | value object |
| **Файл** | `core/EventDraft.h` |
| **UC** | UC-1 (все фазы, эмитирующие события), UC-10 |

**Ответственность.** «Заготовка» события: то, что знает эмитент. Номера тика и последовательности он не знает.

```
- type_ : EventTypeId                     «композиция»
- sourceId_ : std::optional<InstanceId>
- targetId_ : std::optional<InstanceId>
- mechanicId_ : std::optional<MechanicId>
- payload_ : EventPayload                 «композиция»
+ EventDraft(type : EventTypeId)
+ withSource(id : InstanceId) : EventDraft      {const}   «возвращает копию с изменённым полем»
+ withTarget(id : InstanceId) : EventDraft      {const}
+ withMechanic(id : MechanicId) : EventDraft    {const}
+ withField(key : std::string, v : Value) : EventDraft {const}
+ type() / sourceId() / targetId() / mechanicId() / payload()   {const}
```

## 7.3. `Event`

| | |
|---|---|
| **Вид** | неизменяемый value object |
| **Файл** | `core/Event.h` |
| **UC** | UC-1 (шаг 17), UC-10 (шаг 8: фильтры трассы), A.9 |

**Ответственность.** Законченная запись трассы `{tick, sequence, type, sourceId, targetId, mechanicId, data}`.

```
- tick_ : Tick                    «композиция»
- sequence_ : uint64_t            «возрастает внутри тика»
- draft_ : EventDraft             «композиция: содержимое события»
+ Event(tick : Tick, sequence : uint64_t, draft : EventDraft)
+ tick() / sequence()             {const}
+ type() / sourceId() / targetId() / mechanicId() / payload()   {const}   «делегируют в draft_»
```

**Почему так.** `Event` не копирует поля `EventDraft`, а содержит его (композиция): поле добавляется в одном месте.

## 7.4. `IEventSink`

| | |
|---|---|
| **Вид** | интерфейс |
| **Файл** | `core/IEventSink.h` |
| **UC** | UC-1, UC-10 |

```
«interface» IEventSink
+ emit(draft : EventDraft)            {abstract}
```

**Почему так (DIP).** Подсистема урона вызывает `sink.emit(...)` и не знает, куда попадёт событие: в журнал трассы, в экран тест-режима или никуда. Реализации — `EventLog` (блок I) и `NullEventSink` (ниже).

## 7.5. `NullEventSink`

| | |
|---|---|
| **Вид** | реализация интерфейса (паттерн Null Object) |
| **Файл** | `core/NullEventSink.h` |
| **UC** | UC-14 (проверка маршрутов без трассы), тесты |

```
«implements» IEventSink
+ emit(draft : EventDraft)            «ничего не делает»
```

\newpage

# 8. Сводные таблицы

## 8.1. Покрытие юзкейсов блоком A

| UC | Что даёт блок A |
|---|---|
| UC-1 | `Tick`, `FixedTimeStep`, `Vec2`, `Circle`, `Segment`, `InstanceId`, `MonotonicCounter`, `Event*`, `IEventSink` |
| UC-2 | `ConfigId`, `Version`, `Registry`, `Diagnostic*`, `Result`, `FixedTimeStep` |
| UC-3 | `MonotonicCounter` (номер команды), `HexCoord`, `HexLayout`, `Result` |
| UC-4–6 | `ConfigId`, `NumericUtils`, `Diagnostic*`, `Registry` |
| UC-7 | `HexCoord`, `HexDirection`, `Vec2`, `ConfigId` |
| UC-8 | `FixedTimeStep`, `Tick`, `Diagnostic*` |
| UC-9 | `ConfigId`, `Registry`, `Diagnostic*` |
| UC-10 | `Event`, `EventPayload`, `IEventSink` |
| UC-11 | `IRandomGenerator`, `DeterministicRandom` |
| UC-12 | `Version`, `Sha256`, `Digest`, `Diagnostic*` |
| UC-13 | `Version`, `Sha256`, `Digest`, `IRandomGenerator` |
| UC-14 | `HexDirection`, `HexCoord`, `HexLayout`, `NullEventSink`, `InstanceId` |
| UC-15 | `FixedTimeStep`, `NumericUtils`, `ConfigId` |
| UC-16 | `ConfigId`, `Registry` (косвенно) |
| UC-17 | `Version`, `Sha256`, `Digest`, `DeterministicRandom`, `Tick` |

## 8.2. Что намеренно не входит в блок A

| Не входит | Где будет |
|---|---|
| Каталог типов событий (A.9) | Блок E |
| Журнал событий (`EventLog`), присваивание `tick` / `sequence` | Блок I |
| Генератор `InstanceId` для сущностей | Блок G |
| Валидаторы конфигурации | Блок C |
| Файловый ввод-вывод | Блок D |
| Доменные алиасы id (`TowerTypeId` и др.) | Блок B |

## 8.3. Открытые вопросы

1. **Тип чисел (double / fixed-point)** — решение отложено. Затронуты: `Scalar`, `Vec2`, `Circle`, `Segment`, `HexLayout`, `DeterministicRandom`, `Sha256::updateScalar`.
2. `Tick` совмещает роли индекса и длительности. При необходимости строгого разделения вводится отдельный тип длительности; остальные блоки это не затронет.
