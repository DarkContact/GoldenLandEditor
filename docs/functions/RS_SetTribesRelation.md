## RS_SetTribesRelation

```c
int RS_SetTribesRelation(string sourceTribeName, string targetTribeName, string tribeValue)
```

### Описание
Установить значение группы отношений

### Параметры
* **sourceTribeName** - имя группы отношений (тот, чьё отношение мы устанавливаем - активная сторона)
* **targetTribeName** - имя группы отношений (тот, к кому это отношение измеряется - пассивная сторона)
* **tribeValue** - значение группы отношений (см. раздел [Tribe values](#Tribe-values))

### Tribe values
```c
VERY_EVIL = 0
EVIL      = 1
NEUTRAL   = 2
GOOD      = 3
VERY_GOOD = 4
```

### Возвращаемое значение
Возвращает 0 

### Пример
```c
result = RS_SetTribesRelation("h_guard_1", "hero", "VERY_EVIL"); // Стражник теперь относится к герою очень плохо и будет нападать
```

### Смотри также
**[RS_GetTribesRelation](RS_GetTribesRelation.md)**