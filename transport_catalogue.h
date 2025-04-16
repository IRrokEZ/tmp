#pragma once

#include <cmath>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"
#include "geo.h"

namespace transport_catalogue {

struct RouteInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double route_length;
    double curvature;
};

class TransportCatalogue {
public:
    void AddStop(const domain::Stop& stop);
    void AddBus(const domain::Bus& bus);
    void SetDistance(const std::string& from, const std::string& to, int distance);
    
    std::optional<int> GetDistance(const std::string& from, const std::string& to) const;
    const domain::Bus* GetBus(const std::string& bus_number) const;
    const domain::Stop* GetStop(const std::string& stop_name) const;
    std::set<std::string> GetBusesForStop(const std::string& stop_name) const;
    RouteInfo GetRouteInfo(const std::string& bus_number) const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;
    std::unordered_map<std::string, const domain::Stop*> stopname_to_stop_;
    std::unordered_map<std::string, const domain::Bus*> busname_to_bus_;
    std::unordered_map<std::string, std::set<std::string>> stop_to_buses_;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> distances_;
};

} // namespace transport_catalogue