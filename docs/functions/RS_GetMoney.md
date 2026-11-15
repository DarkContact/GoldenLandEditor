## RS_GetMoney

```c
int RS_GetMoney()
```

### Описание
Получить количество золотых у героя

### Заметки
Получить тоже значение можно и через RS_GetItemCountI
```c
money = RS_GetItemCountI("Hero", "MON_1_0_1");
```

### Возвращаемое значение
Возвращает количество золотых у героя

### Пример
```c
money = RS_GetMoney();
```

### Смотри также
**[RS_GetItemCountI](RS_GetItemCountI.md)**