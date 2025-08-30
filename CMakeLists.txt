cmake_minimum_required(VERSION 3.16)
project(GoldenLandEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
include(external/imgui.cmake)

add_executable(GoldenLandEditor
    src/main.cpp
    src/windows/FontSettings.h
    src/windows/FontSettings.cpp
    src/Resources.h
    src/Resources.cpp
    src/ImguiHelper.h
    src/ImguiHelper.cpp
    src/windows/LevelPicker.h
    src/windows/LevelPicker.cpp
    src/parsers/SEF_Parser.h
    src/parsers/SEF_Parser.cpp
    src/Level.h
    src/Level.cpp
    src/StringUtils.h
    src/StringUtils.cpp
    src/TextureLoader.h
    src/TextureLoader.cpp
    src/parsers/CSX_Parser.h
    src/parsers/CSX_Parser.cpp
)

target_include_directories(GoldenLandEditor PRIVATE src)
target_include_directories(GoldenLandEditor PRIVATE external/mINI)
target_include_directories(GoldenLandEditor PRIVATE external/stb)
target_include_directories(GoldenLandEditor PRIVATE imgui)
target_link_libraries(GoldenLandEditor PRIVATE SDL3::SDL3 imgui)

add_custom_target(Docs ALL
    SOURCES ${CMAKE_SOURCE_DIR}/README.md
)
