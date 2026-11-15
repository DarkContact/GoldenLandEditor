## RS_StageEnable

```c
int RS_StageEnable(string questTechName, string questStageTechName)
```

### Описание
Начать этап выполнения квеста

### Параметры
* **questTechName** - название квеста, задаётся из `sdb\diary\single_quest_tech.sdb`
* **questStageTechName** - название шага выполнения квеста, задаётся из `sdb\diary\single_quest_tech.sdb`

### Заметки
Отображает иконку восклицательного знака в левом нижнем углу и добавляет запись о ходе выполнения квеста в дневник.

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_StorylineQuestEnable("long_way_to_home"); // Создаёт запись о квесте
result = RS_StageEnable("long_way_to_home", "long_way_to_home_1"); // Добавляет текст квеста внутрь задания
```

### Смотри также
**[RS_StageComplete](RS_StageComplete.md)**