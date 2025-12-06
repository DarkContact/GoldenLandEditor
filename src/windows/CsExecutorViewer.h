#pragma once
#include <span>
#include <memory>

#include "Types.h"
#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

#include "CsExecutor.h"

class CsExecutorViewer {
public:
    CsExecutorViewer();

    void update(bool& showWindow,
                bool& needUpdate,
                std::span<const CS_Node> nodes,
                const UMapStringVar_t& globalVars,
                const SDB_Data& dialogsPhrases);

    bool isNodeExecuted(int index) const;
    int executedPercent() const;

private:
    std::unique_ptr<CsExecutor> m_pExecutor = nullptr;
    bool m_isDialog = false;
    bool m_resetAllVariables = false;
};
