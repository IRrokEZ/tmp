#pragma once

#include <cmath>

constexpr double EARTH_RAD = 6371000;

struct Coordinates {
    double lat;
    double lng;

    bool operator== (const Coordinates& other) const {
        return (lat == other.lat) && (lng == other.lng);
    }
    bool operator!= (const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance (Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }
    static const double deg_to_rad = std::acos(-1.) / 180.;
    return std::acos(
        std::sin(from.lat * deg_to_rad) * std::sin(to.lat * deg_to_rad)
        + std::cos(from.lat * deg_to_rad) * std::cos(to.lat * deg_to_rad)
            * std::cos(std::abs(from.lng - to.lng) * deg_to_rad))
        * EARTH_RAD;
}