## RS_IsPersonExistsI

```c
int RS_IsPersonExistsI(string levelName, string npcTechName)
```

### Описание
Проверить существует ли NPC

### Параметры
* **levelName** - название локации где проводить проверку (если передать "", то ищем везде)
* **npcTechName** - техническое имя NPC

### Возвращаемое значение
Возвращает 0 если NPC не существует и 1 если существует

### Пример
```c
result = RS_IsPersonExistsI("tower_6", "L15.P10_ruhlus");
```
