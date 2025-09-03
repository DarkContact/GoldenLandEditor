#include "SDB_Parser.h"

#include <cstring>
#include <stdexcept>

#include "FileLoader.h"
#include "StringUtils.h"

SDB_Parser::SDB_Parser(std::string_view sdbPath) :
    m_filePath(sdbPath)
{

}

SDB_Data& SDB_Parser::parse()
{
    auto buffer = FileLoader::loadFile(m_filePath); // Загружаем весь файл в буфер
    if (buffer.size() < 4) {
        throw std::runtime_error("File too small.");
    }

    size_t offset = 0;
    bool xorRequired = true;

    // Проверка заголовка "SDB "
    if (std::memcmp(buffer.data(), "SDB ", 4) == 0) {
        xorRequired = false;
        offset = 4; // Пропускаем заголовок
    }

    while (offset + 8 <= buffer.size()) {
        // Чтение ID
        int32_t id;
        std::memcpy(&id, &buffer[offset], sizeof(int32_t));
        offset += 4;

        // Чтение длины текста
        int32_t textLength;
        std::memcpy(&textLength, &buffer[offset], sizeof(int32_t));
        offset += 4;

        if (textLength < 0 || offset + textLength > buffer.size()) {
            throw std::runtime_error("Invalid text length or corrupted file.");
        }

        // Чтение текста
        std::vector<char> textData(buffer.begin() + offset, buffer.begin() + offset + textLength);
        offset += textLength;

        // Применение XOR-дешифровки, если нужно
        if (xorRequired) {
            for (auto& byte : textData) {
                byte ^= 0xAA;
            }
        }

        // Добавление строки в карту (обрезаем пробелы справа)
        std::string text(textData.begin(), textData.end());
        text = StringUtils::decodeWin1251ToUtf8(text);
        text = StringUtils::trim(text);
        m_data.strings[id] = text;
    }

    return m_data;
}
