## RS_GetPersonParameterI

```c
int RS_GetPersonParameterI(string npcTechName, string paramName)
```

### Описание
Получить значение параметра NPC

### Параметры
* **npcTechName** - техническое имя NPC
* **paramName** - название параметра (см. раздел [Parameters](#Parameters))

### Parameters
* ENERGY
* LIFE
* MAXIMUM LIFE
* PERCEPTION
* LUCK
* DEXTERITY
* CONSTITUTION
* STRENGTH
* WISDOM
* INTELLIGENCE
* REPUTATION
* EXPERIENCE

### Возвращаемое значение
Возвращает значение параметра

### Пример
```c
life = RS_GetPersonParameterI("Hero", "LIFE");
slava = RS_GetPersonParameterI("Hero", "REPUTATION");
exp = RS_GetPersonParameterI("Hero", "EXPERIENCE");
```

### Смотри также
**[RS_SetPersonParameterI](RS_SetPersonParameterI.md)**