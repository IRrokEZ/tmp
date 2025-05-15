#include "transport_catalogue.h"

namespace transport {

void Catalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void Catalogue::AddRoute(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().number] = &buses_.back();
    
    for (const auto stop : buses_.back().stops) {
        stop_to_buses_[stop->name].insert(buses_.back().number);
    }
}

const Bus* Catalogue::FindRoute(std::string_view bus_name) const {
    return busname_to_bus_.count(bus_name) ? busname_to_bus_.at(bus_name) : nullptr;
}

const Stop* Catalogue::FindStop(std::string_view stop_name) const {
    return stopname_to_stop_.count(stop_name) ? stopname_to_stop_.at(stop_name) : nullptr;
}

std::size_t Catalogue::UniqueStopsCount(std::string_view bus_name) const {
    std::unordered_set<std::string_view> unique_stops;
    if (auto bus = FindRoute(bus_name)) {
        for (const auto stop : bus->stops) {
            unique_stops.insert(stop->name);
        }
    }
    return unique_stops.size();
}

void Catalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
    distances_[{from, to}] = distance;
}

int Catalogue::GetDistance(const Stop* from, const Stop* to) const {
    if (distances_.count({from, to})) {
        return distances_.at({from, to});
    }
    if (distances_.count({to, from})) {
        return distances_.at({to, from});
    }
    return 0;
}

BusStat Catalogue::GetBusStats(std::string_view bus_name) const {
    BusStat stats;
    if (auto bus = FindRoute(bus_name)) {
        stats.stops_count = bus->is_circle ? bus->stops.size() : bus->stops.size() * 2 - 1;
        stats.unique_stops_count = UniqueStopsCount(bus_name);
        
        double geo_length = 0.0;
        stats.route_length = 0;
        
        for (size_t i = 1; i < bus->stops.size(); ++ i) {
            const auto from = bus->stops[i-1];
            const auto to = bus->stops[i];
            
            geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
            stats.route_length += GetDistance(from, to);
            
            if (!bus->is_circle) {
                stats.route_length += GetDistance(to, from);
                geo_length += geo::ComputeDistance(to->coordinates, from->coordinates);
            }
        }
        
        stats.curvature = stats.route_length / geo_length;
    }
    return stats;
}

std::set<std::string> Catalogue::GetBusesForStop(std::string_view stop_name) const {
    return stop_to_buses_.count(stop_name) ? stop_to_buses_.at(stop_name) : std::set<std::string>{};
}

const std::map<std::string_view, const Bus*> Catalogue::GetSortedAllBuses() const {
    std::map<std::string_view, const Bus*> sorted_buses;
    for (const auto& [name, bus] : busname_to_bus_) {
        sorted_buses[name] = bus;
    }
    return sorted_buses;
}

} // namespace transport