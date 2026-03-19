#include "Application.h"

#include <algorithm>
#include <format>

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "embedded_resources.h"

#include "Settings.h"
#include "utils/TracyProfiler.h"
#include "utils/ImGuiWidgets.h"

#include "utils/Platform.h"
#include "utils/DebugLog.h"

#ifdef DEBUG_MENU_ENABLE
  #include <filesystem>

  #include "CsExecutor.h"
  #include "utils/FileUtils.h"
  #include "utils/StringUtils.h"
  #include "utils/DialogTests.h"
#endif

Application::Application() {
    Settings settings("settings.ini");
    std::string fontFilepath = settings.readString(Setting::kFontFilepath);
    int fontSize = settings.readInt(Setting::kFontSize, 16);
    std::string rootDir = settings.readString(Setting::kRootDir);
    if (!rootDir.empty()) {
        m_rootDirContext.setRootDirectoryAndReload(rootDir);
    }

    Tracy_HookStbImageMemory();
    Tracy_HookSdlMemory();
    Tracy_HookImGuiMemory();

    initSdl();
    initImGui(fontFilepath, fontSize);

    m_fontSettings = std::make_optional<FontSettings>();
}

Application::~Application() {
    shutdown();
}

void Application::initSdl() {
    LogFmt("[INFO] SDL Version: {}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    if (SDL_VERSION != SDL_GetVersion()) {
        LogFmt("[INFO] Linked SDL Version: {}.{}.{}",
               SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MICRO(SDL_GetVersion()));
    }

    // Линкуемая версия должна быть не ниже компилируемой
    if (SDL_VERSION > SDL_GetVersion()) {
        Log("[ERROR] Compiled SDL version less when linked SDL version!");
        throw std::runtime_error("Compiled SDL version less when linked SDL version!");
    }

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LogFmt("SDL_Init error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_Init error: {}", SDL_GetError()));
    }

    // Create window with SDL_Renderer graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    m_window = SDL_CreateWindow(std::format("GoldenLand Editor v{}", GOLDENLAND_VERSION_STRING).c_str(),
                                (int)(1024 * main_scale), (int)(768 * main_scale), window_flags);
    if (m_window == nullptr) {
        LogFmt("SDL_CreateWindow error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_CreateWindow error: {}", SDL_GetError()));
    }
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (m_renderer == nullptr) {
        LogFmt("SDL_CreateRenderer error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_CreateRenderer error: {}", SDL_GetError()));
    }

#ifdef GOLDENLAND_FPS_LIMIT
    SDL_SetRenderVSync(m_renderer, 1);
#endif

    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(m_window);
}

void Application::initImGui(std::string_view fontFilepath, int fontSize) {
    const char* imguiBranch =
#ifdef IMGUI_HAS_DOCK
        "docking";
#else
        "master";
#endif

    LogFmt("[INFO] ImGui Version: {} (branch: {})", IMGUI_VERSION, imguiBranch);
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;

    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 3.0f;

    style.WindowTitleAlign.x = 0.5f;
    style.WindowTitleAlign.y = 0.75f;

    ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer3_Init(m_renderer);

    io.Fonts->Flags |= ImFontAtlasFlags_NoMouseCursors;

    // Встраиваемый шрифт
    using namespace std::literals::string_view_literals;
    auto fontName = "EmbeddedCarlito"sv;

    ImFontConfig embeddedFontConfig;
    std::copy(fontName.cbegin(), fontName.cend(), embeddedFontConfig.Name);
    embeddedFontConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)carlito_ttf, carlito_ttf_size, 0.0f, &embeddedFontConfig);

    // Загрузка шрифта и его размера
    if (!fontFilepath.empty()) {
        ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontFilepath.data(), fontSize);
        io.FontDefault = newFont;
    }

    style.FontSizeBase = fontSize;
    style._NextFrameFontSizeBase = style.FontSizeBase;
}

bool Application::hasActiveAnimations() const {
    if (m_rootDirContext.isLoading()) {
        return true;
    }
    if (m_mdfViewer.isAnimating()) {
        return true;
    }
    for (const auto& level : m_rootDirContext.levels) {
        if (m_levelViewer.isAnimating(level)) {
            return true;
        }
    }
    return false;
}

