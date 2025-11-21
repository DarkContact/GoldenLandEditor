# Таблица встроенных функций Age Script

| Функция                       | Описание                                            |
|-------------------------------|-----------------------------------------------------|
| [Exit](#exit)                 | Прерывает выполнение скрипта                        |
| Signal                        |                                                     |
| Console                       |                                                     |
| Cmd                           |                                                     |
| D_Say                         |                                                     |
| D_CloseDialog                 |                                                     |
| D_Answer                      |                                                     |
| D_PlaySound                   |                                                     |
| LE_CastEffect                 |                                                     |
| LE_DelEffect                  |                                                     |
| LE_CastMagic                  |                                                     |
| WD_LoadArea                   |                                                     |
| WD_SetCellsGroupFlag          |                                                     |
| RS_SetTribesRelation          |                                                     |
| RS_GetTribesRelation          |                                                     |
| RS_StartDialog                |                                                     |
| WD_SetVisible                 |                                                     |
| C_FINISHED                    |                                                     |
| WD_TitlesAndLoadArea          |                                                     |
| C_TitlesAndFINISHED           |                                                     |
| RS_GetPersonParameterI        |                                                     |
| RS_SetPersonParameterI        |                                                     |
| RS_AddPerson_1                |                                                     |
| RS_AddPerson_2                |                                                     |
| RS_IsPersonExistsI            |                                                     |
| RS_AddExp                     |                                                     |
| RS_DelPerson                  |                                                     |
| RS_AddToHeroPartyName         |                                                     |
| RS_RemoveFromHeroPartyName    |                                                     |
| RS_TestHeroHasPartyName       |                                                     |
| RS_AllyCmd                    |                                                     |
| RS_ShowMessage                |                                                     |
| RS_GetPersonSkillI            |                                                     |
| RS_TestPersonHasItem          |                                                     |
| RS_PersonTransferItemI        |                                                     |
| RS_GetItemCountI              |                                                     |
| RS_PersonTransferAllItemsI    |                                                     |
| RS_PersonAddItem              |                                                     |
| RS_PersonRemoveItem           |                                                     |
| RS_PersonAddItemToTrade       |                                                     |
| RS_PersonRemoveItemToTrade    |                                                     |
| RS_GetMoney                   |                                                     |
| RS_GetDayOrNight              |                                                     |
| RS_GetCurrentTimeOfDayI       |                                                     |
| RS_GetDaysFromBeginningI      |                                                     |
| RS_AddTime                    |                                                     |
| RS_QuestComplete              |                                                     |
| RS_StageEnable                |                                                     |
| RS_QuestEnable                |                                                     |
| RS_StageComplete              |                                                     |
| RS_StorylineQuestEnable       |                                                     |
| RS_SetEvent                   |                                                     |
| RS_GetEvent                   |                                                     |
| RS_ClearEvent                 |                                                     |
| RS_SetLocationAccess          |                                                     |
| RS_EnableTrigger              |                                                     |
| RS_GetRandMinMaxI             |                                                     |
| RS_SetWeather                 |                                                     |
| RS_SetSpecialPerk             |                                                     |
| RS_PassToTradePanel           |                                                     |
| RS_GetDialogEnabled           |                                                     |
| RS_SetUndeadState             |                                                     |
| RS_GlobalMap                  |                                                     |
| RS_SetInjured                 |                                                     |
| RS_SetDoorState               |                                                     |


## Exit
`void Exit(int bNeedExit)`

### Параметры
| Имя         | Тип      | Описание                              |
|-------------|----------|---------------------------------------|
| `bNeedExit` | `int`    | Если > 0, то функция будет вызвана    |

### Пример
```c
// По итогу в диалоге будет только 1 вариант ответа
result = D_Say(10);
result = D_Answer(8);
result = Exit(1);
result = D_Answer(6);
```
