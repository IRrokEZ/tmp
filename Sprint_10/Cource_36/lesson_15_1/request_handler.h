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
    explicit RequestHandler(const transport::Catalogue& catalogue, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue), renderer_(renderer) {}

    void ProcessRequests(const json::Node& stat_requests) const;
    
    const json::Node PrintRoute(const json::Dict& request_map) const;
    const json::Node PrintStop(const json::Dict& request_map) const;
    
    std::optional<transport::BusStat> GetBusStat(const std::string_view bus_number) const;
    const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;
    
    svg::Document RenderMap() const;

private:
    const transport::Catalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
};