#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
#include "domain.h"

namespace transport {

class Catalogue {
public:
    void AddStop(const Stop& stop);
    void AddRoute(const Bus& bus);
    
    const Bus* FindRoute(std::string_view bus_number) const;
    const Stop* FindStop(std::string_view stop_name) const;
    
    std::size_t UniqueStopsCount(std::string_view bus_number) const;
    void SetDistance(const Stop* from, const Stop* to, int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    
    const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;
    BusStat GetBusStats(std::string_view bus_number) const;
    std::set<std::string> GetBusesForStop(std::string_view stop_name) const;

private:
    struct StopDistancesHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            auto hash1 = std::hash<const void*>{}(stops.first);
            auto hash2 = std::hash<const void*>{}(stops.second);
            return hash1 + hash2 * 37;
        }
    };

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistancesHasher> distances_;
    std::unordered_map<std::string_view, std::set<std::string>> stop_to_buses_;
};

} // namespace transport