# Таблица встроенных функций Age Script

| Функция                                                   | Описание                                                                       |
|-----------------------------------------------------------|--------------------------------------------------------------------------------|
| [Exit](#exit)                                             | Прерывает выполнение скрипта                                                   |
| [Signal](#signal)                                         | Отображает отдельное диалоговое окно Windows с текстом                         |
| [Console](#console)                                       | Отобразить текст в отладочной консоли                                          |
| [Cmd](#cmd)                                               | Выполнить строку в отладочной консоли [ConsoleCommands.md](ConsoleCommands.md) |
| [D_Say](#d_say)                                           |                                                                                |
| [D_CloseDialog](#d_closedialog)                           |                                                                                |
| [D_Answer](#d_answer)                                     |                                                                                |
| [D_PlaySound](#d_playsound)                               |                                                                                |
| [LE_CastEffect](#le_casteffect)                           |                                                                                |
| [LE_DelEffect](#le_deleffect)                             |                                                                                |
| [LE_CastMagic](#le_castmagic)                             |                                                                                |
| [WD_LoadArea](#wd_loadarea)                               |                                                                                |
| [WD_SetCellsGroupFlag](#wd_setcellsgroupflag)             |                                                                                |
| [RS_SetTribesRelation](#rs_settribesrelation)             |                                                                                |
| [RS_GetTribesRelation](#rs_gettribesrelation)             |                                                                                |
| [RS_StartDialog](#rs_startdialog)                         |                                                                                |
| [WD_SetVisible](#wd_setvisible)                           |                                                                                |
| [C_FINISHED](#c_finished)                                 |                                                                                |
| [WD_TitlesAndLoadArea](#wd_titlesandloadarea)             |                                                                                |
| [C_TitlesAndFINISHED](#c_titlesandfinished)               |                                                                                |
| [RS_GetPersonParameterI](#rs_getpersonparameteri)         |                                                                                |
| [RS_SetPersonParameterI](#rs_setpersonparameteri)         |                                                                                |
| [RS_AddPerson_1](#rs_addperson_1)                         |                                                                                |
| [RS_AddPerson_2](#rs_addperson_2)                         |                                                                                |
| [RS_IsPersonExistsI](#rs_ispersonexistsi)                 |                                                                                |
| [RS_AddExp](#rs_addexp)                                   |                                                                                |
| [RS_DelPerson](#rs_delperson)                             |                                                                                |
| [RS_AddToHeroPartyName](#rs_addtoheropartyname)           |                                                                                |
| [RS_RemoveFromHeroPartyName](#rs_removefromheropartyname) |                                                                                |
| [RS_TestHeroHasPartyName](#rs_testherohaspartyname)       |                                                                                |
| [RS_AllyCmd](#rs_allycmd)                                 |                                                                                |
| [RS_ShowMessage](#rs_showmessage)                         |                                                                                |
| [RS_GetPersonSkillI](#rs_getpersonskilli)                 |                                                                                |
| [RS_TestPersonHasItem](#rs_testpersonhasitem)             |                                                                                |
| [RS_PersonTransferItemI](#rs_persontransferitemi)         |                                                                                |
| [RS_GetItemCountI](#rs_getitemcounti)                     |                                                                                |
| [RS_PersonTransferAllItemsI](#rs_persontransferallitemsi) |                                                                                |
| [RS_PersonAddItem](#rs_personadditem)                     |                                                                                |
| [RS_PersonRemoveItem](#rs_personremoveitem)               |                                                                                |
| [RS_PersonAddItemToTrade](#rs_personadditemtotrade)       |                                                                                |
| [RS_PersonRemoveItemToTrade](#rs_personremoveitemtotrade) |                                                                                |
| [RS_GetMoney](#rs_getmoney)                               |                                                                                |
| [RS_GetDayOrNight](#rs_getdayornight)                     |                                                                                |
| [RS_GetCurrentTimeOfDayI](#rs_getcurrenttimeofdayi)       |                                                                                |
| [RS_GetDaysFromBeginningI](#rs_getdaysfrombeginningi)     |                                                                                |
| [RS_AddTime](#rs_addtime)                                 |                                                                                |
| [RS_QuestComplete](#rs_questcomplete)                     |                                                                                |
| [RS_StageEnable](#rs_stageenable)                         |                                                                                |
| [RS_QuestEnable](#rs_questenable)                         |                                                                                |
| [RS_StageComplete](#rs_stagecomplete)                     |                                                                                |
| [RS_StorylineQuestEnable](#rs_storylinequestenable)       |                                                                                |
| [RS_SetEvent](#rs_setevent)                               |                                                                                |
| [RS_GetEvent](#rs_getevent)                               |                                                                                |
| [RS_ClearEvent](#rs_clearevent)                           |                                                                                |
| [RS_SetLocationAccess](#rs_setlocationaccess)             |                                                                                |
| [RS_EnableTrigger](#rs_enabletrigger)                     |                                                                                |
| [RS_GetRandMinMaxI](#rs_getrandminmaxi)                   |                                                                                |
| [RS_SetWeather](#rs_setweather)                           |                                                                                |
| [RS_SetSpecialPerk](#rs_setspecialperk)                   |                                                                                |
| [RS_PassToTradePanel](#rs_passtotradepanel)               |                                                                                |
| [RS_GetDialogEnabled](#rs_getdialogenabled)               |                                                                                |
| [RS_SetUndeadState](#rs_setundeadstate)                   |                                                                                |
| [RS_GlobalMap](#rs_globalmap)                             |                                                                                |
| [RS_SetInjured](#rs_setinjured)                           |                                                                                |
| [RS_SetDoorState](#rs_setdoorstate)                       |                                                                                |



## Exit
`void Exit(int bNeedExit)`

### Параметры
| Имя         | Описание                              |
|-------------|---------------------------------------|
| `bNeedExit` | Если > 0, то функция будет вызвана    |

### Пример
```c
// По итогу в диалоге будет только 1 вариант ответа
result = D_Say(10);
result = D_Answer(8);
result = Exit(1);
result = D_Answer(6); // Выполнено не будет
```


## Signal
`int Signal(string message)`

### Параметры
| Имя         | Описание                              |
|-------------|---------------------------------------|
| `message`   | Текст сообщения в диалоговом окне     |

### Возвращаемое значение
Возвращает 0

### Пример
```c
// Отображить диалоговое окно
result = Signal("Hello!");
```


## Console
`int Console(string message)`

### Параметры
| Имя         | Описание                              |
|-------------|---------------------------------------|
| `message`   | Текст сообщения в консоли             |

### Возвращаемое значение
Возвращает 0

### Пример
```c
// Отображить текст в консоли
result = Console("Hello!");
```


## Cmd
`int Cmd(string command)`

### Параметры
| Имя         | Описание                              |
|-------------|---------------------------------------|
| `command`   | Текст сообщения в консоли             |

### Возвращаемое значение
Возвращает 0

### Пример
```c
// Скрыть игровую панель
result = Cmd("show_panel 0");
```