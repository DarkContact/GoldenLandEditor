## RS_StartDialog

```c
int RS_StartDialog(string techName, string scriptName)
```

### Описание
Начать диалог

### Параметры
* **techName** - техническое имя NPC или триггера с которым нужно начать диалог
* **scriptName** - название скрипта который будет запущен. Путь задаётся из директории `scripts\dialogs`. Расширение `.cs` для скомпилированных скриптов указывать не нужно.

### Заметки
NPC или триггер должны присутствовать на локации. Можно поговорить самим с собой используя `techName = "Hero"`

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_StartDialog("LA14_1_trg_9_filling", "pit\L14.P75_plesen.age"); // Триггер
result = RS_StartDialog("L14.P71_Watcher", "pit\L14.P71_Watcher.age"); // NPC
```