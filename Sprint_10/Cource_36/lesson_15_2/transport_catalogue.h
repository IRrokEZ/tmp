#pragma once

#include <deque>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace transport {

class Catalogue {
public:
    struct StopDistancesHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& points) const {
            std::size_t hash_first = std::hash<const void*>{}(points.first);
            std::size_t hash_second = std::hash<const void*>{}(points.second);
            return hash_first + hash_second * 37;
        }
    };

    void AddStop(std::string_view stop_name, const geo::Coordinates coordinates);
    void AddRoute(std::string_view bus_number, const std::vector<const Stop*> stops, bool is_circle);
    const Bus* FindRoute(std::string_view bus_number) const;
    const Stop* FindStop(std::string_view stop_name) const;
    size_t UniqueStopsCount(std::string_view bus_number) const;
    void SetDistance(const Stop* from, const Stop* to, const int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;

private:
    std::deque<Bus> all_buses_;
    std::deque<Stop> all_stops_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistancesHasher> stop_distances_;
};

} // namespace transport