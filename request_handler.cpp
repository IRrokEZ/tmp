#include "request_handler.h"

#include <algorithm>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "json_builder.h"
#include "json_reader.h"
#include "svg.h"

using std::literals::string_literals::operator""s;

void RequestHandler::ProcessRequests (const json::Array& stat_requests) const {
    json::Array result;
    for (const json::Node& request : stat_requests) {
        const json::Dict& request_map = request.AsMap();
        const std::string& type = request_map.at("type"s).AsString();
        if (type == "Stop"s) {
            result.emplace_back(PrintStop(request_map).AsMap());
        }
        if (type == "Bus"s) {
            result.emplace_back(PrintRoute(request_map).AsMap());
        }
        if (type == "Map"s) {
            result.emplace_back(PrintMap(request_map).AsMap());
        }
    }
    
    json::Print(json::Document{ json::Node(result) }, std::cout);
}

void RequestHandler::ProcessRequests (const json::Array& stat_requests, std::ostream& out) const {
    json::Array result;
    for (const json::Node& request : stat_requests) {
        const json::Dict& request_map = request.AsMap();
        const std::string& type = request_map.at("type"s).AsString();
        if (type == "Stop"s) {
            result.push_back(json::Node(PrintStop(request_map).AsMap()));
        }
        if (type == "Bus"s) {
            result.push_back(json::Node(PrintRoute(request_map).AsMap()));
        }
        if (type == "Map"s) {
            result.push_back(json::Node(PrintMap(request_map).AsMap()));
        }
    }
    
    json::Print(json::Document{ json::Node(result) }, out);
}

json::Node RequestHandler::PrintRoute (const json::Dict& request_map) const {
    json::Builder builder;

    const std::string& route_id = request_map.at("name"s).AsString();
    builder.StartDict()
           .Key("request_id"s).Value(request_map.at("id"s).AsInt());

    const transport::Route* route = catalogue_.GetRoute(route_id);
    if (!route) {
        builder.Key("error_message"s).Value("not found"s);
    } else {
        std::optional<transport::ExtendedRouteInfo> info = GetExtendedRouteInfo(route_id);
        if (!info.has_value()) {
            builder.Reset();
            throw std::domain_error("Undefined Behavoir. Data corrupted"s);
        }
        builder.Key("curvature"s).Value(info.value().curvature)
               .Key("route_length"s).Value(info.value().route_length)
               .Key("stop_count"s).Value(static_cast<int>(info.value().all_stops_counter))
               .Key("unique_stop_count"s).Value(static_cast<int>(info.value().unique_stops_counter));
    }
    
    return builder.EndDict().Build();
}

json::Node RequestHandler::PrintStop (const json::Dict& request_map) const {
    json::Builder builder;
    const std::string stop_name = request_map.at("name"s).AsString();
    builder.StartDict()
           .Key("request_id"s).Value(request_map.at("id"s).AsInt());
    if (!IsStopExists(stop_name)) {
        builder.Key("error_message"s).Value("not found"s);
        return builder.EndDict().Build();
    } 
    const std::unordered_set<std::string_view>* buses_ptr = GetBusesByStop(stop_name);
    builder.Key("buses"s).StartArray();
    if (buses_ptr) {
        const std::unordered_set<std::string_view>& buses = *buses_ptr;
        std::vector<std::string_view> sorted_buses_by_stop(buses.begin(), buses.end());
        std::sort(sorted_buses_by_stop.begin(), sorted_buses_by_stop.end()); 
        for (const std::string_view& bus : sorted_buses_by_stop) {
            builder.Value(std::string(bus));
        }
    }
    builder.EndArray();
    return builder.EndDict().Build();
}

json::Node RequestHandler::PrintMap(const json::Dict& request_map) const {
    json::Builder builder;
    builder.StartDict()
           .Key("request_id"s).Value(request_map.at("id"s).AsInt());
    
    std::ostringstream svg_stream;
    RenderMap().Render(svg_stream);
    builder.Key("map"s).Value(std::move(svg_stream.str()));
    
    return builder.EndDict().Build();
}

std::optional<transport::ExtendedRouteInfo> RequestHandler::GetExtendedRouteInfo (const std::string_view route_id) const {
    return catalogue_.GetExtendedRouteInfo(route_id);
}

const std::unordered_set<std::string_view>* RequestHandler::GetBusesByStop (std::string_view stop_name) const {
    return catalogue_.GetAllBusesByStop(stop_name);
}

svg::Document RequestHandler::RenderMap () const {
    return renderer_.GetSVG(catalogue_.GetSortedAllRoutes());
}

bool RequestHandler::IsRouteExists (std::string_view route_id) const {
    return catalogue_.IsRouteExists(route_id);
}
bool RequestHandler::IsStopExists (std::string_view stop_name) const {
    return catalogue_.IsStopExists(stop_name);
}