void Application::mainLoop() {
    ImGuiIO& io = ImGui::GetIO();
    std::string uiError;
    std::string aboutMessage = std::format("Arch: {}\n"
                                           "Compiler: {}\n"
                                           "\n"
                                           "3rd-party libraries:\n"
                                           "SDL v{}.{}.{}\n"
                                           "ImGui v{}\n"
                                           "\n"
                                           "GoldenLand Editor v{}\n"
                                           "© DarkContact 2026\n",
                                           BX_ARCH_NAME,
                                           BX_COMPILER_NAME,
                                           SDL_MAJOR_VERSION,
                                           SDL_MINOR_VERSION,
                                           SDL_MICRO_VERSION,
                                           IMGUI_VERSION,
                                           GOLDENLAND_VERSION_STRING);

    bool showLevelsWindow = false;
    bool showSettingsWindow = false;
    bool showAboutWindow = false;

#ifdef DEBUG_MENU_ENABLE
    struct TestResult {
        std::string filepath;
        std::string status;
        std::string errorMessage;
        float percent;
    };
    static std::vector<TestResult> testResults;
#endif

    while (!m_done)
    {
        bool noWait = true;
#ifdef GOLDENLAND_FPS_LIMIT
        noWait = (m_renderCooldown > 0);
#endif
        bool hasEvents = processEvents(noWait);

        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F11, false)) {
            bool isFullscreen = SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;
            SDL_SetWindowFullscreen(m_window, !isFullscreen);
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());  

        bool loaderWindow = m_rootDirContext.isLoading();
        ImGuiWidgets::Loader("Loading...", loaderWindow);
        ImGuiWidgets::ShowMessageModal("Error", uiError);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::BeginDisabled(m_rootDirContext.isEmptyContext());

                bool levelsDisabled = m_rootDirContext.singleLevelNames().empty()
                                      && m_rootDirContext.multiplayerLevelNames().empty();
                if (ImGui::MenuItem("Load Level...", NULL, false, !levelsDisabled)) {
                    showLevelsWindow = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("CSX Viewer", NULL, false, !m_rootDirContext.csxFiles().empty())) {
                    m_rootDirContext.showCsxWindow = true;
                }
                if (ImGui::MenuItem("SDB Viewer", NULL, false, !m_rootDirContext.sdbFiles().empty())) {
                    m_rootDirContext.showSdbWindow = true;
                }
                if (ImGui::MenuItem("MDF Viewer", NULL, false, !m_rootDirContext.mdfFiles().empty())) {
                    m_rootDirContext.showMdfWindow = true;
                }
                if (ImGui::MenuItem("CS Viewer", NULL, false, !m_rootDirContext.csFiles().empty())) {
                    m_rootDirContext.showCsWindow = true;
                }

                ImGui::EndDisabled();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Set root folder...")) {
                    // TODO: Сделать модальное окно с отображением текущей директории
                    SDL_ShowOpenFolderDialog([] (void* userdata, const char* const* filelist, int filter) {
                        if (!filelist) {
                            LogFmt("Folder dialog error: {}", SDL_GetError());
                            return;
                        } else if (!*filelist) {
                            Log("Dialog was canceled");
                            return;
                        } else if ((*filelist)[0] == '\0') {
                            Log("Filelist empty");
                            return;
                        }

                        if (*filelist) {
                            LogFmt("Selected folder: {}", *filelist);
                            Application* app = static_cast<Application*>(userdata);
                            if (app->m_rootDirContext.rootDirectory() == *filelist) return;

                            app->m_rootDirContext.setRootDirectoryAndReload(*filelist);
                        }
                    }, this, m_window, m_rootDirContext.rootDirectory().data(), false);
                }
                if (ImGui::MenuItem("Font settings")) {
                    showSettingsWindow = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window")) {
                bool isFullscreen = SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;
                if (ImGui::MenuItem("Fullscreen", "F11", isFullscreen)) {
                    SDL_SetWindowFullscreen(m_window, !isFullscreen);
                }

                ImGui::EndMenu();
            }

