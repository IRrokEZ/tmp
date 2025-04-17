#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

void ManualTest () {
    TransportCatalogue catalogue;

    int base_request_count;
    std::cin >> base_request_count >> std::ws;

    {
        input_reader::InputReader reader;
        for (int i = 0; i < base_request_count; ++ i) {
            std::string line;
            std::getline(std::cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++ i) {
        std::string line;
        std::getline(std::cin, line);
        ParseAndPrintStat(catalogue, line, std::cout);
    }
}


int main() {    
    ManualTest();
    return 0;
}