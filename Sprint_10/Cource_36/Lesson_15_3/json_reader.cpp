#include "json_reader.h"

#include <set>
#include <sstream>
#include <string>

using std::literals::string_literals::operator""s;

namespace transport {

void JsonReader::FillCatalogue(transport::Catalogue& catalogue) {
    const json::Array& requests = input_.GetRoot().AsMap().at("base_requests"s).AsArray();
    
    for (const json::Node& request : requests) {
        const json::Dict& req_map = request.AsMap();
        if (req_map.at("type"s).AsString() == "Stop"s) {
            catalogue.AddStop(ParseStop(req_map));
        }
    }
    
    ParseDistances(catalogue);
    
    for (const json::Node& request : requests) {
        const json::Dict& req_map = request.AsMap();
        if (req_map.at("type").AsString() == "Bus") {
            catalogue.AddRoute(ParseBus(req_map, catalogue));
        }
    }
}

Stop JsonReader::ParseStop(const json::Dict& stop_dict) const {
    Stop result_stop;
    result_stop.name = stop_dict.at("name"s).AsString();
    result_stop.coordinates = {stop_dict.at("latitude"s).AsDouble(), 
                               stop_dict.at("longitude"s).AsDouble()};
    return result_stop;
}

Bus JsonReader::ParseBus(const json::Dict& bus_dict, const transport::Catalogue& catalogue) const {
    Bus result_bus;
    result_bus.number = bus_dict.at("name"s).AsString();
    result_bus.is_circle = bus_dict.at("is_roundtrip"s).AsBool();
    
    for (const json::Node& stop_name : bus_dict.at("stops"s).AsArray()) {
        const transport::Stop* stop = catalogue.FindStop(stop_name.AsString());
        if (stop != nullptr) {
            result_bus.stops.push_back(stop);
        }
    }
    
    return result_bus;
}

void JsonReader::ParseDistances(transport::Catalogue& catalogue) const {
    const json::Array& requests = input_.GetRoot().AsMap().at("base_requests"s).AsArray();
    
    for (const json::Node& request : requests) {
        const json::Dict& req_map = request.AsMap();
        if (req_map.at("type"s).AsString() != "Stop"s) {
            continue;
        }
        
        auto from_stop = catalogue.FindStop(req_map.at("name"s).AsString());
        if (!from_stop) {
            continue;
        }
        
        for (const auto& [to_name, dist_node] : req_map.at("road_distances"s).AsMap()) {
            const transport::Stop* to_stop = catalogue.FindStop(to_name);
            if (to_stop != nullptr) {
                catalogue.SetDistance(from_stop, to_stop, dist_node.AsInt());
            }
        }
    }
}

renderer::RenderSettings JsonReader::GetRenderSettings() const {
    const json::Dict& settings = input_.GetRoot().AsMap().at("render_settings"s).AsMap();
    
    renderer::RenderSettings result {
        .width = settings.at("width"s).AsDouble(),
        .height = settings.at("height"s).AsDouble(),
        .padding = settings.at("padding"s).AsDouble(),
        .stop_radius = settings.at("stop_radius"s).AsDouble(),
        .line_width = settings.at("line_width"s).AsDouble(),
        .bus_label_font_size = settings.at("bus_label_font_size"s).AsInt(),
        .bus_label_offset = {settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                            settings.at("bus_label_offset"s).AsArray()[1].AsDouble()},
        .stop_label_font_size = settings.at("stop_label_font_size"s).AsInt(),
        .stop_label_offset = {settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                            settings.at("stop_label_offset"s).AsArray()[1].AsDouble()}
    };
    
    const json::Node& underlayer_color = settings.at("underlayer_color"s);

    if (underlayer_color.IsString()) {
        result.underlayer_color = underlayer_color.AsString();
    } else if (underlayer_color.IsArray()) {
        const json::Array& color = underlayer_color.AsArray();
        if (color.size() == 3) {
            result.underlayer_color = svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
        } else if (color.size() == 4) {
            result.underlayer_color = svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
        }
    }
    
    result.underlayer_width = settings.at("underlayer_width"s).AsDouble();
    
    for (const json::Node& color_node : settings.at("color_palette"s).AsArray()) {
        if (color_node.IsString()) {
            result.color_palette.push_back(color_node.AsString());
        } else if (color_node.IsArray()) {
            const json::Array& color = color_node.AsArray();
            if (color.size() == 3) {
                result.color_palette.push_back(svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt()));
            } else if (color.size() == 4) {
                result.color_palette.push_back(svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble()));
            }
        }
    }
    
    return result;
}

json::Node JsonReader::ProcessBusRequest(const json::Dict& request, const transport::Catalogue& catalogue) const {
    json::Dict response;
    response["request_id"s] = request.at("id"s);
    
    const std::string& bus_name = request.at("name"s).AsString();
    const transport::Bus* bus = catalogue.FindRoute(bus_name);
    
    if (!bus) {
        response["error_message"s] = "not found"s;
        return response;
    } 

    const transport::BusStat stats = catalogue.GetBusStats(bus_name);
    response["curvature"s] = stats.curvature;
    response["route_length"s] = static_cast<int>(stats.route_length);
    response["stop_count"s] = static_cast<int>(stats.stops_count);
    response["unique_stop_count"s] = static_cast<int>(stats.unique_stops_count);
    
    return response;
}

json::Node JsonReader::ProcessStopRequest(const json::Dict& request, const transport::Catalogue& catalogue) const {
    json::Dict response;
    response["request_id"s] = request.at("id"s);
    
    const std::string& stop_name = request.at("name"s).AsString();
    const auto* stop = catalogue.FindStop(stop_name);
    
    if (!stop) {
        response["error_message"s] = "not found"s;
        return response;
    }
    
    const std::set<std::string> buses = catalogue.GetBusesForStop(stop_name);
    json::Array bus_names;
    for (const std::string& bus : buses) {
        bus_names.emplace_back(bus);
    }
    response["buses"s] = bus_names;
    
    return response;
}

json::Node JsonReader::ProcessMapRequest(const json::Dict& request, const transport::Catalogue& catalogue) const {
    json::Dict response;
    response["request_id"s] = request.at("id"s);
    
    renderer::MapRenderer renderer(GetRenderSettings());
    std::ostringstream svg_stream;
    renderer.GetSVG(catalogue.GetSortedAllBuses()).Render(svg_stream);
    response["map"s] = svg_stream.str();
    
    return response;
}

json::Document JsonReader::ProcessRequests(const transport::Catalogue& catalogue) const {
    json::Array responses;
    const json::Array& requests = input_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    
    for (const json::Node& request_node : requests) {
        const json::Dict& request = request_node.AsMap();
        const std::string& type = request.at("type"s).AsString();
        
        json::Node response;
        if (type == "Bus"s) {
            response = ProcessBusRequest(request, catalogue);
        } else if (type == "Stop"s) {
            response = ProcessStopRequest(request, catalogue);
        } else if (type == "Map"s) {
            response = ProcessMapRequest(request, catalogue);
        }
        
        responses.emplace_back(response);
    }
    
    return json::Document{responses};
}

} // namespace transport