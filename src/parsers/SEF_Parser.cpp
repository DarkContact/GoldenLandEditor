#include "SEF_Parser.h"

#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

bool SEF_Parser::parse(std::string_view sefPath, SEF_Data& data, std::string* error) {
    Tracy_ZoneScoped;
    auto fileData = FileUtils::loadFile(sefPath, error);
    if (fileData.empty()) {
        return false;
    }

    std::string_view fileStringView((char*)fileData.data(), fileData.size());
    ParseSection currentSection = ParseSection::NONE;
    StringUtils::forEachLine(fileStringView, [&data, &currentSection] (std::string_view line) {
        line = StringUtils::eraseOneLineComment(line);
        if (line.empty()) return;

        if (line == "persons:") {
            currentSection = ParseSection::PERSONS;
            return;
        } else if (line == "points_entrance:") {
            currentSection = ParseSection::POINTS_ENTRANCE;
            return;
        } else if (line == "cell_groups:") {
            currentSection = ParseSection::CELL_GROUPS;
            return;
        } else if (line == "triggers:") {
            currentSection = ParseSection::TRIGGERS;
            return;
        } else if (line == "doors:") {
            currentSection = ParseSection::DOORS;
            return;
        } else if (line == "}") {
            currentSection = ParseSection::NONE;
            return;
        }

        if (currentSection == ParseSection::NONE) {
            if (line.starts_with("version:")) {
                data.version = getValue(line);
            } else if (line.starts_with("pack:")) {
                data.pack = StringUtils::extractQuotedValue(line);
            } else if (line.starts_with("internal_location:")) {
                data.internalLocation = (getValue(line) == "1");
            } else if (line.starts_with("exit_to_globalmap:")) {
                data.exitToGlobalMap = (getValue(line) == "1");
            } else if (line.starts_with("weather:")) {
                data.weather = StringUtils::toInt(getValue(line));
            }
        } else if (currentSection == ParseSection::PERSONS) {
            parsePersonLine(line, data);
        } else if (currentSection == ParseSection::POINTS_ENTRANCE) {
            parsePointEntranceLine(line, data);
        } else if (currentSection == ParseSection::CELL_GROUPS) {
            parseCellGroupLine(line, data);
        }
    });
    return true;
}

Direction SEF_Parser::parseDirection(std::string_view dir) {
    if (dir == "LEFT") return Direction::LEFT;
    if (dir == "RIGHT") return Direction::RIGHT;
    if (dir == "UP") return Direction::UP;
    if (dir == "DOWN") return Direction::DOWN;
    if (dir == "UP_LEFT") return Direction::UP_LEFT;
    if (dir == "UP_RIGHT") return Direction::UP_RIGHT;
    if (dir == "DOWN_LEFT") return Direction::DOWN_LEFT;
    if (dir == "DOWN_RIGHT") return Direction::DOWN_RIGHT;
    return Direction::LEFT;
}

RouteType SEF_Parser::parseRouteType(std::string_view type) {
    if (type == "STAY") return RouteType::STAY;
    if (type == "RANDOM_RADIUS") return RouteType::RANDOM_RADIUS;
    if (type == "STAY_ROTATE") return RouteType::STAY_ROTATE;
    if (type == "MOVED_FLIP") return RouteType::MOVED_FLIP;
    if (type == "MOVED") return RouteType::MOVED;
    if (type == "RANDOM") return RouteType::RANDOM;
    return RouteType::STAY;
}

std::string_view SEF_Parser::getValue(std::string_view rawLine)
{
    size_t sepPos = rawLine.find_first_of("\t ");
    if (sepPos == std::string::npos) {
        return {};
    }

    return StringUtils::trim(rawLine.substr(sepPos + 1));
}

void SEF_Parser::parsePersonLine(std::string_view rawLine, SEF_Data& data) {
    std::string_view line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string_view key = StringUtils::trimRight(line.substr(0, sepPos));
    std::string_view value = StringUtils::trimLeft(line.substr(sepPos + 1));

    if (key == "name:") {
        SEF_Person newPerson;
        newPerson.techName = StringUtils::extractQuotedValue(value);
        data.persons.push_back(newPerson);
    } else {
        SEF_Person& currentPerson = data.persons.back();
        if (key == "position") {
            StringUtils::parsePosition(value, currentPerson.position.x, currentPerson.position.y);
        } else if (key == "literary_name") {
            currentPerson.literaryNameIndex = StringUtils::toInt(value);
        } else if (key == "direction") {
            currentPerson.direction = StringUtils::extractQuotedValue(value);
        } else if (key == "route_type") {
            currentPerson.routeType = StringUtils::extractQuotedValue(value);
        } else if (key == "route") {
            currentPerson.route = StringUtils::extractQuotedValue(value);
        } else if (key == "radius") {
            currentPerson.radius = StringUtils::toInt(value);
        } else if (key == "delay_min") {
            currentPerson.delayMin = StringUtils::toInt(value);
        } else if (key == "delay_max") {
            currentPerson.delayMax = StringUtils::toInt(value);
        } else if (key == "tribe") {
            currentPerson.tribe = StringUtils::extractQuotedValue(value);
        } else if (key == "scr_dialog") {
            currentPerson.scriptDialog = StringUtils::extractQuotedValue(value);
        } else if (key == "scr_inv") {
            currentPerson.scriptInventory = StringUtils::extractQuotedValue(value);
        }
    }
}

void SEF_Parser::parsePointEntranceLine(std::string_view rawLine, SEF_Data& data)
{
    std::string_view line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string_view key = StringUtils::trimRight(line.substr(0, sepPos));
    std::string_view value = StringUtils::trimLeft(line.substr(sepPos + 1));

    if (key == "name:") {
        SEF_PointEntrance newPoint;
        newPoint.techName = StringUtils::extractQuotedValue(value);
        data.pointsEntrance.push_back(newPoint);
    } else {
        SEF_PointEntrance& currentPoint = data.pointsEntrance.back();
        if (key == "position") {
            StringUtils::parsePosition(value, currentPoint.position.x, currentPoint.position.y);
        } else if (key == "direction") {
            currentPoint.direction = StringUtils::extractQuotedValue(value);
        }
    }
}

void SEF_Parser::parseCellGroupLine(std::string_view rawLine, SEF_Data& data)
{
    std::string_view line = StringUtils::trim(rawLine);
    if (line.empty() || line == "{" || line == "}") return;

    size_t sepPos = line.find_first_of("\t ");
    if (sepPos == std::string::npos) return;

    std::string_view key = StringUtils::trimRight(line.substr(0, sepPos));
    std::string_view value = StringUtils::trimLeft(line.substr(sepPos + 1));

    if (key == "name:") {
        CellGroup newGroup;
        newGroup.name = StringUtils::extractQuotedValue(value);
        data.cellGroups.push_back(newGroup);
    } else {
        auto& currentGroup = data.cellGroups.back();
        if (key.starts_with("cell")) {
            TilePosition position;
            StringUtils::parsePosition(value, position.x, position.y);
            currentGroup.cells.push_back(position);
        }
    }
}
