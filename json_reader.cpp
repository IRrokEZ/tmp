#include "json_reader.h"

#include <algorithm>
#include <sstream>

namespace json_reader {

using namespace std::literals;

JsonReader::JsonReader(TransportCatalogue& catalogue, request_handler::RequestHandler& handler)
    : catalogue_(catalogue), handler_(handler) {
}

void JsonReader::LoadData(std::istream& input) {
    json::Document doc = json::Load(input);
    const json::Dict& root_map = doc.GetRoot().AsMap();
    
    if (root_map.count("base_requests"s)) {
        ParseBaseRequests(root_map.at("base_requests"s).AsArray());
    }
}

void JsonReader::ProcessRequests(std::istream& input, std::ostream& output) {
    json::Document doc = json::Load(input);
    const json::Dict& root_map = doc.GetRoot().AsMap();
    
    json::Array result;
    if (root_map.count("stat_requests"s)) {
        ParseStatRequests(root_map.at("stat_requests"s).AsArray(), output);
    }
}

void JsonReader::ParseBaseRequests(const json::Array& requests) {
    // Сначала обрабатываем все остановки
    for (const auto& request_node : requests) {
        const json::Dict& request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Stop"s) {
            CreateStopNode(request_map);
        }
    }
    
    // Затем обрабатываем маршруты
    for (const auto& request_node : requests) {
        const json::Dict& request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Bus"s) {
            CreateBusNode(request_map);
        }
    }
}

json::Node JsonReader::CreateStopNode(const json::Dict& stop_map) {
    domain::Stop stop;
    stop.name = stop_map.at("name"s).AsString();
    stop.coordinates.lat = stop_map.at("latitude"s).AsDouble();
    stop.coordinates.lng = stop_map.at("longitude"s).AsDouble();
    
    catalogue_.AddStop(stop.name, stop.coordinates);
    
    // Добавляем расстояния до соседних остановок
    if (stop_map.count("road_distances"s)) {
        const json::Dict& road_distances = stop_map.at("road_distances"s).AsMap();
        for (const auto& [neighbor_stop, distance_node] : road_distances) {
            catalogue_.SetDistance(stop.name, neighbor_stop, distance_node.AsInt());
        }
    }
    
    return json::Node(nullptr);
}

json::Node JsonReader::CreateBusNode(const json::Dict& bus_map) {
    domain::Bus bus;
    bus.name = bus_map.at("name"s).AsString();
    bus.is_roundtrip = bus_map.at("is_roundtrip"s).AsBool();
    
    const json::Array& stops_array = bus_map.at("stops"s).AsArray();
    std::vector<std::string> stops;
    stops.reserve(stops_array.size());
    
    for (const auto& stop_node : stops_array) {
        stops.push_back(stop_node.AsString());
    }
    
    catalogue_.AddBus(bus.name, stops, bus.is_roundtrip);
    return json::Node(nullptr);
}

void JsonReader::ParseStatRequests(const json::Array& requests, std::ostream& output) {
    json::Array result;
    
    for (const auto& request_node : requests) {
        const json::Dict& request_map = request_node.AsMap();
        std::string type = request_map.at("type"s).AsString();
        
        if (type == "Bus"s) {
            result.push_back(ProcessBusRequest(request_map));
        } else if (type == "Stop"s) {
            result.push_back(ProcessStopRequest(request_map));
        }
    }
    
    json::Print(json::Document{json::Node(result)}, output);
}

json::Node JsonReader::ProcessBusRequest(const json::Dict& request_map) {
    int id = request_map.at("id"s).AsInt();
    std::string bus_name = request_map.at("name"s).AsString();
    
    if (!catalogue_.IsRouteExists(bus_name)) {
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    
    auto route_info = catalogue_.GetExtendedRouteInfo(bus_name);
    
    return json::Builder{}
        .StartDict()
            .Key("curvature"s).Value(route_info.curvature)
            .Key("request_id"s).Value(id)
            .Key("route_length"s).Value(static_cast<int>(route_info.route_length))
            .Key("stop_count"s).Value(static_cast<int>(route_info.all_stops_counter))
            .Key("unique_stop_count"s).Value(static_cast<int>(route_info.unique_stops_counter))
        .EndDict()
        .Build();
}

json::Node JsonReader::ProcessStopRequest(const json::Dict& request_map) {
    int id = request_map.at("id"s).AsInt();
    std::string stop_name = request_map.at("name"s).AsString();
    
    if (!catalogue_.IsStopExists(stop_name)) {
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    
    auto buses = catalogue_.GetAllBusesByStop(stop_name);
    std::vector<std::string> sorted_buses(buses.begin(), buses.end());
    std::sort(sorted_buses.begin(), sorted_buses.end());
    
    return json::Builder{}
        .StartDict()
            .Key("buses"s).Value(sorted_buses)
            .Key("request_id"s).Value(id)
        .EndDict()
        .Build();
}

} // namespace json_reader