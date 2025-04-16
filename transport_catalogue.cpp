#include <algorithm>
#include <stdexcept>
#include <utility>

#include "transport_catalogue.h"

#include <iostream>

using std::literals::string_literals::operator""s;

void TransportCatalogue::AddStop (const std::string &stop_name, Coordinates coordinates) {
    if (std::isnan(coordinates.lat) || std::isnan(coordinates.lng)) {
        throw std::invalid_argument("Invalid coordinates for stop: "s + stop_name);
    }

    stops_.push_back({stop_name, coordinates});
    stop_name_to_stops_[stops_.back().name] = &(stops_.back());
}

void TransportCatalogue::AddBus (const std::string &route_id, const std::vector<std::string_view> &parsed_route, bool is_circular) {
    routes_.push_back({route_id, {}/*empty route*/, is_circular});
    for (const std::string_view &stop : parsed_route) {
        auto it = stop_name_to_stops_.find(stop);
        if (it != stop_name_to_stops_.end()) {
            routes_.back().stops.push_back(it->second);
        } else {
            throw std::logic_error("Unknown stop in route: ["s + std::string(stop) + "]"s);
        }
    }
    route_id_to_route_[routes_.back().route_id] = &(routes_.back());
}

void TransportCatalogue::SetDistance (const std::string &from_stop, const std::string &to_stop, double distance) {
    distances_[from_stop][to_stop] = distance;
}

bool TransportCatalogue::IsRouteExists (std::string_view route_id) const {
    return route_id_to_route_.find(route_id) != route_id_to_route_.end();
}

bool TransportCatalogue::IsStopExists (std::string_view stop_name) const {
    return stop_name_to_stops_.find(stop_name) != stop_name_to_stops_.end();
}

std::optional<const Stop*> TransportCatalogue::GetStop (std::string_view stop_name) const {
    auto it = stop_name_to_stops_.find(stop_name);
    if (it != stop_name_to_stops_.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::optional<const Route*> TransportCatalogue::GetRoute (std::string_view route_id) const {
    auto it = route_id_to_route_.find(route_id);
    if (it != route_id_to_route_.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::optional<double> TransportCatalogue::GetDistance (std::string_view from_stop, std::string_view to_stop) const {
    auto from_it = distances_.find(std::string(from_stop));
    if (from_it != distances_.end()) {
        auto to_it = from_it->second.find(std::string(to_stop));
        if (to_it != from_it->second.end()) {
            return to_it->second;
        }
    }
    return std::nullopt;
}

ExtendedRouteInfo TransportCatalogue::GetExtendedRouteInfo (std::string_view route_id) {
    auto it = route_id_to_route_.find(route_id);

    if (it == route_id_to_route_.end()) {
        throw std::domain_error("Bus with id "s + std::string(route_id) + " not found. Data is corrupted.");
    }

    auto route_info = route_info_.find(it->second->route_id);
    if (route_info != route_info_.end()) {
        return route_info->second;
    }

    std::unordered_set<std::string_view> unique_stops;
    for (const auto &el : it->second->stops) {
        unique_stops.insert(el->name);
    }

    double distance = 0.;
    auto prev = it->second->stops.begin();
    auto sec = std::next(it->second->stops.begin());
    
    if (!it->second->is_circular) {
        std::optional<double> extra_dist_begin = GetDistance((*prev)->name, (*prev)->name);

        std::size_t center_id = it->second->stops.size() / 2;
        std::optional<double> extra_dist_center = GetDistance(it->second->stops[center_id]->name, it->second->stops[center_id]->name);

        if (extra_dist_begin.has_value()) {
            distance += extra_dist_begin.value();
        }

        if (extra_dist_center.has_value()) {
            distance += extra_dist_center.value();
        }
    }

    while (sec != it->second->stops.end()) {
        std::optional<double> dist = GetDistance((*prev)->name, (*sec)->name);
        if (!dist.has_value()) {
            dist = GetDistance((*sec)->name, (*prev)->name);
        }
        if (dist.has_value()) {
            distance += dist.value();
        }
        ++ prev;
        ++ sec;
    }

    double straight_distance = 0.;
    prev = it->second->stops.begin();
    sec = std::next(it->second->stops.begin());
    while (sec != it->second->stops.end()) {
        straight_distance += ComputeDistance((*prev)->coordinates, (*sec)->coordinates);
        ++ prev;
        ++ sec;
    }

    auto t = it->second->route_id;
    route_info_[it->second->route_id] = {
        it->second->stops.size(), /*total stops*/
        unique_stops.size(), /*total unique stops*/
        distance,
        (straight_distance > 0) ? (distance / straight_distance) : 0};

    route_info = route_info_.find(it->second->route_id);
    if (route_info != route_info_.end()) {
        return route_info->second;
    }

    return {};
}

const std::unordered_set<std::string_view>& TransportCatalogue::GetAllBusesByStop (std::string_view stop_name) {
    auto it = buses_at_stop_.find(stop_name);
    if (it != buses_at_stop_.end()) {
        return it->second;
    }

    auto curr_stop = stop_name_to_stops_.find(stop_name);

    if (curr_stop == stop_name_to_stops_.end()) {
        throw std::domain_error("Stop with Name "s + std::string(stop_name) + " not found. Data is corrupted.");
    }

    std::string stop_name_str(stop_name);
    for (const auto &route: routes_) {
        auto stop_iter = std::find_if(route.stops.begin(), route.stops.end(),
                                        [&stop_name_str](const Stop* stop)
                                            {return stop->name == stop_name_str;}
                                    );
        if (stop_iter != route.stops.end()) {
            buses_at_stop_[stop_name].insert(route.route_id);
        }
    }

    auto result = buses_at_stop_.find(stop_name);
    if (result == buses_at_stop_.end()) {
        static std::unordered_set<std::string_view> empty_set;
        buses_at_stop_[stop_name] = empty_set;
        return buses_at_stop_.at(stop_name);
    }

    return result->second;
}