#ifdef DEBUG_MENU_ENABLE
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Load all levels")) {
                    for (const auto& levelName : m_rootDirContext.singleLevelNames()) {
                        std::string error;
                        auto level = Level::loadLevel(m_renderer, m_rootDirContext.rootDirectory(), levelName, LevelType::kSingle, &error);
                        if (level) {
                            m_rootDirContext.levels.push_back(std::move(*level));
                        } else {
                            uiError = std::move(error);
                        }
                    }
                }

                if (ImGui::MenuItem("Test all dialogs")) {
                    testResults.clear();
                    testResults.reserve(m_rootDirContext.csFiles().size());

                    std::unordered_map<std::string, std::vector<DialogInstruction>> manualCases = getManualDialogTestData();
                    for (const auto& csFile : m_rootDirContext.csFiles()) {
                        std::string csError;
                        CS_Data csData;

                        std::string csPath = std::format("{}/{}", m_rootDirContext.rootDirectory(), csFile);
                        LogFmt("Cs file processing: {}", csFile);
                        bool isOkParse = CS_Parser::parse(csPath, csData, &csError);

                        if (!isOkParse) {
                            LogFmt("CS_Parser error: {}", csError);
                            testResults.push_back({csFile, "IncorrectFormat", csError, 100.0f});
                            continue;
                        }

                        CsExecutor executor(csData.nodes, m_rootDirContext.globalVars());
                        if (auto it = manualCases.find(csFile); it != manualCases.end()) {
                            const std::vector<DialogInstruction>& instructions = it->second;
                            for (const auto& inst : instructions) {
                                switch (inst.inst) {
                                    case kExec: {
                                        while (executor.next());
                                        break;
                                    }

                                    case kUserInputAndExec: {
                                        assert(executor.currentStatus() == CsExecutor::kWaitUser);
                                        executor.userInput(inst.value);
                                        while (executor.next());
                                        break;
                                    }

                                    case kSetVariable: {
                                        if (auto it = executor.scriptVars().find(inst.text); it != executor.scriptVars().end()) {
                                            it->second = inst.value;
                                        } else {
                                            LogFmt("kSetVariable variable '{}' not found", inst.text);
                                        }
                                        break;
                                    }

                                    case kSoftRestart: {
                                        executor.restart(false);
                                        break;
                                    }

                                    case kHardRestart: {
                                        executor.restart(true);
                                        break;
                                    }
                                }
                            }
                            testResults.push_back({csFile, executor.currentStatusString(), {}, executor.executedPercent()});
                        } else {
                            // Алгоритм тестирования диалогов:
                            // 1. Пытаемся прокликать первые варианты ответа
                            //    Если попали в бесконечный цикл выбираем последний ответ и выходим
                            // 2. Делаем 10 попыток проклика первых ответов после завершения диалога
                            //    (Т.к. меняются переменные и возможны новые пути выполнения)
                            try {
                                float executedPercent = executor.executedPercent();
                                uint8_t answer = 1;
                                int tryCount = 10;

                                while (tryCount) {
                                    while (executor.currentStatus() != CsExecutor::kEnd
                                           && executor.currentStatus() != CsExecutor::kInfinity)
                                    {
                                        while (executor.next()) {}

                                        if (executedPercent == executor.executedPercent()) { // Прогресс остановился, вероятно попали в цикл
                                            if (executor.dialogsAnswersCount() >= 2) {
                                                answer = executor.dialogsAnswersCount();
                                            } else {
                                                answer = 1;
                                            }
                                        }

                                        if (executor.currentStatus() == CsExecutor::kWaitUser) {
                                            executor.userInput(answer);
                                        }

                                        executedPercent = executor.executedPercent();
                                    }
                                    executor.restart(false);
                                    tryCount--;
                                }

                                testResults.push_back({csFile, executor.currentStatusString(), {}, executor.executedPercent()});
                            } catch (const std::exception& ex) {
                                testResults.push_back({csFile, "FatalError", ex.what(), 0.0f});
                            }
                        }
                    }
                }

                ImGui::EndMenu();
            }
