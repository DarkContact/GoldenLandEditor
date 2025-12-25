#pragma once
#include <string_view>

class FontSettings {
public:
    FontSettings();

    void update(bool& showWindow);

private:
    std::string_view m_title;
    int m_fontSize;
};
