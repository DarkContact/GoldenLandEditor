cmake_minimum_required(VERSION 3.16)
project(GoldenLandEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
include(external/imgui.cmake)

add_executable(GoldenLandEditor main.cpp)

target_include_directories(GoldenLandEditor PUBLIC imgui)
target_link_libraries(GoldenLandEditor PRIVATE SDL3::SDL3 imgui)

add_custom_target(Docs ALL
    SOURCES ${CMAKE_SOURCE_DIR}/README.md
)
