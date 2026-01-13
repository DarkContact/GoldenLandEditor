enable_testing()

add_executable(GoldenLandEditorTests
    main.cpp
    StringUtilsTest.h

    ../src/utils/StringUtils.cpp
)

target_link_libraries(
    GoldenLandEditorTests
    GTest::gtest_main
)

target_include_directories(GoldenLandEditorTests PRIVATE ../src)

include(GoogleTest)
gtest_discover_tests(GoldenLandEditorTests)
