#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct TilePosition {
    uint16_t x;
    uint16_t y;
};

struct CellGroup {
    std::string name;
    std::vector<TilePosition> cells;
};
