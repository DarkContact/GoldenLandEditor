## RS_GetTribesRelation

```c
int RS_GetTribesRelation(string sourceTribeName, string targetTribeName)
```

### Описание
Получить значение группы отношений

### Параметры
* **sourceTribeName** - имя группы отношений (тот, чьё отношение мы проверяем - активная сторона)
* **targetTribeName** - имя группы отношений (тот, к кому это отношение измеряется - пассивная сторона)

### Возвращаемое значение
Возвращает значение группы отношений (см. раздел [Tribe values](#Tribe-values))

### Tribe values
```c
VERY_EVIL = 0
EVIL      = 1
NEUTRAL   = 2
GOOD      = 3
VERY_GOOD = 4
```

### Пример
```c
result = RS_GetTribesRelation("fione", "hero"); // Проверяем как Фиона относится к герою
```

### Смотри также
**[RS_SetTribesRelation](RS_SetTribesRelation.md)**