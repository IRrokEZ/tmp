#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <chrono>
#include <fstream>

int64_t FileTest (const std::string &filename, int n1, int n2) {
    std::ifstream in_file(filename);
    if (!in_file) {
        std::cerr << "Failed file open" << std::endl;
    }    

    std::ofstream outp("output.txt");

    auto start = std::chrono::high_resolution_clock::now();

    TransportCatalogue catalogue;

    int base_request_count = n1;

    {
        input_reader::InputReader reader;
        for (int i = 0; i < base_request_count; ++ i) {
            std::string line;
            std::getline(in_file, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count = n2;
    
    for (int i = 0; i < stat_request_count; ++ i) {
        std::string line;
        std::getline(in_file, line);
        ParseAndPrintStat(catalogue, line, outp);
    }

    in_file.close();
    outp.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return duration.count();
}

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
    auto t = FileTest("tsC_case1_input.txt", 1986, 2000);
    std::cout << std::endl << t << std::endl;
    //ManualTest();
    return 0;
}