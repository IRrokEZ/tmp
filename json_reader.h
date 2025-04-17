#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace json_reader {

class JsonReader {
public:
    JsonReader(TransportCatalogue& catalogue, request_handler::RequestHandler& handler);
    
    void LoadData(std::istream& input);
    void ProcessRequests(std::istream& input, std::ostream& output);
    
private:
    void ParseBaseRequests(const json::Array& requests);
    void ParseStatRequests(const json::Array& requests, std::ostream& output);
    json::Node CreateStopNode(const json::Dict& stop_map);
    json::Node CreateBusNode(const json::Dict& bus_map);
    json::Node ProcessBusRequest(const json::Dict& request_map);
    json::Node ProcessStopRequest(const json::Dict& request_map);
    
    TransportCatalogue& catalogue_;
    request_handler::RequestHandler& handler_;
};

} // namespace json_reader