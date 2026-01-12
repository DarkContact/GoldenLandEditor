#pragma once
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>
#include <string>

#include "utils/StringUtils.h"

TEST(NaturalCompare, SimpleWithLeadingZero) {
    std::vector<std::string> result = {
        "0001",
        "01",
        "1",
        "2",
        "3",
        "11",
        "30"
    };

    std::vector<std::string> data = result;
    std::sort(data.begin(), data.end(), StringUtils::naturalCompare);
    EXPECT_EQ(data, result);
}

TEST(NaturalCompare, SimpleWithSpaces) {
    std::vector<std::string> result = {
        "0 ab",
        "0ab",
        "a 1",
        "a 10",
        "a 11",
        "a b",
        "a_000",
        "a_00",
        "a_0",
        "ab"
    };

    std::vector<std::string> data = result;
    std::sort(data.begin(), data.end(), StringUtils::naturalCompare);
    EXPECT_EQ(data, result);
}

TEST(NaturalCompare, Overflow) {
    std::vector<std::string> result = {
        "a_11",
        "a_1118446744073709",
        "a_1118446744073709551610",
        "a_1118446744073709551615",
        "a_1118446744073709551620",
        "a_18446744073709551614",
        "a_18446744073709551615", // uint64 max
        "a_18446744073709551616"
    };

    std::vector<std::string> data = result;
    std::sort(data.begin(), data.end(), StringUtils::naturalCompare);
    EXPECT_EQ(data, result);
}

TEST(NaturalCompare, Gold2) {
    std::vector<std::string> result = {
        "l1_1", "l1_1_1", "l1_1_3", "l1_1_4",
        "l1_2", "l1_2_1", "l1_2_2", "l1_2_3", "l1_2_4", "l1_2_5", "l1_2_6", "l1_2_8", "l1_2_10",
        "l1_3",
        "l2_1", "l2_1_2", "l2_1_3", "l2_1_4", "l2_2_1", "l2_2_2", "l2_2_3",
        "l3_1", "l3_1_2", "l3_1_3", "l3_1_4",
        "l3_2", "l3_2_1", "l3_2_2", "l3_2_3",
        "l3_3", "l3_3_1", "l3_3_3", "l3_3_4", "l3_3_5",
        "l4_1", "l4_2", "l4_2_1", "l4_2_2", "l4_2_3",
        "l5_1", "l5_1_1", "l5_1_2", "l5_2", "l5_2_3",
        "l6_1", "l6_1_1", "l6_1_2", "l6_1_3",
        "l6_2", "l6_2_1", "l6_2_2", "l6_3",
        "l7_1", "l7_1_1", "l7_1_2", "l7_2", "l7_2_1", "l7_2_2", "l7_2_3", "l7_2_4", "l7_2_5", "l7_2_9", "l7_2_10",
        "l8_1", "l8_1_1", "l8_1_2",
        "l9_1", "l9_1_1", "l9_1_2", "l9_1_3", "l9_1_4_1", "l9_1_4_2", "l9_2", "l9_2_1", "l9_2_3", "l9_2_4", "l9_3", "l9_3_2", "l9_3_4",
        "l10_1", "l10_1_1", "l10_1_2", "l10_1_3", "l10_1_4", "l10_1_5", "l10_1_6", "l10_1_7", "l10_2", "l10_2_1", "l10_5", "l10_5_1",
        "l11_1", "l11_1_1", "l11_1_2", "l11_1_3", "l11_1_4", "l11_2", "l11_2_1", "l11_2_2", "l11_2_3",
        "l12_1", "l12_1_1", "l12_1_2", "l12_1_3", "l12_2", "l12_2_1", "l12_2_2", "l12_2_3", "l12_2_4", "l12_3", "l12_3_1", "l12_3_2",
        "l13_1", "l13_1_1", "l13_1_2", "l13_2",
        "l14_1", "l14_1_1", "l14_1_3", "l14_1_4", "l14_2", "l14_3",
        "l15",
        "l18_1", "l18_2", "l19", "l19_2", "l19_3", "l19_4",
        "l21_1", "l21_2", "l22", "l23", "l24", "l25_1", "l25_2", "l26", "l27", "l28", "l28_1", "l28_2", "l29_1", "l29_2", "l31_2",
        "l32", "l32_1_1", "l32_1_2", "l32_1_3", "l32_1_4", "l33_1", "l33_2", "l34_1", "l34_1_1", "l35",
        "l36", "l36_1", "l36_1_1", "l36_1_2", "l37_1", "l37_3", "l40_2", "l41", "l42", "l42_1", "l43_1", "l43_2", "l48", "l49",
        "l52_1", "l52_2", "l53", "l54", "l56", "l57", "l64", "l65", "l65_1", "l66", "l68", "l69", "l69_1", "l73_a", "l73_a_1", "l73_a_2",
        "l75", "l78", "l83", "l89_1_a", "l89_3", "l89_3_2", "l91", "l91_1_1", "l93_3", "l93_4", "l94", "l96", "l97", "l99_1", "l99_2",
        "l100", "l101", "l102", "l103", "l105", "l106",
        "rl_1", "rl_2", "rl_3", "rl_4", "rl_5", "rl_6", "rl_7", "rl_8", "rl_9", "rl_10",
        "rl_11", "rl_12", "rl_13", "rl_14", "rl_15", "rl_16", "rl_17", "rl_18", "rl_19", "rl_20",
        "rl_21", "rl_23", "rl_24", "rl_25", "rl_26", "rl_27", "rl_29", "rl_30", "rl_31", "rl_34",
        "rl_35", "rl_36", "rl_37", "rl_38", "rl_39", "rl_40", "rl_41", "rl_42", "rl_61", "rl_62", "rl_82", "rl_83"
    };

    std::vector<std::string> data = result;
    std::sort(data.begin(), data.end());
    EXPECT_NE(data, result);

    std::sort(data.begin(), data.end(), StringUtils::naturalCompare);
    EXPECT_EQ(data, result);
}
