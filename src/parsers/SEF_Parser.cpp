#include "SEF_Parser.h"

#include <fstream>
#include <iostream>

#include "StringUtils.h"

SEF_Parser::SEF_Parser(std::string_view sefPath) {
    std::ifstream file(sefPath.data());
    if (!file) {
        fprintf(stderr, "Error: cannot open file: %s\n", sefPath);
        return;
    }

    std::string line;
    PersonParserContext personsCtx;
    ParseSection currentSection = ParseSection::NONE;
    while (std::getline(file, line)) {

        if (line == "persons:") {
            currentSection = ParseSection::PERSONS;
            continue;
        } else if (line == "triggers:") {
            currentSection = ParseSection::TRIGGERS;
            continue;
        } else if (line == "points_entrance:") {
            currentSection = ParseSection::POINTS_ENTRANCE;
            continue;
        } else if (line == "}") {
            currentSection = ParseSection::NONE;
            continue;
        }
        if (line.starts_with("pack:")) {
            m_data.pack = StringUtils::extractQuotedValue(line);
        } else if (currentSection == ParseSection::PERSONS) {
            parsePersonLine(line, m_data, personsCtx);
        }
    }
}

const SEF_Data& SEF_Parser::data() const
{
    return m_data;
}

Direction SEF_Parser::parseDirection(const std::string& dir) {
    if (dir == "LEFT") return Direction::LEFT;
    if (dir == "RIGHT") return Direction::RIGHT;
    if (dir == "UP") return Direction::UP;
    if (dir == "DOWN") return Direction::DOWN;
    if (dir == "UP_LEFT") return Direction::UP_LEFT;
    if (dir == "UP_RIGHT") return Direction::UP_RIGHT;
    if (dir == "DOWN_LEFT") return Direction::DOWN_LEFT;
    if (dir == "DOWN_RIGHT") return Direction::DOWN_RIGHT;
    throw std::runtime_error("Unknown direction: " + dir);
}

RouteType SEF_Parser::parseRouteType(const std::string& type) {
    if (type == "STAY") return RouteType::STAY;
    if (type == "RANDOM_RADIUS") return RouteType::RANDOM_RADIUS;
    if (type == "STAY_ROTATE") return RouteType::STAY_ROTATE;
    if (type == "MOVED_FLIP") return RouteType::MOVED_FLIP;
    if (type == "MOVED") return RouteType::MOVED;
    if (type == "RANDOM") return RouteType::RANDOM;
    throw std::runtime_error("Unknown route type: " + type);
}

void SEF_Parser::parsePersonLine(const std::string& rawLine, SEF_Data& outData, PersonParserContext& ctx) {
    std::string line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" ) return;

    if (line == "}") {
        // Закрытие блока персонажа — сохраняем
        outData.persons.push_back(ctx.currentPerson);
        return;
    }

    // Удаление комментария
    // TODO: Брать имя из sdb
    auto commentPos = line.find("//");
    if (commentPos != std::string::npos) {
        std::string comment = StringUtils::trim(line.substr(commentPos + 2));
        ctx.currentPerson.literaryName = StringUtils::decodeWin1251ToUtf8(comment);
        line = StringUtils::trim(line.substr(0, commentPos));
        if (line.empty()) return;
    }

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string key = StringUtils::trim(line.substr(0, sepPos));
    std::string value = StringUtils::trim(line.substr(sepPos + 1));

    if (key == "name:") {
        ctx.currentPerson = SEF_Person();  // Начинаем нового персонажа
        ctx.currentPerson.techName = StringUtils::extractQuotedValue(value);
    } else if (key == "position") {
        auto tokens = StringUtils::splitBySpaces(value);
        if (tokens.size() == 2) {
            ctx.currentPerson.position.x = std::stoi(tokens[0]);
            ctx.currentPerson.position.y = std::stoi(tokens[1]);
        }
    } else if (key == "literary_name") {
        ctx.currentPerson.literaryNameIndex = std::stoi(value);
    } else if (key == "direction") {
        ctx.currentPerson.direction = StringUtils::extractQuotedValue(value);
    } else if (key == "route_type") {
        ctx.currentPerson.routeType = StringUtils::extractQuotedValue(value);
    } else if (key == "route") {
        ctx.currentPerson.route = StringUtils::extractQuotedValue(value);
    } else if (key == "radius") {
        ctx.currentPerson.radius = std::stoi(value);
    } else if (key == "delay_min") {
        ctx.currentPerson.delayMin = std::stoi(value);
    } else if (key == "delay_max") {
        ctx.currentPerson.delayMax = std::stoi(value);
    } else if (key == "tribe") {
        ctx.currentPerson.tribe = StringUtils::extractQuotedValue(value);
    } else if (key == "scr_dialog") {
        ctx.currentPerson.scriptDialog = StringUtils::extractQuotedValue(value);
    } else if (key == "scr_inv") {
        ctx.currentPerson.scriptInventory = StringUtils::extractQuotedValue(value);
    }
}
