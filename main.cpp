#include "transport_catalogue.h"
#include "json_reader.h"
#include "json.h"

#include <iostream>

int main() {
    auto doc = json::Load(std::cin);
    const auto& root_map = doc.GetRoot().AsMap();
    
    transport_catalogue::TransportCatalogue catalogue;
    json_reader::ProcessBaseRequests(root_map.at("base_requests").AsArray(), catalogue);
    
    auto responses = json_reader::ProcessStatRequests(
        catalogue, 
        root_map.at("stat_requests").AsArray()
    );
    
    json::Print(json::Document(responses), std::cout);
    
    return 0;
}