#pragma once
#include <optional>
#include <string>
#include <vector>

struct TilePosition {
    int x = -1;
    int y = -1;
};

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
    std::string literaryName;
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

struct SEF_Data {
    std::string version;
    std::string pack;
    bool internalLocation = false;
    bool exitToGlobalMap = false;
    std::optional<int> weather;
    std::vector<SEF_Person> persons;
    std::vector<SEF_PointEntrance> pointsEntrance;
};

class SEF_Parser
{
public:
    SEF_Parser(std::string_view sefPath);

    const SEF_Data& data() const;

private:
    Direction parseDirection(const std::string& dir);
    RouteType parseRouteType(const std::string& type);

    std::string getValue(const std::string& rawLine);

    void parsePersonLine(const std::string& rawLine, SEF_Person& currentPerson);
    void parsePointEntranceLine(const std::string& rawLine, SEF_PointEntrance& currentPoint);

    SEF_Data m_data;
};
