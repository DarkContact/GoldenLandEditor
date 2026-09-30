## RS_AddToHeroPartyName

```c
int RS_AddToHeroPartyName(string npcTechName)
```

### Описание
Добавить NPC в группу

### Параметры
* **npcTechName** - техническое имя NPC из sef файла локации

### Заметки
Возможно добавление в группу существ с одинаковыми `npcTechName`, сначала будет выбран тот что был раньше описан в sef файле.  
Для корректного отображения имени присоединённого NPC, оно должно быть добавлено в файлы `sdb\persons\dynamictechnames.sdb` и `sdb\persons\dynamiclitnames.sdb`

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = RS_AddToHeroPartyName("L0.PWolf");
```

### Смотри также
**[RS_RemoveFromHeroPartyName](RS_RemoveFromHeroPartyName.md)**