#include "DialogTests.h"

std::unordered_map<std::string, std::vector<DialogInstruction> > getManualDialogTestData()
{
    std::unordered_map<std::string, std::vector<DialogInstruction>> manualCases = {
        {
            "scripts\\dialogs\\common\\l0.pt26_armor_traider.age.cs",
            std::vector<DialogInstruction>{
                { kExec },
                { kUserInputAndExec, 1 },

                { kHardRestart },
                { kExec },
                { kUserInputAndExec, 2 },

                { kHardRestart },
                { kSetVariable, 1, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 1 },

                { kHardRestart },
                { kSetVariable, 1, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 2 },

                { kHardRestart },
                { kSetVariable, 2, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 1 },

                { kHardRestart },
                { kSetVariable, 2, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 2 },

                { kHardRestart },
                { kSetVariable, 3, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 1 },

                { kHardRestart },
                { kSetVariable, 3, "armor_traider_povtor" },
                { kExec },
                { kUserInputAndExec, 2 },
            }
        }
    };
    return manualCases;
}
