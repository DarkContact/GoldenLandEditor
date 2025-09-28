#include "SEF_Parser.h"

#include <fstream>

#include "StringUtils.h"

SEF_Parser::SEF_Parser(std::string_view sefPath) {
    std::ifstream file(sefPath.data());
    if (!file) {
        fprintf(stderr, "Error: cannot open file: %s\n", sefPath);
        return;
    }

    std::string line;
    ParseSection currentSection = ParseSection::NONE;
    while (std::getline(file, line)) {

        if (line == "persons:") {
            currentSection = ParseSection::PERSONS;
            continue;
        } else if (line == "points_entrance:") {
            currentSection = ParseSection::POINTS_ENTRANCE;
            continue;
        } else if (line == "cell_groups:") {
            currentSection = ParseSection::CELL_GROUPS;
            continue;
        } else if (line == "triggers:") {
            currentSection = ParseSection::TRIGGERS;
            continue;
        } else if (line == "doors:") {
            currentSection = ParseSection::DOORS;
            continue;
        } else if (line == "}") {
            currentSection = ParseSection::NONE;
            continue;
        }

        if (currentSection == ParseSection::NONE) {
            if (line.starts_with("version:")) {
                m_data.version = getValue(line);
            } else if (line.starts_with("pack:")) {
                m_data.pack = StringUtils::extractQuotedValue(line);
            } else if (line.starts_with("internal_location:")) {
                m_data.internalLocation = (getValue(line) == "1");
            } else if (line.starts_with("exit_to_globalmap:")) {
                m_data.exitToGlobalMap = (getValue(line) == "1");
            } else if (line.starts_with("weather:")) {
                m_data.weather = std::stoi(getValue(line));
            }
        } else if (currentSection == ParseSection::PERSONS) {
            parsePersonLine(line);
        } else if (currentSection == ParseSection::POINTS_ENTRANCE) {
            parsePointEntranceLine(line);
        } else if (currentSection == ParseSection::CELL_GROUPS) {
            parseCellGroupLine(line);
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

std::string SEF_Parser::getValue(const std::string& rawLine)
{
    size_t sepPos = rawLine.find_first_of("\t ");
    if (sepPos == std::string::npos) {
        return {};
    }

    return StringUtils::trim(rawLine.substr(sepPos + 1));
}

void SEF_Parser::parsePersonLine(const std::string& rawLine) {
    std::string line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string key = StringUtils::trim(line.substr(0, sepPos));
    std::string value = StringUtils::trim(line.substr(sepPos + 1));

    if (key == "name:") {
        SEF_Person newPerson;
        newPerson.techName = StringUtils::extractQuotedValue(value);
        m_data.persons.push_back(newPerson);
    } else {
        SEF_Person& currentPerson = m_data.persons.back();

        // Удаление комментария
        auto commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            std::string comment = StringUtils::trim(line.substr(commentPos + 2));
            // TODO: Брать имя из sdb
            currentPerson.literaryName = StringUtils::decodeWin1251ToUtf8(comment);
            line = StringUtils::trim(line.substr(0, commentPos));
            if (line.empty()) return;
        }

        if (key == "position") {
            auto tokens = StringUtils::splitBySpaces(value);
            if (tokens.size() == 2) {
                currentPerson.position.x = std::stoi(tokens[0]);
                currentPerson.position.y = std::stoi(tokens[1]);
            }
        } else if (key == "literary_name") {
            currentPerson.literaryNameIndex = std::stoi(value);
        } else if (key == "direction") {
            currentPerson.direction = StringUtils::extractQuotedValue(value);
        } else if (key == "route_type") {
            currentPerson.routeType = StringUtils::extractQuotedValue(value);
        } else if (key == "route") {
            currentPerson.route = StringUtils::extractQuotedValue(value);
        } else if (key == "radius") {
            currentPerson.radius = std::stoi(value);
        } else if (key == "delay_min") {
            currentPerson.delayMin = std::stoi(value);
        } else if (key == "delay_max") {
            currentPerson.delayMax = std::stoi(value);
        } else if (key == "tribe") {
            currentPerson.tribe = StringUtils::extractQuotedValue(value);
        } else if (key == "scr_dialog") {
            currentPerson.scriptDialog = StringUtils::extractQuotedValue(value);
        } else if (key == "scr_inv") {
            currentPerson.scriptInventory = StringUtils::extractQuotedValue(value);
        }
    }
}

void SEF_Parser::parsePointEntranceLine(const std::string& rawLine)
{
    std::string line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string key = StringUtils::trim(line.substr(0, sepPos));
    std::string value = StringUtils::trim(line.substr(sepPos + 1));

    if (key == "name:") {
        SEF_PointEntrance newPoint;
        newPoint.techName = StringUtils::extractQuotedValue(value);
        m_data.pointsEntrance.push_back(newPoint);
    } else {
        SEF_PointEntrance& currentPoint = m_data.pointsEntrance.back();
        if (key == "position") {
            auto tokens = StringUtils::splitBySpaces(value);
            if (tokens.size() == 2) {
                currentPoint.position.x = std::stoi(tokens[0]);
                currentPoint.position.y = std::stoi(tokens[1]);
            }
        } else if (key == "direction") {
            currentPoint.direction = StringUtils::extractQuotedValue(value);
        }
    }
}

void SEF_Parser::parseCellGroupLine(const std::string& rawLine)
{
    std::string line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string key = StringUtils::trim(line.substr(0, sepPos));
    std::string value = StringUtils::trim(line.substr(sepPos + 1));

    if (key == "name:") {
        SEF_CellGroup newGroup;
        newGroup.techName = StringUtils::extractQuotedValue(value);
        m_data.cellGroups.push_back(newGroup);
    } else {
        SEF_CellGroup& currentGroup = m_data.cellGroups.back();
        if (key.starts_with("cell")) {
            auto tokens = StringUtils::splitBySpaces(value);
            if (tokens.size() == 2) {
                TilePosition position;
                position.x = std::stoi(tokens[0]);
                position.y = std::stoi(tokens[1]);
                currentGroup.cells.push_back(position);
            }
        }
    }
}
