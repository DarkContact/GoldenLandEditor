## RS_StorylineQuestEnable

```c
int RS_StorylineQuestEnable(string questTechName)
```

### Описание
Начать сюжетный квест

### Параметры
* **questTechName** - название квеста, задаётся из `sdb\diary\single_quest_tech.sdb`

### Заметки
Отображает иконку восклицательного знака в левом нижнем углу и делает запись в дневник. Добавляет запись в "Сюжетные" в формате `dd.MM.yyyy questLitName`.  
Обычно сразу после идёт шаг c `RS_StageEnable`, т.к. иначе в дневнике задания не будет никакого текста.

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_StorylineQuestEnable("long_way_to_home");
result = RS_StageEnable("long_way_to_home", "long_way_to_home_1");
```

### Смотри также
**[RS_QuestEnable](RS_QuestEnable.md)**
**[RS_StageEnable](RS_StageEnable.md)**