#pragma once
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
    POINTS,
    CELL_GROUPS,
    TRIGGERS,
    DOORS
};

struct SEF_Person {
    TilePosition position;
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

struct PersonParserContext {
    ParseSection currentSection = ParseSection::NONE;
    bool insidePersonBlock = false;
    int sectionBraceDepth = 0;
    SEF_Person currentPerson;
};

struct SEF_Data {
    std::string pack;
    std::vector<SEF_Person> persons;
};

class SEF_Parser
{
public:
    SEF_Parser(std::string_view sefPath);

    const SEF_Data& data() const;

private:
    Direction parseDirection(const std::string& dir);
    RouteType parseRouteType(const std::string& type);

    void parsePersonLine(const std::string& rawLine, SEF_Data& outData, PersonParserContext& ctx);

    SEF_Data m_data;
};
