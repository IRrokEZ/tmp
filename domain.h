#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::unordered_map<std::string, int> road_distances;
};

struct Bus {
    std::string number;
    std::vector<std::string> stops;
    bool is_roundtrip;
};

}  // namespace domain