#include "CsViewer.h"

#include <filesystem>
#include <format>

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"
#include "parsers/SDB_Parser.h"
#include "parsers/CS_Parser.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"

#include "CsExecutorViewer.h"

CsViewer::CsViewer() {}

void CsViewer::update(bool& showWindow,
                      std::string_view rootDirectory,
                      const std::vector<std::string>& csFiles,
                      const std::map<int, std::string>& dialogPhrases,
                      const StringHashTable<AgeVariable_t>& globalVars)
{
    Tracy_ZoneScoped;
    if (showWindow && !csFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;
        bool needUpdate = false;

        if (!m_onceWhenOpen) {
            /*Unused*/
            m_onceWhenOpen = true;
        }

        bool isOpenedWindow = true;
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("CS Viewer", &showWindow);

        ImGui::BeginChild("left pane", ImVec2(340, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        m_textFilterFile.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(csFiles.size()); ++i)
            {
                std::string_view csFile = csFiles[i];
                if (m_textFilterFile.PassFilter(csFile.data())
                    && ImGui::Selectable(csFile.data(), m_selectedIndex == i))
                {
                    m_selectedIndex = i;

                    m_csError.clear();
                    m_csData.nodes.clear();
                    m_funcNodes.clear();

                    std::string csPath = std::format("{}/{}", rootDirectory, csFile);
                    bool isOk = CS_Parser::parse(csPath, m_csData, &m_csError);

                    if (isOk) {
                        // Заполнение данных для фильтрации функций
                        m_funcNodes.resize(m_csData.nodes.size(), false);
                        for (size_t i = 0; i < m_csData.nodes.size(); ++i) {
                            const auto& node = m_csData.nodes[i];
                            if (node.opcode == kFunc) {
                                m_funcNodes[i] = true;
                                for (int j = 0; j < node.args.size(); ++j) {
                                    int32_t idx = node.args[j];
                                    if (idx == -1) break;

                                    m_funcNodes[++i] = true;
                                }
                            }
                        }

                        needResetScroll = true;
                        needUpdate = true;
                    }
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();


        ImGui::BeginChild("right pane");
        m_textFilterString.Draw();

            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (!m_csData.nodes.empty()) {
                char nodeInfoBuffer[4096];

                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                const ImGuiStyle& style = ImGui::GetStyle();
                ImGui::PushFont(NULL, style.FontSizeBase - 1.0f);
                for (size_t i = 0; i < m_csData.nodes.size(); ++i) {
                    const CS_Node* prevNode = nullptr;
                    if (i > 0) {
                        prevNode = &m_csData.nodes[i - 1];
                    }

                    bool isDialogPhrase = false;
                    if (prevNode) {
                        if (prevNode->opcode == kStringVarName) {
                            if (prevNode->text == "LastPhrase" || prevNode->text == "LastAnswer")
                                isDialogPhrase = true;
                        }

                        if (prevNode->opcode == kFunc) {
                            std::string_view funcStr = csFuncToString(prevNode->value);
                            if (funcStr == "D_Say" || funcStr == "D_Answer")
                                isDialogPhrase = true;
                        }
                    }

                    const CS_Node& node = m_csData.nodes[i];
                    node.toStringBuffer(nodeInfoBuffer, (isDialogPhrase && m_showDialogPhrases), dialogPhrases);

                    if (m_showOnlyFunctions && m_funcNodes[i]
                        || !m_showOnlyFunctions) {
                        if (m_textFilterString.PassFilter(nodeInfoBuffer)) {
                            bool isFunc = node.opcode == kFunc;
                            bool isExec = m_csExecutorViewer.isNodeExecuted(i);
                            ImVec4 textColor = isExec ? m_execTextColor
                                                      : (isFunc ? m_funcTextColor
                                                                : style.Colors[ImGuiCol_Text]);
                            ImGui::TextColored(textColor, "[i:%zu] %s", i, nodeInfoBuffer);
                        }
                    }
                }
                ImGui::PopFont();
            } else if (m_selectedIndex >= 0) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_csError.c_str());
            }
            ImGui::EndChild();

        ImGui::Checkbox("Funcs only", &m_showOnlyFunctions);
        ImGui::SameLine();
        ImGui::Checkbox("Dialog phrases", &m_showDialogPhrases);
        ImGui::SameLine();

        if (!m_csData.nodes.empty()) {
            ImGui::BeginDisabled(m_showExecuteWindow);
            if (ImGui::Button("Execute")) {
                m_showExecuteWindow = true;
            }
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        ImGui::Text("Executed: %.2f%%", m_csExecutorViewer.executedPercent());

        ImGui::EndChild();

        ImGui::End();

        if (m_selectedIndex >= 0) {
            m_csExecutorViewer.update(m_showExecuteWindow,
                                      needUpdate,
                                      csFiles[m_selectedIndex],
                                      m_csData.nodes,
                                      dialogPhrases,
                                      globalVars);
        }
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_csError.clear();
        m_csData.nodes.clear();
        m_textFilterFile.Clear();
        m_textFilterString.Clear();
        m_funcNodes.clear();
        m_showExecuteWindow = false;
        m_onceWhenOpen = false;
        m_onceWhenClose = true;
    }
}

void CsViewer::injectPlaySoundAndGeneratePhrases(std::string_view saveRootDirectory, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    assert(saveRootDirectory != rootDirectory);
    assert(std::filesystem::exists(StringUtils::toUtf8View(saveRootDirectory)));
    if (saveRootDirectory == rootDirectory) return;

    std::string error;
    std::string sdbPath = std::format("{}/sdb/dialogs/dialogsphrases.sdb", rootDirectory);
    SDB_Data sdbDialogs;
    if (!SDB_Parser::parse(sdbPath, sdbDialogs, &error))
        LogFmt("Load dialogsphrases.sdb error: {}", error);

    std::string_view prefix = "scripts\\dialogs\\";
    std::string_view suffix = ".age.cs";

    std::string phrases;
    for (std::string_view csFile : csFiles) {
        CS_Data csData;

        std::string csPath = std::format("{}/{}", rootDirectory, csFile);

        auto personName = csFile.substr(0, csFile.size() - suffix.size()); // Удаляем .age.cs
        personName = personName.substr(prefix.size(), personName.size()); // Удаляем префикс
        if (!CS_Parser::parse(csPath, csData, &error))
            LogFmt("CS_Parser::parse error: {}", error);

        std::set<size_t> uniquePhrases;
        bool havePhrases = false;
        std::string csPhrases;
        for (size_t i = 0; i < csData.nodes.size(); ++i) {
            const CS_Node& node = csData.nodes[i];
            if (node.opcode == kFunc && (uint32_t)node.value == kD_Say) {
                const CS_Node& phraseNode = csData.nodes[node.args[0]];
                int phraseIndex = phraseNode.value;
                auto soundFile = std::format("{}\\phrase_{}.ogg", personName, phraseIndex);
                injectPlaySoundFunc(csData, i + 2, soundFile);

                if (!uniquePhrases.contains(phraseIndex)) {
                    std::string_view sayPhrase = "[NOT FOUND!]";
                    if (auto it = sdbDialogs.strings.find(phraseIndex); it != sdbDialogs.strings.cend()) {
                        sayPhrase = it->second;
                    }

                    sayPhrase = StringUtils::trim(sayPhrase);
                    if (sayPhrase.starts_with('*') && sayPhrase.ends_with('*')) {
                        continue;
                    }

                    csPhrases += std::format("phrase_{}: {}\n", phraseIndex, sayPhrase);
                    uniquePhrases.insert(phraseIndex);
                    havePhrases = true;
                }
            }
        }
        if (havePhrases) {
            csPhrases = std::format("person: {}\n", personName) + csPhrases + "\n";
            phrases += csPhrases;
        }

        auto saveCsFileString = std::format("{}/{}", saveRootDirectory, csFile);
        std::filesystem::path saveCsFilePath(StringUtils::toUtf8View(saveCsFileString));
        std::filesystem::create_directories(saveCsFilePath.parent_path());
        if (!CS_Parser::save(StringUtils::fromUtf8View(saveCsFilePath.u8string()), csData, &error))
            LogFmt("CS_Parser::save error: {}", error);
    }

    std::span<const uint8_t> fileData(reinterpret_cast<const uint8_t*>(phrases.data()), phrases.size());
    if (!FileUtils::saveFile(std::format("{}/phrases.txt", saveRootDirectory), fileData, &error))
        LogFmt("FileUtils::saveFile error: {}", error);
}

void CsViewer::injectPlaySoundFunc(CS_Data& csData, size_t insertPos, std::string_view soundFile)
{
    CS_Node assign;
    assign.opcode = kAssign;
    assign.a = insertPos + 1;
    assign.b = insertPos + 2;
    assign.c = assign.d = insertPos + 4;

    CS_Node strVar;
    strVar.opcode = kStringVarName;
    strVar.text = "result";

    CS_Node func;
    func.opcode = kFunc;
    func.value = kD_PlaySound;
    func.args[0] = insertPos + 3;

    CS_Node strLit;
    strLit.opcode = kStringLiteral;
    strLit.text = std::string(soundFile);

    std::array<CS_Node, 4> playSoundNodes = {assign, strVar, func, strLit};
    csData.insertNodes(insertPos, playSoundNodes);

    csData.nodes[insertPos - 4].c = csData.nodes[insertPos - 4].d = insertPos;
}