#endif

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    showAboutWindow = true;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (!m_rootDirContext.isLoading()) {
            if (m_rootDirContext.isEmptyContext()) {
                ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration
                    | ImGuiWindowFlags_AlwaysAutoResize
                    | ImGuiWindowFlags_NoSavedSettings
                    | ImGuiWindowFlags_NoFocusOnAppearing
                    | ImGuiWindowFlags_NoNav
                    | ImGuiWindowFlags_NoMove;

                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowBgAlpha(0.75f);
                if (ImGui::Begin("##WarningEmptyRootContext", nullptr, windowFlags))
                {
                    ImGui::Text("Root folder not specified!\n"
                                "Root folder is a directory with unpacked game resources\n"
                                "(levels, scripts, etc.)\n"
                                "\n"
                                "Set it via 'Settings -> Set root folder...'\n"
                                "\n"
                                "Or edit settings.ini:\n"
                                "[resources]\n"
                                "root_dir = <path_to_root_folder>");
                }
                ImGui::End();
            }

            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_csxViewer.update(m_rootDirContext.showCsxWindow, m_renderer, m_rootDirContext.rootDirectory(), m_rootDirContext.csxFiles());
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_sdbViewer.update(m_rootDirContext.showSdbWindow, m_rootDirContext.rootDirectory(), m_rootDirContext.sdbFiles());
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_mdfViewer.update(m_rootDirContext.showMdfWindow, m_renderer, m_rootDirContext.rootDirectory(), m_rootDirContext.mdfFiles());
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_csViewer.update(m_rootDirContext.showCsWindow, m_rootDirContext.rootDirectory(), m_rootDirContext.csFiles(),
                              m_rootDirContext.dialogPhrases(), m_rootDirContext.globalVars());

            if (showSettingsWindow) {
                m_fontSettings->update(showSettingsWindow);
            }

            if (showAboutWindow) {
                showAboutWindow = ImGuiWidgets::ShowMessageModalEx("About", [&aboutMessage] () {
                    ImGui::TextLinkOpenURL("GitHub repository", "https://github.com/DarkContact/GoldenLandEditor");
                    ImGui::TextUnformatted(aboutMessage.data(), aboutMessage.data() + aboutMessage.size());
                });
            }

#ifdef DEBUG_MENU_ENABLE
            if (!testResults.empty()) {
                if (ImGui::Begin("Dialog test result")) {
                    int fatals = 0;
                    int lowPercent = 0;
                    float totalPercents = 0.0f;
                    if (ImGui::BeginTable("Main Table", 2, ImGuiTableFlags_Borders)) {

                        ImGui::TableSetupColumn("Dialog");
                        ImGui::TableSetupColumn("Result");
                        ImGui::TableHeadersRow();

                        int id = 0;
                        for (const auto& [filename, status, errorMessage, percent] : testResults) {
                            ImGui::TableNextRow();

                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(filename.data(), filename.data() + filename.size());

                            ImGui::PushID(id++);
                            if (ImGui::BeginPopupContextItem("filename context menu")) {
                                if (ImGui::MenuItem("Copy")) {
                                    ImGui::SetClipboardText(filename.data());
                                }
                                if (ImGui::MenuItem("Explorer")) {
                                    std::string dialogFile = std::format("{}/{}", m_rootDirContext.rootDirectory(), filename);
                                    std::array<std::string_view, 1> files = {dialogFile};
                                    std::string error;
                                    std::filesystem::path dialogFilePath(StringUtils::toUtf8View(dialogFile));
                                    if (!FileUtils::openFolderAndSelectItems(StringUtils::fromUtf8View(dialogFilePath.parent_path().u8string()), files, &error)) {
                                        Log(error);
                                    }
                                }
                                ImGui::EndPopup();
                            }
                            ImGui::PopID();

                            ImGui::TableNextColumn();
                            ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f);
                            if (percent == 0.0f) {
                                color = ImVec4(0.98f, 0.0f, 0.0f, 1.0f);
                                fatals++;
                            } else if (percent <= 50.0f) {
                                color = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
                                lowPercent++;
                            } else if (percent <= 75.0f) {
                                color = ImVec4(0.9f, 0.9f, 0.1f, 1.0f);
                            } else if (percent <= 100.0f) {
                                color = ImVec4(0.1f, 0.9f, 0.1f, 1.0f);
                            }
                            ImGui::TextColored(color, "%s", std::format("Status: {} ({:.2f} %)", status, percent).c_str());
                            if (!errorMessage.empty()) {
                                ImGui::Text("%s", errorMessage.c_str());
                            }

                            totalPercents += percent;
                        }
                        ImGui::EndTable();
                    }

                    ImGui::Text("Total percents: %.2f / %.2f (%.2f %%)", totalPercents, testResults.size() * 100.0f,
                                (totalPercents / (testResults.size() * 100.0f)) * 100.0f);
                    ImGui::Separator();
                    ImGui::Text("Total: %zu", testResults.size());
                    ImGui::Text("Fatals: %d", fatals);
                    ImGui::Text("Low percent: %d", lowPercent);
                }
                ImGui::End();
            }
