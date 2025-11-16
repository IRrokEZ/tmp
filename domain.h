#define _USE_MATH_DEFINES

#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "geo.h"

namespace transport {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;

    bool operator== (const Stop& other) {
        return name == other.name && coordinates == other.coordinates;
    }

    bool operator!= (const Stop& other) {
        return !(*this == other);
    }
};

struct Route {
    std::string route_id;
    std::vector<const Stop*> stops;
    bool is_circular;
};

struct ExtendedRouteInfo {
    std::size_t all_stops_counter;
    std::size_t unique_stops_counter;
    double route_length;
    double curvature;
};

} // namespace transport

namespace renderer {
    inline const double EPSILON = 1e-6;
} // namespace renderer

namespace geo {
    static const double dr = M_PI / 180.;
    static const int earth_rd = 6371000;
} // namespace geo