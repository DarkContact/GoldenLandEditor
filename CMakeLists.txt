cmake_minimum_required(VERSION 3.16)
project(GoldenLandEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
include(external/imgui.cmake)

add_executable(GoldenLandEditor WIN32
    src/main.cpp
    src/windows/FontSettings.h
    src/windows/FontSettings.cpp
    src/Resources.h
    src/Resources.cpp
    src/utils/ImguiHelper.h
    src/utils/ImguiHelper.cpp
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
    src/utils/FileLoader.h
    src/utils/FileLoader.cpp
    src/windows/SdbViewer.h
    src/windows/SdbViewer.cpp
    src/utils/TracyProfiler.h
    src/utils/TracyProfiler.cpp
    src/Texture.h
    src/Texture.cpp
    src/utils/DebugLog.h
    src/utils/DebugLog.cpp
)

option(USE_TRACY "Enable Tracy profiler" OFF)
if(USE_TRACY)
    include(FetchContent)

    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(tracy)

    option(TRACY_ENABLE "" ON)
    option(TRACY_ON_DEMAND "" ON)
    target_link_libraries(GoldenLandEditor PUBLIC Tracy::TracyClient)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-g -fno-omit-frame-pointer -O2)
    endif()
    # add_compile_definitions(TRACY_CALLSTACK=16)
endif()

target_include_directories(GoldenLandEditor PRIVATE src)
target_include_directories(GoldenLandEditor PRIVATE external/mINI)
target_include_directories(GoldenLandEditor PRIVATE external/stb)
target_include_directories(GoldenLandEditor PRIVATE imgui)
target_link_libraries(GoldenLandEditor PRIVATE SDL3::SDL3 imgui)

# OpenMP
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(GoldenLandEditor PRIVATE -fopenmp)
    target_link_libraries(GoldenLandEditor PRIVATE libgomp)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(GoldenLandEditor PRIVATE /openmp)
endif()

add_custom_target(Docs ALL
    SOURCES ${CMAKE_SOURCE_DIR}/README.md
)
