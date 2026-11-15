## RS_PassToTradePanel

```c
int RS_PassToTradePanel()
```

### Описание
Открыть окно торговли c говорящим

### Заметки
Если вызвать не из диалога, то ничего не делает

### Возвращаемое значение
Возвращает 0

### Пример
```c
if (dialog_branch == 1)
{	
    test = RS_PassToTradePanel();
	dialog_branch = 0;
}
```