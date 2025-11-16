#include "transport_catalogue.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using std::literals::string_literals::operator""s;

namespace transport {

void Catalogue::AddStop(std::string && stop_name, const geo::Coordinates coordinates) {
    if (std::isnan(coordinates.lat) || std::isnan(coordinates.lng)) {
        throw std::invalid_argument("Invalid coordinates for stop: "s + stop_name);
    }

    stops_.push_back( {std::move(stop_name), coordinates} );
    stop_name_to_stops_[stops_.back().name] = &stops_.back();
}

void Catalogue::AddRoute(std::string&& route_id, std::vector<const Stop*>&& stops, bool is_circle) {
    routes_.push_back({ std::move(route_id), std::move(stops), is_circle });
    route_id_to_route_[routes_.back().route_id] = &routes_.back();
    for (const transport::Stop *const& route_stop : routes_.back().stops) {
        buses_at_stop_[route_stop->name].insert(routes_.back().route_id);
    }
}

bool Catalogue::IsRouteExists (std::string_view route_id) const {
    return route_id_to_route_.find(route_id) != route_id_to_route_.end();
}

bool Catalogue::IsStopExists (std::string_view stop_name) const {
    return stop_name_to_stops_.find(stop_name) != stop_name_to_stops_.end();
}

const Route* Catalogue::GetRoute(std::string_view route_id) const {
    auto it = route_id_to_route_.find(route_id);
    return (it != route_id_to_route_.end()) ? it->second : nullptr;
}

const Stop* Catalogue::GetStop(std::string_view stop_name) const {
    auto it = stop_name_to_stops_.find(stop_name);
    return (it != stop_name_to_stops_.end()) ? it->second : nullptr;
}

std::optional<double> Catalogue::GetDistance (std::string_view from_stop, std::string_view to_stop) const {
    if ((!IsStopExists(from_stop)) || (!IsStopExists(to_stop))) {
        return std::nullopt;
    }
    
    auto first_col_it = distances_.find(from_stop);
    if (first_col_it != distances_.end()) {
        auto sec_col_it = first_col_it->second.find(to_stop);
        if(sec_col_it != first_col_it->second.end()) {
            return sec_col_it->second;
        }
    } 
    
    first_col_it = distances_.find(to_stop);
    if (first_col_it != distances_.end()) {
        auto sec_col_it = first_col_it->second.find(from_stop);
        if (sec_col_it != first_col_it->second.end()) {
            return sec_col_it->second;
        }
    }

    return std::nullopt;
}

void Catalogue::SetDistance(const Stop* from, const Stop* to, double distance) {
    distances_[from->name][to->name] = distance;
}

const std::unordered_set<std::string_view>* Catalogue::GetAllBusesByStop (std::string_view stop_name) const {
    auto it = buses_at_stop_.find(stop_name);
    return (it != buses_at_stop_.end()) ? &it->second : nullptr;
}

std::optional<ExtendedRouteInfo> Catalogue::GetExtendedRouteInfo (std::string_view route_id) {
    auto info_it = route_info_.find(route_id);
    if (info_it != route_info_.end()) {
        return info_it->second;
    }

    auto route_it = route_id_to_route_.find(route_id);

    if (route_it == route_id_to_route_.end()) {
        throw std::domain_error("Bus with id "s + std::string(route_id) + " not found. Data is corrupted."s);
    }

    std::unordered_set<std::string_view> unique_stops(route_it->second->stops.size());
    for (const transport::Stop* const stop : route_it->second->stops) {
        unique_stops.insert(stop->name);
    }


    std::size_t stops_count;
    if (!route_it->second->is_circular) {
        stops_count = route_it->second->stops.size() * 2 - 1;
    } else {
        stops_count = route_it->second->stops.size();
    }

    double distance = 0.;
    double straight_distance = 0.;

    for (std::size_t i = 1; i < route_it->second->stops.size(); ++ i) {
        const transport::Stop* const from = route_it->second->stops[i - 1];
        const transport::Stop* const to = route_it->second->stops[i];
        std::optional<double> dist = GetDistance(from->name, to->name);
        distance += (dist.has_value()) ? dist.value() : 0.;
        straight_distance += geo::ComputeDistance(from->coordinates, to->coordinates);

        if (!route_it->second->is_circular) {
            dist = GetDistance(to->name, from->name);
            distance += (dist.has_value()) ? dist.value() : 0.;
            straight_distance += geo::ComputeDistance(to->coordinates, from->coordinates);
        }
    }

    route_info_[route_it->second->route_id] = {
        stops_count, /*total stops*/
        unique_stops.size(), /*total unique stops*/
        distance,
        (straight_distance > 0) ? (distance / straight_distance) : 0};

    info_it = route_info_.find(route_it->second->route_id);
    if (info_it != route_info_.end()) {
        return info_it->second;
    }

    return std::nullopt;
}

std::map<std::string_view, const Stop*> Catalogue::GetSortedAllStops () const {
    return {stop_name_to_stops_.begin(), stop_name_to_stops_.end()};
}

std::map<std::string_view, const Route*> Catalogue::GetSortedAllRoutes () const {
    return {route_id_to_route_.begin(), route_id_to_route_.end()};
}

} // namespace transport