#endif
        }

        // NOTE: Для генерации озвучки
        // static bool csViewerOnce = false;
        // if (!m_rootDirContext.isBusy() && !csViewerOnce) {
        //     m_csViewer.injectPlaySoundAndGeneratePhrases("C:/Games/Холодные Небеса", m_rootDirContext.rootDirectory(), m_rootDirContext.csFiles());
        //     csViewerOnce = true;
        // }

        if (showLevelsWindow && !m_rootDirContext.singleLevelNames().empty()) {
            if (auto result = m_levelPicker.update(showLevelsWindow,
                                                   m_renderer,
                                                   m_rootDirContext.rootDirectory(),
                                                   m_rootDirContext.singleLevelNames(),
                                                   m_rootDirContext.multiplayerLevelNames(),
                                                   m_rootDirContext.levelHumanNamesDict(),
                                                   m_rootDirContext.selectedLevelIndex); result.selected) {
                bool alreadyLoaded = false;
                for (const auto& level : m_rootDirContext.levels) {
                    if (level.data().name == result.loadedLevelName && level.data().type == result.loadedLevelType) {
                        alreadyLoaded = true;
                        break;
                    }
                }

                if (alreadyLoaded) {
                    ImGui::SetWindowFocus(Level::levelWindowName(result.loadedLevelName, result.loadedLevelType).c_str());
                } else {
                    // Загрузка уровня
                    std::string error;
                    auto level = Level::loadLevel(m_renderer, m_rootDirContext.rootDirectory(), result.loadedLevelName, result.loadedLevelType, &error);
                    if (level) {
                        m_rootDirContext.levels.push_back(std::move(*level));
                    } else {
                        uiError = std::move(error);
                    }
                }
            }
        }

        if (!m_rootDirContext.isLoading()) {
            for (auto it = m_rootDirContext.levels.begin(); it != m_rootDirContext.levels.end();) {
                bool openLevel = true;
                Level& level = *it;
                if (level.data().background) {
                    ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
                    m_levelViewer.update(openLevel, m_rootDirContext.rootDirectory(), level);
                    if (!openLevel) {
                        it = m_rootDirContext.levels.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }

#ifdef GOLDENLAND_FPS_LIMIT
        if (hasActiveAnimations() || hasEvents) {
            m_renderCooldown = kRenderCooldownFrames;
        } else if (m_renderCooldown > 0) {
            m_renderCooldown--;
        }
#endif

        render();
    }
}

bool Application::processEvents(bool noWait) {
    SDL_Event event;
    bool hasEvent = noWait ? SDL_PollEvent(&event)
                           : SDL_WaitEventTimeout(&event, kWaitTimeoutMs);

    bool hasEvents = hasEvent;
    while (hasEvent) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT)
            m_done = true;

        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(m_window))
        {
            m_done = true;
        }

        hasEvent = SDL_PollEvent(&event);
    }
    return hasEvents;
}

void Application::render() {
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();

    Tracy_VideoMemoryPlot(m_renderer);
    {
        Tracy_ZoneScopedN("Rendering prepare");
        Tracy_ZoneColor(0xadff2f); // GreenYellow

        ImGui::Render();

        SDL_SetRenderScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(m_renderer, clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        SDL_RenderClear(m_renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    }

    {
        Tracy_ZoneScopedN("CaptureImage");
        Tracy_ZoneColor(0xc0c0c0); // Silver

        Tracy_CaptureImage(m_renderer);
    }

    {
        Tracy_ZoneScopedN("Rendering");
        Tracy_ZoneColor(0x32cd32); // LimeGreen

        SDL_RenderPresent(m_renderer);
    }

    Tracy_FrameMark;
}

void Application::shutdown() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
