cmake_minimum_required(VERSION 3.16)

project(GoldenLandEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GoldenLandEditor main.cpp)

include(GNUInstallDirs)
install(TARGETS GoldenLandEditor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
