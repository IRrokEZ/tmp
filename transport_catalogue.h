#pragma once

#include <deque>
#include <functional>
#include <optional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace transport {

class Catalogue {
public:

	void AddStop(std::string && stop_name, const geo::Coordinates coordinates);
	void AddRoute(std::string&& route_id, std::vector<const Stop*>&& stops, bool is_circle);
    void SetDistance(const Stop* from, const Stop* to, double distance);

	bool IsRouteExists (std::string_view route_id) const;
	bool IsStopExists (std::string_view stop_name) const;

    const Stop* GetStop(std::string_view stop_name) const;
	const Route* GetRoute(std::string_view route_id) const;
	std::optional<double> GetDistance (std::string_view from_stop, std::string_view to_stop) const;

	const std::unordered_set<std::string_view>* GetAllBusesByStop (std::string_view stop_name) const;
	std::optional<ExtendedRouteInfo> GetExtendedRouteInfo (std::string_view route_id);

	std::map<std::string_view, const Stop*> GetSortedAllStops () const;
	std::map<std::string_view, const Route*> GetSortedAllRoutes () const;

private:
    std::deque<Stop> stops_;
    std::deque<Route> routes_;

	std::unordered_map<std::string_view, const Stop*> stop_name_to_stops_;
	std::unordered_map<std::string_view, const Route*> route_id_to_route_;

    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_at_stop_;
	std::unordered_map<std::string_view, ExtendedRouteInfo> route_info_;
	std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> distances_;	
};

} // namespace transport