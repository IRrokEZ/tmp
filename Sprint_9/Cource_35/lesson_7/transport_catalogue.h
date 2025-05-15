#pragma once

#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

struct Stop {
    std::string name;
    Coordinates coordinates;

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

class TransportCatalogue {
private:
	std::deque<Stop> stops_;
	std::deque<Route> routes_;
	std::unordered_map<std::string_view, const Stop*> stop_name_to_stops_;
	std::unordered_map<std::string_view, const Route*> route_id_to_route_;

	std::unordered_map<std::string, std::unordered_map<std::string, double>> distances_;

    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_at_stop_;
	std::unordered_map<std::string_view, ExtendedRouteInfo> route_info_;
public:
	void AddStop (const std::string &stop_name, Coordinates Coordinates);
	void AddBus (const std::string &route_id, const std::vector<std::string_view> &route, bool is_circular);
	void SetDistance (const std::string &from_stop, const std::string &to_stop, double distance);

	bool IsRouteExists (std::string_view route_id) const;
	bool IsStopExists (std::string_view stop_name) const;

    std::optional<const Stop*> GetStop (std::string_view stop_name) const;
    std::optional<const Route*> GetRoute (std::string_view route_id) const;
	std::optional<double> GetDistance (std::string_view from_stop, std::string_view to_stop) const;

	ExtendedRouteInfo GetExtendedRouteInfo (std::string_view route_id);
	const std::unordered_set<std::string_view>& GetAllBusesByStop (std::string_view stop_name);
};