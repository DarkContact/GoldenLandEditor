#pragma once
#include <optional>
#include <string>
#include <vector>

#include "Types.h"

enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
};

enum class RouteType {
    STAY,
    RANDOM_RADIUS,
    STAY_ROTATE,
    MOVED_FLIP,
    MOVED,
    RANDOM
};

enum class ParseSection {
    NONE,
    PERSONS,
    POINTS_ENTRANCE,
    CELL_GROUPS,
    TRIGGERS,
    DOORS
};

struct SEF_Person {
    TilePosition position;
    std::string techName;
    int literaryNameIndex = -1;
    std::string direction; // Direction
    std::string routeType; // RouteType
    std::string route;
    int radius = -1;
    int delayMin = -1;
    int delayMax = -1;
    std::string tribe;
    std::string scriptDialog;
    std::string scriptInventory;
};

struct SEF_PointEntrance {
    TilePosition position;
    std::string techName;
    std::string direction; // Direction
};

struct SEF_Trigger {
    std::string techName;
    int literaryNameIndex = -1;
    std::string cursorName;
    std::string scriptName;
    std::optional<std::string> invName;
    std::optional<std::string> cellsName;
    bool isActive = false;
    std::optional<bool> isTransition;
    std::optional<bool> isVisible;
};

struct SEF_Door {
    std::string techName;
    int literaryNameOpenIndex = -1;
    int literaryNameCloseIndex = -1;
    std::string cellsName;
    std::optional<std::string> scriptName;
    std::optional<bool> isOpened;
};

struct SEF_Data {
    std::string version;
    std::string pack;
    bool internalLocation = false;
    bool exitToGlobalMap = false;
    std::optional<int> weather;
    std::vector<SEF_Person> persons;
    std::vector<SEF_PointEntrance> pointsEntrance;
    std::vector<CellGroup> cellGroups;
    std::vector<SEF_Trigger> triggers;
    std::vector<SEF_Door> doors;
};

class SEF_Parser {
public:
    SEF_Parser() = delete;

    static bool parse(std::string_view sefPath, SEF_Data& data, std::string* error);

private:
    static Direction parseDirection(std::string_view dir);
    static RouteType parseRouteType(std::string_view type);

    static std::string_view getValue(std::string_view rawLine);
    static std::optional<std::pair<std::string_view, std::string_view>> getKeyValue(std::string_view rawLine);

    static void parsePersonLine(std::string_view rawLine, SEF_Data& data);
    static void parsePointEntranceLine(std::string_view rawLine, SEF_Data& data);
    static void parseCellGroupLine(std::string_view rawLine, SEF_Data& data);
    static void parseTriggerLine(std::string_view rawLine, SEF_Data& data);
    static void parseDoorLine(std::string_view rawLine, SEF_Data& data);
};
