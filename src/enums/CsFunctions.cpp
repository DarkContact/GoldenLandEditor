#include "CsFunctions.h"

// SERVER.DLL [14042464]
const char* csFuncToString(double value) noexcept {
    if (value == 0) return "RS_GetPersonParameterI (0)";

    if (value == 0x1000000) return "Exit";
    if (value == 0x1000001) return "Signal";
    if (value == 0x1000002) return "Console";
    if (value == 0x1000003) return "Cmd";
    if (value == 0x1000004) return "D_Say";
    if (value == 0x1000005) return "D_CloseDialog";
    if (value == 0x1000006) return "D_Answer";
    if (value == 0x1000007) return "D_PlaySound";

    if (value == 0x2000000) return "LE_CastEffect";
    if (value == 0x2000001) return "LE_DelEffect";
    if (value == 0x2000002) return "LE_CastMagic";

    if (value == 0x3000000) return "WD_LoadArea";
    if (value == 0x3000001) return "WD_SetCellsGroupFlag";
    if (value == 0x3000002) return "RS_SetTribesRelation";
    if (value == 0x3000003) return "RS_GetTribesRelation";
    if (value == 0x3000004) return "RS_StartDialog";
    if (value == 0x3000005) return "WD_SetVisible";
    if (value == 0x3000006) return "C_FINISHED";
    if (value == 0x3000007) return "WD_TitlesAndLoadArea";
    if (value == 0x3000008) return "C_TitlesAndFINISHED";

    if (value == 0x4000000) return "RS_GetPersonParameterI";
    if (value == 0x4000001) return "RS_SetPersonParameterI";
    if (value == 0x4000002) return "RS_AddPerson_1";
    if (value == 0x4000003) return "RS_AddPerson_2";
    if (value == 0x4000004) return "RS_IsPersonExistsI";
    if (value == 0x4000005) return "RS_AddExp";
    if (value == 0x4000006) return "RS_DelPerson";
    if (value == 0x4000007) return "RS_AddToHeroPartyName";
    if (value == 0x4000008) return "RS_RemoveFromHeroPartyName";
    if (value == 0x4000009) return "RS_TestHeroHasPartyName";
    if (value == 0x400000A) return "RS_AllyCmd";
    if (value == 0x400000B) return "RS_ShowMessage";
    if (value == 0x400000C) return "RS_GetPersonSkillI";

    if (value == 0x5000000) return "RS_TestPersonHasItem";
    if (value == 0x5000001) return "RS_PersonTransferItemI";
    if (value == 0x5000002) return "RS_GetItemCountI";
    if (value == 0x5000003) return "RS_PersonTransferAllItemsI";
    if (value == 0x5000004) return "RS_PersonAddItem";
    if (value == 0x5000005) return "RS_PersonRemoveItem";
    if (value == 0x5000006) return "RS_PersonAddItemToTrade";
    if (value == 0x5000007) return "RS_PersonRemoveItemToTrade";
    if (value == 0x5000008) return "RS_GetMoney";

    if (value == 0x6000000) return "RS_GetDayOrNight";
    if (value == 0x6000001) return "RS_GetCurrentTimeOfDayI";
    if (value == 0x6000002) return "RS_GetDaysFromBeginningI";
    if (value == 0x6000003) return "RS_AddTime";

    if (value == 0x7000000) return "RS_QuestComplete";
    if (value == 0x7000001) return "RS_StageEnable";
    if (value == 0x7000002) return "RS_QuestEnable";
    if (value == 0x7000003) return "RS_StageComplete";
    if (value == 0x7000004) return "RS_StorylineQuestEnable";
    if (value == 0x7000005) return "RS_SetEvent";
    if (value == 0x7000006) return "RS_GetEvent";
    if (value == 0x7000007) return "RS_ClearEvent";
    if (value == 0x7000008) return "RS_SetLocationAccess";
    if (value == 0x7000009) return "RS_EnableTrigger";
    if (value == 0x700000A) return "RS_GetRandMinMaxI";
    if (value == 0x700000B) return "RS_SetWeather";
    if (value == 0x700000C) return "RS_SetSpecialPerk";
    if (value == 0x700000D) return "RS_PassToTradePanel";
    if (value == 0x700000E) return "RS_GetDialogEnabled";
    if (value == 0x700000F) return "RS_SetUndeadState";
    if (value == 0x7000010) return "RS_GlobalMap";
    if (value == 0x7000013) return "RS_SetInjured";
    if (value == 0x7000014) return "RS_SetDoorState";

    return "unknown";
}
