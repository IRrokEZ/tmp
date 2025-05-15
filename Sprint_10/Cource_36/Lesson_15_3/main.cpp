#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

int main() {
    transport::Catalogue catalogue;
    transport::JsonReader reader(std::cin);
    
    reader.FillCatalogue(catalogue);
    
    json::Print(reader.ProcessRequests(catalogue), std::cout);
    
    return 0;
}