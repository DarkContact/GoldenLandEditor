#pragma once
#include <string_view>
#include <memory>
#include <span>

#include "Types.h"
#include "parsers/CS_Parser.h"

#include "CsExecutor.h"

class CsExecutorViewer {
public:
    CsExecutorViewer();

    void update(bool& showWindow,
                bool& needUpdate,
                std::string_view title,
                std::span<const CS_Node> nodes,
                const std::map<int, std::string>& dialogsPhrases,
                const UMapStringVar_t& globalVars);

    bool isNodeExecuted(int index) const;
    float executedPercent() const;

private:
    std::unique_ptr<CsExecutor> m_pExecutor = nullptr;
    bool m_isDialog = false;
    bool m_resetAllVariables = false;
};
