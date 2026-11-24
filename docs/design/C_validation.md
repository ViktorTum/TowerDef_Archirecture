# Блок C. Validation (namespace td::validation) — ЧЕРНОВИК

Зависит от: A, B. Все проверки допустимости конфигурации (A.6 и «блокирует сохранение/публикацию» из UC-2…UC-10, UC-15).
Результат любой проверки — `DiagnosticList` (блок A): ВСЕ ошибки за один проход, каждая с путём к полю.
Классы B (определения) проверок НЕ содержат: данные и правила допустимости разделены (SRP).

## Принцип
Один валидатор = один вид определения (Strategy). Общий интерфейс `IValidator<T>`; валидаторы собирает в целое `ProjectValidator` (Facade).
Блок C зависит от поведения механик (блок E) и маршрутов (блок F) ТОЛЬКО через «порты» — интерфейсы, которыми владеет C, а реализуют E и F (DIP). Поэтому C остаётся ниже E и F и проверяется отдельно.

## C.1 Инфраструктура
| Класс | Что | Зачем / почему | UC |
|---|---|---|---|
| IValidator<T> | интерфейс: validate(const T&, const ValidationContext&) -> DiagnosticList | единый контракт; редактор вызывает «валидатор нужного типа» одинаково | 2-10,15 |
| ValidationContext | только чтение: GameProject, IMechanicCatalog, IRouteVerifier, SchemaVersionPolicy | всё, что нужно проверкам «ссылка существует?», без глобальных переменных | 2-10,15 |
| MechanicKind (enum) | Fire / Damage / Targeting / Effect / Activation | вид механики в каталоге | 4,5 |
| IMechanicCatalog (порт) | exists(id, kind), schemaOf(id, kind), checkCompatibility(fireId, damageId) -> Result<void> | UC-4.10: «mechanic ids существуют», «выход fire читается damage» (A.3); реализует блок E | 4,5 |
| IRouteVerifier (порт) | isGroundPairReachable(map, spawnId, goalId) | UC-7.10, 8.4: достижимость пары через UC-14; реализует блок F | 7,8,14 |
| SchemaVersionPolicy | множество поддерживаемых Version; isSupported(v) | «поддерживаемый schemaVersion» (UC-2.4, 10.2, 12.6) в одном месте | 2,10,12 |

## C.2 Параметры и постройки
| Класс | Что | Зачем / почему | UC |
|---|---|---|---|
| ParameterSetValidator | ParameterSet против ParameterSchema: обязательные, тип, finite, диапазон, перечисления; неизвестное необязательное поле -> warning | ядро проверок A.6; переиспользуется башнями, апгрейдами, эффектами | 4,5,6 |
| StructureSchemaBuilder | собирает схему: таблица A.6 для вида + схемы выбранных fire/damage/targeting механик | «обязательные поля зависят от механики» (UC-4.6); тот же код нужен апгрейдам (UC-5.7) | 4,5 |
| StructureTypeValidator (абстрактный) | общее: cost, sellRefund 0..cost, placement, ассет; шаблонный метод + hook validateSpecific | общее один раз (Template Method) | 4 |
| TowerTypeValidator : StructureTypeValidator | damage/range/attackSpeed, механики, совместимость fire x damage | UC-4.3, 4.6-4.7, 4.10 | 4 |
| TrapTypeValidator : StructureTypeValidator | onEnter, blocksGroundMovement=false, нет range/attackSpeed | UC-4.4, 4.9 | 4 |
| BarrierTypeValidator : StructureTypeValidator | blocksGroundMovement=true, destructible => health > 0 | UC-4.5, 4.9 | 4 |
| UpgradeTreeValidator | уникальность, существование родителей, связность, циклы, цена >= 0, unlockId существует, результат КАЖДОГО узла проходит схему | UC-5.7-5.8 | 5 |
| DamageTypeValidator | resistance в [-1,1], параметры эффекта, effect id есть в каталоге | A.2 | 6 |
| EnemyTypeValidator | диапазоны A.6, ground => obstacleAttack*, ассет | UC-6.7 | 6 |

