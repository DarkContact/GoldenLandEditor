## LE_DelEffect

```c
int LE_DelEffect(string groupName)
```

### Описание
Удалить эффекты заданные идентификатором в `LE_CastEffect`

### Параметры
**groupName** - название группы эффектов, заданное через `LE_CastEffect`

### Возвращаемое значение
Возвращает 0

### Пример
```c
test = LE_DelEffect("center");
```

### Смотри также
**[LE_CastEffect](LE_CastEffect.md)**