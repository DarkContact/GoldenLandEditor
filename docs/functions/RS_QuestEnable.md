## RS_QuestEnable

```c
int RS_QuestEnable(string questTechName, string cityTechName)
```

### Описание
Начать второстепенный квест

### Параметры
* **questTechName** - название квеста, задаётся из `sdb\diary\single_quest_tech.sdb`
* **cityTechName** - название города где был взят квест, задаётся из `sdb\diary\cities_tech.sdb`

### Заметки
Отображает иконку восклицательного знака в левом нижнем углу и делает запись в дневник. Добавляет запись в "Несюжетные" в формате `cityLitName -> dd.MM.yyyy questLitName`

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_QuestEnable("starosta", "som");
result = RS_StageEnable("starosta", "starosta_1");
```

### Смотри также
**[RS_StorylineQuestEnable](RS_StorylineQuestEnable.md)**
**[RS_StageEnable](RS_StageEnable.md)**