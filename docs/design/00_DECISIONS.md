# Tower Defence engine — архитектура: журнал решений

Источник: usecases_tower_defence_v5.docx (UC-1..UC-17, Appendix A). Стек: C++ + Godot.
Итог работы: список классов по блокам -> PDF-схема классов для коллеги (диаграмма в Visual Paradigm).

## Согласовано
1. Разбивка на 13 блоков (A..M) подтверждена (см. ниже).
2. Godot = только рендер (GDExtension). Редактор разработчика = ОТДЕЛЬНОЕ приложение (не EditorPlugin, не экраны игры).
   => Блок L (Dev Tools) вместе с Persistence/Validation/Definitions входит в отдельное приложение-редактор;
      блок M (Godot Integration) содержит только рендер игры, ввод игрока, UI игры/кампании/реплея.
3. Классы с наследованием и композицией (не ECS): Structure -> Tower/Trap/Barrier, Enemy, Projectile + системы по фазам тика.
4. Детерминизм чисел (double vs fixed-point): ОТКРЫТО (?). Мера: числовой тип изолирован алиасом Scalar (блок A),
   смена на fixed-point не ломает остальные блоки.

## Блоки (зависимости только вниз)
A Foundation | B Definitions | C Validation | D Persistence | E Mechanics&Plugins | F Spatial (hex map & routing)
G Simulation State | H Commands | I Simulation Engine | J Campaign&Session | K Replay | L Dev Tools | M Godot Integration

## Статус блоков
- A Foundation: СОГЛАСОВАН (ConfigId<Tag> шаблоном; overlapsCell внутри HexLayout). Подробное описание: A_foundation_detailed.md/.pdf
- B Definitions: СОГЛАСОВАН (ParameterSet + TowerStats; допущения 1-4). Описание: B_definitions_detailed.md/.pdf. Добавлено после согласования: CampaignDefinition.allowedPresetIds (список карты приоритетнее) - ждёт подтверждения
- C Validation: черновик выдан, ждёт согласования (C_validation.md). Порты IMechanicCatalog/IRouteVerifier (владеет C, реализуют E/F)
- D..M: не начаты

## Правила работы
- Выдаём блок за блоком: список классов + «зачем и почему» + покрытие UC. После согласования блок фиксируем здесь.
- Каждый класс привязан к UC; в конце — сверка «UC -> классы» по всем 17 юзкейсам.
- В конце: PDF со схемой классов (пакеты = блоки, связи: наследование, композиция, агрегация, зависимость).

## Дополнения к ранее согласованным блокам
- A: NumericUtils получил floorTolerant (нужен DifficultyPreset.startingCoins, UC-15.8).

## Замечания к документу юзкейсов (для правки v5 -> v6)
1. UC-2.5: нет реестра наград (unlocks), а UC-5.5 и UC-9.8 на него ссылаются.
2. UC-3.4: не определено, как башня становится «заблокированной». Принято: башня без ссылающегося UnlockDefinition доступна всегда.
3. UC-2.5: «mechanics» - принято как список требуемых механик (id + версия).
4. UC-5.2: область уникальности nodeId (принято: в пределах дерева одной башни).
5. UC-15.5: allowedPresetIds у карты ИЛИ кампании - не определён приоритет (принято: карта важнее; оба пусты - ошибка).
