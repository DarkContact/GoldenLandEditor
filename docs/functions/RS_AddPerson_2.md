## RS_AddPerson_2

```c
int RS_AddPerson_2(string npcTechName, int positionX, int positionY, string direction, string litName, string tribe, string scrDialog, string scrInv)
```

### Описание
Добавить NPC в текущую локацию

### Параметры
* **npcTechName** - техническое имя NPC
* **positionX** - позиция NPC по x (в кол-ве big cell)
* **positionY** - позиция NPC по x (в кол-ве big cell)
* **direction** - направление взгляда NPC (см. раздел [Direction](#Direction))
* **litName** - имя NPC которое отображается в UI
* **tribe** - группа отношений (описаны в `scripts\tribes.scr`)
* **scrDialog** - путь до файла диалогов. Путь задаётся из директории `scripts\dialogs`. Расширение `.cs` для скомпилированных скриптов указывать не нужно.
* **scrInv** - путь до файла описания инвентаря. Путь задаётся из директории `scripts\inventory`. Расширение `.inv` указывать не нужно. Путь не чувствительн к регистру.

### Direction
* UP
* UP_LEFT
* LEFT
* DOWN_LEFT
* DOWN
* DOWN_RIGHT
* RIGHT
* UP_RIGHT

### Заметки
Функция вызывается в паре с `RS_AddPerson_1`

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_AddPerson_1("MOVED", "kill1", 0, 2000, 3000);
result = RS_AddPerson_2("L00.alva_guard8", 160, 124, "DOWN_LEFT", "Стражница Альвы",  "temple_2", "selm\temple_priest.age", "INV002_RICH_SPEARMAN");
```

### Смотри также
**[RS_AddPerson_1](RS_AddPerson_1.md)**