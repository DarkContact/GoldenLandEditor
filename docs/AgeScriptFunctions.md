## Встроенные функции Age Script

### Системные
* **[Exit](functions/Exit.md)** - Прервать выполнение скрипта
* **[Signal](functions/Signal.md)** - Показать диалоговое окно Windows с текстом
* **[Console](functions/Console.md)** - Напечатать текст в отладочной консоли
* **[Cmd](functions/Cmd.md)** - Выполнить строку в отладочной консоли ([Список консольных команд](ConsoleCommands.md))

### Визуальные эффекты
* **[LE_CastMagic](functions/LE_CastMagic.md)** - Отобразить визуальный эффект в локации 1 раз
* **[LE_CastEffect](functions/LE_CastEffect.md)** - Отобразить закицленный визуальный эффект в локации
* **[LE_DelEffect](functions/LE_DelEffect.md)** - Удалить эффекты заданные идентификатором в `LE_CastEffect`

### Диалоги с NPC
* **[RS_StartDialog](functions/RS_StartDialog.md)**
* **[D_Say](functions/D_Say.md)**
* **[D_Answer](functions/D_Answer.md)**
* **[D_PlaySound](functions/D_PlaySound.md)**
* **[D_CloseDialog](functions/D_CloseDialog.md)**
* **[RS_GetDialogEnabled](functions/RS_GetDialogEnabled.md)**
* **[RS_PassToTradePanel](functions/RS_PassToTradePanel.md)**

### Квесты
* **[RS_StorylineQuestEnable](functions/RS_StorylineQuestEnable.md)** - Начать сюжетный квест
* **[RS_QuestEnable](functions/RS_QuestEnable.md)** - Начать второстепенный квест
* **[RS_StageEnable](functions/RS_StageEnable.md)** - Активировать этап выполнения квеста
* **[RS_StageComplete](functions/RS_StageComplete.md)** - Завершить этап выполнения квеста
* **[RS_QuestComplete](functions/RS_QuestComplete.md)** - Завершить квест

### Party
* **[RS_AddToHeroPartyName](functions/RS_AddToHeroPartyName.md)**
* **[RS_RemoveFromHeroPartyName](functions/RS_RemoveFromHeroPartyName.md)**
* **[RS_TestHeroHasPartyName](functions/RS_TestHeroHasPartyName.md)**
* **[RS_AllyCmd](functions/RS_AllyCmd.md)**

### NPC
* **[RS_AddPerson_1](functions/RS_AddPerson_1.md)**
* **[RS_AddPerson_2](functions/RS_AddPerson_2.md)**
* **[RS_DelPerson](functions/RS_DelPerson.md)**
* **[RS_IsPersonExistsI](functions/RS_IsPersonExistsI.md)**
* **[RS_GetPersonSkillI](functions/RS_GetPersonSkillI.md)**
* **[RS_GetPersonParameterI](functions/RS_GetPersonParameterI.md)**
* **[RS_SetPersonParameterI](functions/RS_SetPersonParameterI.md)**
* **[RS_GetTribesRelation](functions/RS_GetTribesRelation.md)**
* **[RS_SetTribesRelation](functions/RS_SetTribesRelation.md)**

### NPC items
* **[RS_PersonAddItem](functions/RS_PersonAddItem.md)**
* **[RS_PersonRemoveItem](functions/RS_PersonRemoveItem.md)**
* **[RS_PersonAddItemToTrade](functions/RS_PersonAddItemToTrade.md)**
* **[RS_PersonRemoveItemToTrade](functions/RS_PersonRemoveItemToTrade.md)**
* **[RS_PersonTransferItemI](functions/RS_PersonTransferItemI.md)**
* **[RS_PersonTransferAllItemsI](functions/RS_PersonTransferAllItemsI.md)**
* **[RS_TestPersonHasItem](functions/RS_TestPersonHasItem.md)**
* **[RS_GetItemCountI](functions/RS_GetItemCountI.md)**

### Infos
* **[RS_GetMoney](functions/RS_GetMoney.md)**
* **[RS_GetDayOrNight](functions/RS_GetDayOrNight.md)**
* **[RS_GetCurrentTimeOfDayI](functions/RS_GetCurrentTimeOfDayI.md)**
* **[RS_GetDaysFromBeginningI](functions/RS_GetDaysFromBeginningI.md)**

### Глобальная карта
* **[RS_GlobalMap](functions/RS_GlobalMap.md)**
* **[RS_SetLocationAccess](functions/RS_SetLocationAccess.md)**

### Перемещение по локациям
* **[WD_LoadArea](functions/WD_LoadArea.md)**
* **[WD_TitlesAndLoadArea](functions/WD_TitlesAndLoadArea.md)**

### Конец игры
* **[C_FINISHED](functions/C_FINISHED.md)**
* **[C_TitlesAndFINISHED](functions/C_TitlesAndFINISHED.md)**

### Прочие
* **[RS_GetRandMinMaxI](functions/RS_GetRandMinMaxI.md)**
* **[WD_SetCellsGroupFlag](functions/WD_SetCellsGroupFlag.md)**
* **[WD_SetVisible](functions/WD_SetVisible.md)**
* **[RS_AddExp](functions/RS_AddExp.md)**
* **[RS_ShowMessage](functions/RS_ShowMessage.md)**
* **[RS_AddTime](functions/RS_AddTime.md)**
* **[RS_EnableTrigger](functions/RS_EnableTrigger.md)**
* **[RS_SetWeather](functions/RS_SetWeather.md)**
* **[RS_SetSpecialPerk](functions/RS_SetSpecialPerk.md)**
* **[RS_SetUndeadState](functions/RS_SetUndeadState.md)**
* **[RS_SetInjured](functions/RS_SetInjured.md)**
* **[RS_SetDoorState](functions/RS_SetDoorState.md)**

### Events (Не работают)
* **[RS_SetEvent](functions/RS_SetEvent.md)**
* **[RS_GetEvent](functions/RS_GetEvent.md)**
* **[RS_ClearEvent](functions/RS_ClearEvent.md)**