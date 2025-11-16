#pragma once

#include <optional>
#include <set>
#include <string>
#include <string_view>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

class RequestHandler {
public:
    explicit RequestHandler (transport::Catalogue& catalogue, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue), renderer_(renderer) {}

    void ProcessRequests (const json::Array& stat_requests) const;
    void ProcessRequests (const json::Array& stat_requests, std::ostream& out) const;
    
    json::Node PrintRoute (const json::Dict& request_map) const;
    json::Node PrintStop (const json::Dict& request_map) const;
    json::Node PrintMap (const json::Dict& request_map) const;
    
    std::optional<transport::ExtendedRouteInfo> GetExtendedRouteInfo (const std::string_view route_id) const;
    const std::unordered_set<std::string_view>* GetBusesByStop (std::string_view stop_name) const;

	bool IsRouteExists (std::string_view route_id) const;
	bool IsStopExists (std::string_view stop_name) const;
    
    svg::Document RenderMap () const;

private:
    transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
};