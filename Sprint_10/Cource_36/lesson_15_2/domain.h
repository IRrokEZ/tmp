#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string> buses_by_stop;
};

struct Bus {
    std::string number;
    std::vector<const Stop*> stops;
    bool is_circle;
};

struct BusStat {
    std::size_t stops_count;
    std::size_t unique_stops_count;
    double route_length;
    double curvature;
};

} // namespace transport