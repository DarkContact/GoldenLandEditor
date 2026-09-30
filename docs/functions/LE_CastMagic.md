## LE_CastMagic

```c
int LE_CastMagic(string magicTechName, int x, int y)
```

### Описание
Отобразить визуальный эффект в локации 1 раз

### Параметры
**magicTechName** - название магического эффекта (эффекты описаны в `scripts\magic.scr` и `sdb\magic\magictechnames.sdb`)
**x** - координата x (в пикселях) где отобразить центр эффекта (для простоты поиска координат можно выполнить в консоли `d_info_world 1`)
**x** - координата y (в пикселях) где отобразить центр эффекта

### Возвращаемое значение
Возвращает 0

### Пример
```c
result = LE_CastMagic("ef_icebeastrising", 902, 762);
```

### Смотри также
**[LE_CastEffect](LE_CastEffect.md)**