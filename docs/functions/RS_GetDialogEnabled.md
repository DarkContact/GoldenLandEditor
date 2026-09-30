## RS_GetDialogEnabled

```c
int RS_GetDialogEnabled(int value)
```

### Описание
Выполнить проверку на убеждение

### Параметры
* **value** - обычно здесь указывают 2 (из игровых примеров)

### Заметки
[Особенности выполнения диалогов](../AgeScript.md#особенности-выполнения-диалогов).

### Возвращаемое значение
Возвращает 0 в случае провала проверки и 1 в случае успеха

### Пример
```c
if (dialog_branch == 0)
{	
    rs_e = RS_GetDialogEnabled(2); // Проверка на убеждение  
	test = D_Say(10);
	test = D_Answer(8);
	test = D_Answer(6);
    if (rs_e) {
        test = D_Answer(20); // Скрытая ветка диалога
    }
	dialog_branch = 1;
}
```