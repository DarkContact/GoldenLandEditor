## RS_TestHeroHasPartyName

```c
int RS_TestHeroHasPartyName(string npcTechName)
```

### Описание
Проверить состоит ли NPC в группе

### Параметры
* **npcTechName** - техническое имя NPC

### Возвращаемое значение
Возвращает 0 если NPC не в группе и 1 если в группе

### Пример
```c
result = RS_TestHeroHasPartyName("L0.PWolf");
```