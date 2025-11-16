#include "geo.h"

#include <cmath>

#include "domain.h"

namespace geo {

double ComputeDistance (Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }
    
    return std::acos(std::sin(from.lat * dr) * std::sin(to.lat * dr)
                + std::cos(from.lat * dr) * std::cos(to.lat * dr) 
                * std::cos(std::abs(from.lng - to.lng) * dr))
                * earth_rd;
}

} // namespace geo