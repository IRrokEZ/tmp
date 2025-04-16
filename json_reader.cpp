#include "domain.h"
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

using std::literals::string_literals::operator""s;

namespace json_reader {

void ProcessBaseRequests(const json::Array& base_requests,
                         transport_catalogue::TransportCatalogue &catalogue) {
    // Сначала обрабатываем все остановки
    for (const auto& request : base_requests) {
        const auto& req_map = request.AsMap();
        if (req_map.at("type").AsString() == "Stop") {
            domain::Stop stop;
            stop.name = req_map.at("name"s).AsString();
            stop.coordinates = {
                req_map.at("latitude"s).AsDouble(),
                req_map.at("longitude"s).AsDouble()
            };
            
            const auto& distances = req_map.at("road_distances"s).AsMap();
            for (const auto& [neighbor, dist_node] : distances) {
                stop.road_distances[neighbor] = dist_node.AsInt();
            }
            
            catalogue.AddStop(stop);
        }
    }
    
    // Затем обрабатываем маршруты
    for (const auto& request : base_requests) {
        const auto& req_map = request.AsMap();
        if (req_map.at("type"s).AsString() == "Bus"s) {
            domain::Bus bus;
            bus.number = req_map.at("name"s).AsString();
            bus.is_roundtrip = req_map.at("is_roundtrip"s).AsBool();
            
            const auto& stops = req_map.at("stops"s).AsArray();
            for (const auto& stop : stops) {
                bus.stops.push_back(stop.AsString());
            }
            
            catalogue.AddBus(bus);
            
            for (size_t i = 0; i < bus.stops.size() - 1; ++ i) {
                const auto& from = bus.stops[i];
                const auto& to = bus.stops[i+1];
                const auto* stop_from = catalogue.GetStop(from);
                if (stop_from && (stop_from->road_distances.count(to))) {
                    catalogue.SetDistance(from, to, stop_from->road_distances.at(to));
                }
            }
        }
    }
}

json::Node ProcessStatRequest(const transport_catalogue::TransportCatalogue& catalogue, const json::Node& request) {
    const auto& req_map = request.AsMap();
    const std::string& type = req_map.at("type"s).AsString();
    int id = req_map.at("id"s).AsInt();
    
    if (type == "Bus"s) {
        const std::string& name = req_map.at("name"s).AsString();
        const domain::Bus* bus = catalogue.GetBus(name);
        
        if (!bus) {
            return Builder{}
                .StartDict()
                    .Key("request_id"s).Value(id)
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build();
        }
        
        auto info = catalogue.GetRouteInfo(name);
        return Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("stop_count"s).Value(static_cast<int>(info.stops_count))
                .Key("unique_stop_count"s).Value(static_cast<int>(info.unique_stops_count))
                .Key("route_length"s).Value(static_cast<int>(info.route_length))
                .Key("curvature"s).Value(info.curvature)
            .EndDict()
            .Build();
    }
    else if (type == "Stop"s) {
        const std::string& name = req_map.at("name"s).AsString();
        const domain::Stop* stop = catalogue.GetStop(name);
        
        if (!stop) {
            return Builder{}
                .StartDict()
                    .Key("request_id"s).Value(id)
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build();
        }
        
        auto buses = catalogue.GetBusesForStop(name);
        json::Array buses_array;
        for (const auto& bus : buses) {
            buses_array.emplace_back(bus);
        }
        
        return Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("buses"s).Value(buses_array)
            .EndDict()
            .Build();
    }
    
    return json::Node{};
}

} // namespace json_reader