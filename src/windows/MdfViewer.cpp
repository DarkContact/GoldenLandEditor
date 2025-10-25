#include "MdfViewer.h"

#include <format>
#include <array>

#include "SDL3/SDL_render.h"
#include "imgui.h"

#include "parsers/MDF_Parser.h"
#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "Types.h"

void MdfViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& mdfFiles)
{
    Tracy_ZoneScoped;

    static int selectedIndex = -1;
    static std::vector<std::vector<BaseAnimation>> animationEntries;
    static std::string uiError;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static int activeButtonIndex = 0;
    static ImGuiTextFilter textFilter;

    bool needResetScroll = false;

    ImGui::Begin("MDF Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilter.Draw();
        ImGui::Separator();
        ImGui::BeginChild("file list");
        for (int i = 0; i < static_cast<int>(mdfFiles.size()); ++i)
        {
            if (textFilter.PassFilter(mdfFiles[i].c_str())
                && ImGui::Selectable(mdfFiles[i].c_str(), selectedIndex == i))
            {
                selectedIndex = i;

                uiError.clear();

                auto mdfDataOpt = MDF_Parser::parse(std::format("{}/{}", rootDirectory, mdfFiles[i]), &uiError);
                if (mdfDataOpt) {
                    auto& mdfData = *mdfDataOpt;

                    animationEntries.clear();
                    animationEntries.resize(mdfData.entries.size());

                    int entryIndex = 0;
                    for (const auto& entry : mdfData.entries) {
                        auto& animationEntry = animationEntries[entryIndex];
                        animationEntry.resize(entry.packs.size());

                        int packIndex = 0;
                        for (const auto& pack : entry.packs) {
                            auto& animationPack = animationEntry[packIndex];
                            animationPack.delay = 100;

                            if (pack.animationPath.ends_with(".bmp")) {
                                uiError = ".bmp format unsupported et!";
                                break;
                            }

                            bool isOk = TextureLoader::loadCountAnimationFromCsxFile(std::format("{}/magic/bitmap/{}", rootDirectory, pack.animationPath), pack.framesCount, renderer, animationPack.textures, &uiError);
                            if (!isOk)
                                break;
                            ++packIndex;
                        }
                        ++entryIndex;
                    }
                }

                needResetScroll = true;
            }
        }
        ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    if (!animationEntries.empty() && uiError.empty()) {
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            ImVec2 originalSpacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(originalSpacing.x, 0)); // убрать вертикальный отступ

            int csxTextureWidth = 0;
            int csxTextureHeight = 0;
            std::array<int, 5> entriesPackCount;
            ImVec2 startPos = ImGui::GetCursorScreenPos();
            uint64_t now = SDL_GetTicks();

            int entryIndex = 0;
            for (auto& entry : animationEntries) {
                entriesPackCount[entryIndex] = entry.size();
                for (auto& animation : entry) {
                    animation.update(now);
                    ImGui::SetCursorScreenPos(startPos);
                    ImGui::ImageWithBg((ImTextureID)animation.currentTexture().get(), ImVec2(animation.currentTexture()->w, animation.currentTexture()->h), ImVec2(0, 0), ImVec2(1, 1), bgColor);

                    csxTextureWidth = animation.currentTexture()->w;
                    csxTextureHeight = animation.currentTexture()->h;
                }
                ++entryIndex;
            }

            ImGui::PopStyleVar();

            ImGui::EndChild();

            ImGui::Text("%dx%d", csxTextureWidth, csxTextureHeight);

            ImGui::SameLine(0.0f, 12.0f);
            std::array<char, 80> entries;
            {
                int offset = 0;
                for (int i = 0; i < animationEntries.size(); ++i) {
                    auto result = std::format_to_n(entries.data() + offset, entries.size() - offset, "L{} = {}, ", i + 1, entriesPackCount[i]);
                    offset += result.size;
                }
                entries[offset - 2] = '\0';
            }

            std::array<char, 128> entriesMessage;
            auto resultFmt = std::format_to_n(entriesMessage.data(), entriesMessage.size() - 1, "Animations count: [{}]", entries.data());
            entriesMessage[std::min((size_t)resultFmt.size, entriesMessage.size() - 1)] = '\0';
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f) ,"%s", entriesMessage.data());

            if (ImGui::RadioButton("Transparent", activeButtonIndex == 0)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
                activeButtonIndex = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Black", activeButtonIndex == 1)) {
                bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                activeButtonIndex = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Gray", activeButtonIndex == 2)) {
                bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                activeButtonIndex = 2;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("White", activeButtonIndex == 3)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                activeButtonIndex = 3;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Custom", activeButtonIndex == 4)) {
                activeButtonIndex = 4;
            }
            ImGui::SameLine();
            if (activeButtonIndex == 4) {
                ImGui::ColorEdit4("Color", (float*)&bgColor, ImGuiColorEditFlags_NoInputs);
            }
        }
        ImGui::EndGroup();
    } else if (selectedIndex >= 0 && !uiError.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", uiError.c_str());
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        animationEntries.clear();
        uiError.clear();
        textFilter.Clear();
    }
}
