## RS_StageComplete

```c
int RS_StageComplete(string questTechName, string questStageTechName)
```

### Описание
Записать этап выполнения квеста

### Параметры
* **questTechName** - название квеста, задаётся из `sdb\diary\single_quest_tech.sdb`
* **questStageTechName** - название шага выполнения квеста, задаётся из `sdb\diary\single_quest_tech.sdb`

### Заметки
Зачёркивает выполненую часть квеста в дневнике.

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_StageComplete("starosta", "starosta_1"); // Зачёркивает выполненый этап задания в дневнике
```

### Смотри также
**[RS_StageEnable](RS_StageEnable.md)**
