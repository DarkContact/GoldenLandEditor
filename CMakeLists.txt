cmake_minimum_required(VERSION 3.16)

# Гарантируем пересборку CMake при изменении файла версии
set(VERSION_FILE "${CMAKE_SOURCE_DIR}/VERSION")
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${VERSION_FILE}")

# Считываем версию проекта
file(STRINGS "${VERSION_FILE}" VERSION_FILE_DATA LIMIT_COUNT 1)
project(GoldenLandEditor VERSION ${VERSION_FILE_DATA})
message(STATUS "Project version: ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GOLDENLAND_ENABLE_TRACY              "Enable Tracy profiler"   OFF)
option(GOLDENLAND_ENABLE_OPENMP             "Enable OpenMP"           OFF)
option(GOLDENLAND_ENABLE_DEPENDENCY_HINTS   "Enable dependency hints" ON)
option(GOLDENLAND_ENABLE_STATIC_RUNTIME     "Enable static runtime"   OFF)
option(GOLDENLAND_ENABLE_COPY_DLL           "Enable copy dll to exe"  OFF)
option(GOLDENLAND_ENABLE_FPS_LIMIT          "Enable FPS limit in app" ON)
option(GOLDENLAND_BUILD_TESTS               "Build tests"             OFF)

# SDL3 Hint
if(WIN32)
    if(MINGW OR CYGWIN)
        set(SDL3_SEARCH_HINT "${CMAKE_SOURCE_DIR}/external/SDL3_MinGW/cmake")
    elseif(MSVC)
        set(SDL3_SEARCH_HINT "${CMAKE_SOURCE_DIR}/external/SDL3_MSVC/cmake")
    endif()

    if(GOLDENLAND_ENABLE_DEPENDENCY_HINTS AND NOT EXISTS "${SDL3_SEARCH_HINT}/SDL3Config.cmake")
        message(WARNING "SDL3_SEARCH_HINT path not exist: ${SDL3_SEARCH_HINT}\n"
                        "You can download SDL3 by running: external/get_SDL3.py\n"
                        "(Or set GOLDENLAND_ENABLE_DEPENDENCY_HINTS=OFF to suppress this message)")
    endif()
endif()

# SDL3
find_package(SDL3 REQUIRED CONFIG PATHS ${SDL3_SEARCH_HINT})
message(STATUS "Found SDL3: ${SDL3_DIR} (version: ${SDL3_VERSION})")

# ImGui
include(external/imgui.cmake)

# stb_image
include(external/stb_image.cmake)

add_executable(GoldenLandEditor WIN32
    src/main.cpp
    src/windows/FontSettings.h
    src/windows/FontSettings.cpp
    src/Resources.h
    src/Resources.cpp
    src/utils/ImGuiWidgets.h
    src/utils/ImGuiWidgets.cpp
    src/windows/LevelPicker.h
    src/windows/LevelPicker.cpp
    src/parsers/SEF_Parser.h
    src/parsers/SEF_Parser.cpp
    src/Level.h
    src/Level.cpp
    src/utils/StringUtils.h
    src/utils/StringUtils.cpp
    src/utils/TextureLoader.h
    src/utils/TextureLoader.cpp
    src/parsers/CSX_Parser.h
    src/parsers/CSX_Parser.cpp
    src/windows/CsxViewer.h
    src/windows/CsxViewer.cpp
    src/windows/LevelViewer.h
    src/windows/LevelViewer.cpp
    src/parsers/LVL_Parser.h
    src/parsers/LVL_Parser.cpp
    src/parsers/SDB_Parser.h
    src/parsers/SDB_Parser.cpp
    src/utils/FileUtils.h
    src/utils/FileUtils.cpp
    src/windows/SdbViewer.h
    src/windows/SdbViewer.cpp
    src/utils/TracyProfiler.h
    src/Texture.h
    src/Texture.cpp
    src/utils/DebugLog.h
    src/utils/DebugLog.cpp
    src/parsers/LAO_Parser.h
    src/parsers/LAO_Parser.cpp
    src/Types.h
    src/Settings.h
    src/Settings.cpp
    src/parsers/MDF_Parser.h
    src/parsers/MDF_Parser.cpp
    src/windows/MdfViewer.h
    src/windows/MdfViewer.cpp
    src/utils/IoUtils.h
    src/utils/IoUtils.cpp
    src/utils/AnimationCachedLoader.h
    src/utils/AnimationCachedLoader.cpp
    src/parsers/CS_Parser.h
    src/parsers/CS_Parser.cpp
    src/windows/CsViewer.h
    src/windows/CsViewer.cpp
    src/CsExecutor.h
    src/CsExecutor.cpp
    src/windows/CsExecutorViewer.h
    src/windows/CsExecutorViewer.cpp
    src/enums/CsFunctions.h
    src/enums/CsFunctions.cpp
    src/enums/CsOpcodes.h
    src/enums/CsOpcodes.cpp
    src/Application.h
    src/Application.cpp
    src/RootDirectoryContext.h
    src/RootDirectoryContext.cpp
    src/utils/Platform.h
    src/utils/RenderUtils.h
    src/utils/RenderUtils.cpp
    src/Cache.h
)

target_sources(GoldenLandEditor PRIVATE
    "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/VERSION"
)
set_source_files_properties(
    "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/VERSION"
    PROPERTIES HEADER_FILE_ONLY TRUE
)

set_source_files_properties(
    src/Application.cpp
    PROPERTIES COMPILE_DEFINITIONS
    GOLDENLAND_VERSION_STRING="${PROJECT_VERSION}"
)

# Файлы ресурсов для Windows
if(WIN32)
    set(ICON_PATH "${CMAKE_SOURCE_DIR}/resources/win/icon.ico")

    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/win/resources.rc.in"
        "${CMAKE_BINARY_DIR}/resources.rc"
        @ONLY
    )
    target_sources(GoldenLandEditor PRIVATE "${CMAKE_BINARY_DIR}/resources.rc")
