cmake_minimum_required(VERSION 3.16)
project(GoldenLandEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GOLDENLAND_ENABLE_TRACY  "Enable Tracy profiler" OFF)
option(GOLDENLAND_ENABLE_OPENMP "Enable OpenMP"         ON)

# Требует большего тестирования. Нужно понимание как подружить это с CMAKE_PREFIX_PATH
# if(WIN32 AND NOT SDL3_DIR)
#     if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#         set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3_MinGW/cmake" CACHE PATH "Path to SDL3")
#     elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#         set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3_MSVC/cmake" CACHE PATH "Path to SDL3")
#     endif()
# endif()

find_package(SDL3 REQUIRED CONFIG)
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
