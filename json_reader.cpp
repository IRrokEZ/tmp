#include "json_reader.h"

#include <map>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

using std::literals::string_literals::operator""s;

const json::Node* JsonReader::GetBaseRequests() const {
    const json::Dict& root = input_.GetRoot().AsMap();
    auto it = root.find("base_requests"s);
    return (it != root.end()) ? &it->second : nullptr;
}

const json::Node* JsonReader::GetRenderSettings() const {
    const json::Dict& root = input_.GetRoot().AsMap();
    auto it = root.find("render_settings"s);
    return (it != root.end()) ? &it->second : nullptr;
}

const json::Node* JsonReader::GetStatRequests() const {
    const json::Dict& root = input_.GetRoot().AsMap();
    auto it = root.find("stat_requests"s);
    return (it != root.end()) ? &it->second : nullptr;
}

void JsonReader::FillCatalogue(transport::Catalogue& catalogue) {
    if (const json::Node* base_requests_node = GetBaseRequests()) {
        const json::Array& arr = base_requests_node->AsArray();
        
        for (const json::Node& request : arr) {
            const json::Dict& request_map = request.AsMap();
            const std::string& type = request_map.at("type"s).AsString();
            if (type == "Stop"s) {
                FillStop(request_map, catalogue);
            }
        }

        for (const json::Node& request : arr) {
            const json::Dict& request_map = request.AsMap();
            const std::string& type = request_map.at("type"s).AsString();
            if (type == "Stop"s) {
                FillStopDistances(request_map, catalogue);
            }
        }

        for (const json::Node& request : arr) {
            const json::Dict& request_map = request.AsMap();
            const std::string& type = request_map.at("type"s).AsString();
            if (type == "Bus"s) {
                FillRoute(request_map, catalogue);
            }
        }
    } else {
        throw std::runtime_error("Missing 'base_requests' in JSON"s);
    }
}

void JsonReader::FillStop(const json::Dict& request_map, transport::Catalogue& catalogue) const {
    std::string stop_name = request_map.at("name"s).AsString();
    geo::Coordinates coordinates = { request_map.at("latitude"s).AsDouble(), request_map.at("longitude"s).AsDouble() };
    catalogue.AddStop(std::move(stop_name), coordinates);
}

void JsonReader::FillStopDistances(const json::Dict& request_map, transport::Catalogue& catalogue) const {
    const std::string& from_stop_name = request_map.at("name"s).AsString();
    const json::Dict& distances = request_map.at("road_distances"s).AsMap();
    for (const auto& [to_stop_name, distance] : distances) {
        const transport::Stop* from = catalogue.GetStop(from_stop_name);
        const transport::Stop* to = catalogue.GetStop(to_stop_name);
        if (from && to) {
            catalogue.SetDistance(from, to, distance.AsDouble());
        } else {
            throw std::domain_error("unknown stops in query"s);
        }
    }
}

void JsonReader::FillRoute(const json::Dict& request_map, transport::Catalogue& catalogue) const {
    std::string route_id = request_map.at("name"s).AsString();
    const json::Array input_stops = request_map.at("stops"s).AsArray();
    std::vector<const transport::Stop*> stops;
    stops.reserve(input_stops.size());
    for (const json::Node& stop : input_stops) {
        std::optional<const transport::Stop *> add = catalogue.GetStop(stop.AsString());
        if (add.has_value()) {
            stops.push_back(add.value());
        } else {
            throw std::domain_error("unknown stop in query: ["s + stop.AsString() + "]"s);
        }
    }

    catalogue.AddRoute(std::move(route_id), std::move(stops), request_map.at("is_roundtrip"s).AsBool());
}

renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width"s).AsDouble();
    render_settings.height = request_map.at("height"s).AsDouble();
    render_settings.padding = request_map.at("padding"s).AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius"s).AsDouble();
    render_settings.line_width = request_map.at("line_width"s).AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size"s).AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset"s).AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size"s).AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset"s).AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
    
    render_settings.underlayer_color = std::move(ParseColor(request_map.at("underlayer_color"s)));
    render_settings.underlayer_width = request_map.at("underlayer_width"s).AsDouble();

    const json::Array& color_palette = request_map.at("color_palette"s).AsArray();
    render_settings.color_palette.reserve(color_palette.size());
    for (const json::Node& color_node : color_palette) {
        render_settings.color_palette.push_back(std::move(ParseColor(color_node)));
    }
    renderer::MapRenderer result(render_settings);
    return result;
}

svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    } 
    
    if (node.IsArray()) {
        const json::Array& arr = node.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
        }
        if (arr.size() == 4) {
            return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
        }
        throw std::runtime_error("Invalid color array size"s);
    }
    throw std::runtime_error("Unsupported color format in JSON"s);
}