## C.3 Карта, волны, сложность
| Класс | Что | Зачем / почему | UC |
|---|---|---|---|
| MapValidator | размеры > 0, spawn/goal существуют и уникальны, клетки внутри карты, здоровье целей >= 1, условия победы/поражения, targetTick, воздушные маршруты (уникальный id, первая точка = spawn, последняя = goal, точки в границах), достижимость пар (через IRouteVerifier), allowedPresetIds существуют | UC-7.10-7.11 | 7 |
| WaveScheduleValidator | список конечен и непуст, waveId уникальны, enemyId существуют, count >= 1, interval > 0, delay >= 0, spawn/goal существуют, ground => потенциальный маршрут, air => airRouteId ведёт от spawn к goal | UC-8.3-8.4, 8.10; UC-6.8 | 8 |
| DifficultyPresetValidator | четыре множителя finite > 0, уникальный id | UC-15.2-15.4 | 15 |

## C.4 Кампания
| Класс | Что | Зачем / почему | UC |
|---|---|---|---|
| CampaignGraphAnalyzer | чистый анализ графа: циклы связей, циклы пререквизитов, итеративная достижимость от стартового узла, самозависимость | алгоритм UC-9.9-9.10 отдельно от правил, тестируется на графах | 9 |
| CampaignValidator | ссылки levelNodeId/mapId/prerequisite/unlockId, связь между двумя СУЩЕСТВУЮЩИМИ РАЗНЫМИ узлами, единственный стартовый, награды (Fixed: 1 вариант, Choice: >= 2 разных), вызов анализатора | UC-9.8-9.10 | 9 |

## C.5 Проект и готовность уровня
| Класс | Что | Зачем / почему | UC |
|---|---|---|---|
| ProjectConfigValidator | baseStartingCoins >= 0, schemaVersion поддерживается, dt валиден | UC-2.3-2.4 | 2 |
| ProjectValidator (фасад) | запускает все валидаторы по всему GameProject + межреестровая целостность: defaultPresetId существует и входит в allowed, unlock -> башня существует, ссылки на ассеты и типы урона | UC-15.6; A.6 «все ссылки разрешаются в проекте» | 2,10,15 |
| LevelReadinessValidator | для пары (карта, пресет): схема поддерживается, все id зарегистрированы, цели валидны, >= 1 конечная волна, маршрут для каждой группы, пресет разрешён, wavesCleared => конечное расписание | UC-10.2 и проверка перед UC-11.3; UC-7.11 (отложенная проверка wavesCleared) | 10,11 |

Итого 23 типа (1 перечисление).

## Распределение проверок по юзкейсам (кто за что отвечает)
UC-2 -> ProjectConfigValidator; UC-4 -> TowerTypeValidator / TrapTypeValidator / BarrierTypeValidator (+StructureSchemaBuilder, ParameterSetValidator);
UC-5 -> UpgradeTreeValidator; UC-6 -> EnemyTypeValidator, DamageTypeValidator; UC-7 -> MapValidator; UC-8 -> WaveScheduleValidator;
UC-9 -> CampaignValidator + CampaignGraphAnalyzer; UC-10 -> LevelReadinessValidator (+ProjectValidator); UC-15 -> DifficultyPresetValidator + ProjectValidator.
НЕ в блоке C: проверки команд игрока (UC-3) -> блок H; проверки файлов сохранений и реплеев (UC-12.6, UC-13.1) -> блоки D и K.

## Вопросы
1. Порты IMechanicCatalog / IRouteVerifier: ок? (альтернатива - поменять порядок блоков и поставить C выше E и F)
2. Правило пресетов (дополнение к B): список карты приоритетнее списка кампании; пусты оба -> ошибка.
