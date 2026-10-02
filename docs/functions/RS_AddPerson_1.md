## RS_AddPerson_1

```c
int RS_AddPerson_1(string routeType, string route, int radius, int delayMin, int delayMax)
```

### Описание
Добавить NPC в текущую локацию

### Параметры
* **routeType** - тип движения NPC (см. раздел [RouteType](#RouteType))
* **route** - имя cell group которое задаёт траекторию движения
* **radius** - радиус движения
* **delayMin** - минимальная задержка для смены движения
* **delayMax** - максимальная задержка для смены движения

### RouteType
* RANDOM
* RANDOM_RADIUS
* MOVED_FLIP
* MOVED
* STAY_ROTATE

### Заметки
Функция вызывается в паре с `RS_AddPerson_2`

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_AddPerson_1("MOVED", "kill1", 0, 2000, 3000);
result = RS_AddPerson_2("L00.alva_guard8", 160, 124, "DOWN_LEFT", "Стражница Альвы",  "temple_2", "selm\temple_priest.age", "INV002_RICH_SPEARMAN");
```

### Смотри также
**[RS_AddPerson_2](RS_AddPerson_2.md)**