endif()

# Встроенные ресурсы
include(resources/EmbeddedResources.cmake)
add_embedded_resources(resources_lib "${CMAKE_SOURCE_DIR}/resources/embedded" "embedded_resources.h")
target_link_libraries(GoldenLandEditor PRIVATE resources_lib)

target_include_directories(GoldenLandEditor PRIVATE src)
target_include_directories(GoldenLandEditor PRIVATE external/mINI)
target_include_directories(GoldenLandEditor PRIVATE imgui)

target_link_libraries(GoldenLandEditor PRIVATE
    SDL3::SDL3
    imgui
    stb_image
)

if(GOLDENLAND_ENABLE_STATIC_RUNTIME)
    message(STATUS "Building with static runtime enabled")

    if(MINGW OR CYGWIN)
        set(STATIC_OPTIONS -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic)
        target_link_options(GoldenLandEditor PRIVATE ${STATIC_OPTIONS})
        target_link_options(imgui PRIVATE ${STATIC_OPTIONS})
    elseif(MSVC)
        set(STATIC_OPTIONS "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET GoldenLandEditor PROPERTY MSVC_RUNTIME_LIBRARY ${STATIC_OPTIONS})
        set_property(TARGET imgui PROPERTY MSVC_RUNTIME_LIBRARY ${STATIC_OPTIONS})
    endif()
endif()

if(GOLDENLAND_ENABLE_TRACY)
    include(FetchContent)

    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG 05cceee0df3b8d7c6fa87e9638af311dbabc63cb # v13.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(tracy)

    target_compile_definitions(GoldenLandEditor PRIVATE DEBUG_LOG_ENABLE)
    target_sources(GoldenLandEditor PRIVATE src/utils/TracyProfiler.cpp)

    option(TRACY_ENABLE "" ON)
    option(TRACY_ON_DEMAND "" ON)
    target_link_libraries(GoldenLandEditor PUBLIC Tracy::TracyClient)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-g -fno-omit-frame-pointer)
    elseif(MSVC)
        add_compile_definitions(TRACY_CALLSTACK=24)

        get_target_property(SDL3_DLL_PATH SDL3::SDL3-shared IMPORTED_LOCATION)
        get_filename_component(SDL3_DLL_DIR "${SDL3_DLL_PATH}" DIRECTORY)
        set(SDL3_PDB_PATH "${SDL3_DLL_DIR}/SDL3.pdb")

        add_custom_command(TARGET GoldenLandEditor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SDL3_PDB_PATH}
                $<TARGET_FILE_DIR:GoldenLandEditor>
            VERBATIM
        )
    endif()
endif()

if(GOLDENLAND_ENABLE_OPENMP)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(GoldenLandEditor PRIVATE -fopenmp)
        target_link_libraries(GoldenLandEditor PRIVATE libgomp)
    elseif(MSVC)
        target_compile_options(GoldenLandEditor PRIVATE /openmp)
    endif()
endif()

if(GOLDENLAND_ENABLE_COPY_DLL AND WIN32)
    add_custom_command(
        TARGET GoldenLandEditor POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:GoldenLandEditor>
        VERBATIM
    )
endif()

if(GOLDENLAND_ENABLE_FPS_LIMIT)
    set_property(
        SOURCE src/Application.cpp
        APPEND PROPERTY COMPILE_DEFINITIONS
            GOLDENLAND_FPS_LIMIT
    )
endif()

if(GOLDENLAND_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(test)
endif()
