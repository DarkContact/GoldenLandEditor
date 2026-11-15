## RS_GetPersonSkillI

```c
int RS_GetPersonSkillI(string npcTechName, string skillName)
```

### Описание
Получить значение навыка NPC

### Параметры
* **npcTechName** - техническое имя NPC
* **skillName** - название навыка (см. раздел [SkillName](#SkillName))

### SkillName
```c
SKILL_WPN_SWORD       // навык владения мечом
SKILL_WPN_AXE         // навык владения топором
SKILL_WPN_CRUSH       // навык владения дробящим оружием
SKILL_WPN_STAFF       // навык владения посохом
SKILL_WPN_DIST        // навык владения стрелковым оружием
SKILL_WPN_SPEAR       // навык владения копьем
SKILL_WPN_THROW       // навык владения метательным оружием
SKILL_WPN_HAND        // навык рукопашного боя
SKILL_CRITICAL_HIT    // навык критического удара

SKILL_SHADMAG         // навык магии теней
SKILL_NATRMAG         // навык магии природы 
SKILL_GODSMAG         // навык магии богов
SKILL_ELEMMAG         // навык магии стихий
SKILL_LGHTMAG         // навык магии света
SKILL_DARKMAG         // навык магии тьмы
SKILL_MAGICUSE        // навык волшебство
SKILL_ALCHEMY         // алхимия
SKILL_IDENTIFY        // эрудиция

SKILL_TACTIC          // навык тактики
SKILL_SCOUT           // следопыт
SKILL_HEALING         // навык знахарства
SKILL_TALKING         // красноречие
SKILL_TRADE           // торговля
SKILL_STEAL           // воровство
SKILL_HACK            // естествознание
SKILL_SMITH           // кузнечное дело     
SKILL_ATHLETIC        // навык атлетизм
```

### Возвращаемое значение
Возвращает значение навыка

### Пример
```c
skill = RS_GetPersonSkillI("Hero", "SKILL_STEAL");
```