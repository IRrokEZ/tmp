#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const domain::Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const domain::Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().number] = &buses_.back();

    for (const auto& stop_name : bus.stops) {
        stop_to_buses_[stop_name].insert(bus.number);
    }
    
    if (!bus.is_roundtrip) {
        for (int i = bus.stops.size() - 2; i >= 0; -- i) {
            stop_to_buses_[bus.stops[i]].insert(bus.number);
        }
    }
}

void TransportCatalogue::SetDistance(const std::string& from, const std::string& to, int distance) {
    distances_[from][to] = distance;
}

std::optional<int> TransportCatalogue::GetDistance(const std::string& from, const std::string& to) const {
    if ((distances_.count(from)) && (distances_.at(from).count(to))) {
        return distances_.at(from).at(to);
    }
    return std::nullopt;
}

const domain::Bus* TransportCatalogue::GetBus(const std::string& bus_number) const {
    if (busname_to_bus_.count(bus_number)) {
        return busname_to_bus_.at(bus_number);
    }
    return nullptr;
}

const domain::Stop* TransportCatalogue::GetStop(const std::string& stop_name) const {
    if (stopname_to_stop_.count(stop_name)) {
        return stopname_to_stop_.at(stop_name);
    }
    return nullptr;
}

std::set<std::string> TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    if (stop_to_buses_.count(stop_name)) {
        return stop_to_buses_.at(stop_name);
    }
    return {};
}

RouteInfo TransportCatalogue::GetRouteInfo(const std::string& bus_number) const {
    RouteInfo info;
    const domain::Bus* bus = GetBus(bus_number);
    if (!bus) return info;
    
    info.stops_count = (bus->is_roundtrip) ? (bus->stops.size()) : (bus->stops.size() * 2 - 1);
    
    std::unordered_set<std::string> unique_stops(bus->stops.begin(), bus->stops.end());
    info.unique_stops_count = unique_stops.size();
    
    // Расчет географической длины
    double geo_length = 0.0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++ i) {
        const domain::Stop* from = GetStop(bus->stops[i]);
        const domain::Stop* to = GetStop(bus->stops[i+1]);
        if (from && to) {
            geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }
    }
    if (!bus->is_roundtrip) {
        geo_length *= 2;
    }
    
    // Расчет дорожной длины
    info.route_length = 0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++ i) {
        const std::string& from = bus->stops[i];
        const std::string& to = bus->stops[i+1];
        if (auto dist = GetDistance(from, to)) {
            info.route_length += *dist;
        } else if (auto reverse_dist = GetDistance(to, from)) {
            info.route_length += *reverse_dist;
        }
    }
    if (!bus->is_roundtrip) {
        for (int i = bus->stops.size() - 1; i > 0; --i) {
            const std::string& from = bus->stops[i];
            const std::string& to = bus->stops[i-1];
            if (auto dist = GetDistance(from, to)) {
                info.route_length += *dist;
            } else if (auto reverse_dist = GetDistance(to, from)) {
                info.route_length += *reverse_dist;
            }
        }
    }
    
    info.curvature = info.route_length / geo_length;
    return info;
}

}  // namespace transport_catalogue