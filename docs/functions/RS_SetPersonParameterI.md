## RS_SetPersonParameterI

```c
int RS_SetPersonParameterI(string npcTechName, string paramName, int paramValue)
```

### Описание
Установить значение параметра NPC

### Параметры
* **npcTechName** - техническое имя NPC
* **paramName** - название параметра (см. раздел [Parameters](#Parameters))
* **paramValue** - значение параметра

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
Возвращает 0

### Пример
```c
result = RS_SetPersonParameterI("L6.PS5_Adai", "LIFE", 0);
```

### Смотри также
**[RS_GetPersonParameterI](RS_GetPersonParameterI.md)**