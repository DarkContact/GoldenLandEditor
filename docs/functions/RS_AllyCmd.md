## RS_AllyCmd

```c
int RS_AllyCmd(string npcTechName, string cmd)
```

### Описание
Изменить поведение NPC из группы

### Параметры
* **npcTechName** - техническое имя NPC из sef файла локации
* **cmd** - команда изменения поведения (см. раздел [Команды](#команды))

### Команды
* CMD_ALLY_NOT_HERO_TARGET
* CMD_ALLY_HERO_TARGET
* CMD_ALLY_HERO_DANGER
* CMD_ALLY_WEAK_TARGET
* CMD_ALLY_ALL_TARGET - Атаковать всех
* CMD_ALLY_DO_NOT_FIGHT - Никого не атаковать

### Заметки
В игре используются команды `CMD_ALLY_DO_NOT_FIGHT` и `CMD_ALLY_ALL_TARGET`. Работоспособность остальных нужно проверить.

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_AllyCmd("L0.PWolf", "CMD_ALLY_DO_NOT_FIGHT");
```

### Смотри также
**[RS_RemoveFromHeroPartyName](RS_RemoveFromHeroPartyName.md)**