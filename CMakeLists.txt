cmake_minimum_required(VERSION 3.16)
project(GoldenLandEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GOLDENLAND_ENABLE_TRACY  "Enable Tracy profiler" OFF)
option(GOLDENLAND_ENABLE_OPENMP "Enable OpenMP"         ON)

# Попытка найти SDL3 стандартными путями или через CMAKE_PREFIX_PATH / SDL3_DIR
find_package(SDL3 QUIET CONFIG)

if(WIN32 AND NOT SDL3_FOUND)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(_SDL3_HINT "${CMAKE_SOURCE_DIR}/external/SDL3_MinGW/cmake")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(_SDL3_HINT "${CMAKE_SOURCE_DIR}/external/SDL3_MSVC/cmake")
    endif()

    if(EXISTS "${_SDL3_HINT}/SDL3Config.cmake")
        set(SDL3_DIR "${_SDL3_HINT}")
    else()
        message(WARNING "SDL3 not found! Fallback path not exist: ${_SDL3_HINT}\n"
            "You can download SDL3 by running: external/get_SDL3.py\n"
            "Alternatively, specify SDL3_DIR to the folder containing SDL3Config.cmake, "
            "or add the SDL3 installation path to CMAKE_PREFIX_PATH.")
    endif()
endif()

find_package(SDL3 REQUIRED CONFIG)
message(STATUS "Found SDL3: ${SDL3_DIR} (version: ${SDL3_VERSION})")

include(external/imgui.cmake)

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
    src/files/BgMdfFile.h
    src/utils/IoUtils.h
    src/utils/IoUtils.cpp
    src/utils/AnimationCachedLoader.h
    src/utils/AnimationCachedLoader.cpp
)

target_sources(GoldenLandEditor PRIVATE "${CMAKE_SOURCE_DIR}/README.md")
set_source_files_properties("${CMAKE_SOURCE_DIR}/README.md" PROPERTIES HEADER_FILE_ONLY TRUE)

target_include_directories(GoldenLandEditor PRIVATE src)
target_include_directories(GoldenLandEditor PRIVATE external/mINI)
target_include_directories(GoldenLandEditor PRIVATE external/stb)
target_include_directories(GoldenLandEditor PRIVATE imgui)
target_link_libraries(GoldenLandEditor PRIVATE SDL3::SDL3 imgui)

if(GOLDENLAND_ENABLE_TRACY)
    include(FetchContent)

    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(tracy)

    target_sources(GoldenLandEditor PRIVATE src/utils/TracyProfiler.cpp)

    option(TRACY_ENABLE "" ON)
    option(TRACY_ON_DEMAND "" ON)
    target_link_libraries(GoldenLandEditor PUBLIC Tracy::TracyClient)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-g -fno-omit-frame-pointer -O2)
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_definitions(TRACY_CALLSTACK=24)
    endif()
endif()

if(GOLDENLAND_ENABLE_OPENMP)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(GoldenLandEditor PRIVATE -fopenmp)
        target_link_libraries(GoldenLandEditor PRIVATE libgomp)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(GoldenLandEditor PRIVATE /openmp)
    endif()
endif()
