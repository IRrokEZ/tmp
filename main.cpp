#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

int main() {
    std::ifstream inpf;
    inpf.open("test1.json");
    std::ofstream outf;
    outf.open("responce.json", std::ios::trunc);
        std::fstream f;
        f.open("chrono.txt", std::ios::app);
    transport::Catalogue catalogue;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::tm local_time;
        localtime_s(&local_time, &now_time);

        std::ostringstream oss;
        oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        f << oss.str() << "\n";
        auto start = std::chrono::high_resolution_clock::now();
    //JsonReader json_doc(std::cin);
    JsonReader json_doc(inpf);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "JsonReader json_doc(std::cin) :" << duration.count() << "\n";

    
        start = std::chrono::high_resolution_clock::now();
    json_doc.FillCatalogue(catalogue);
    
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "json_doc.FillCatalogue(catalogue) :" << duration.count() << "\n";
    
    
        start = std::chrono::high_resolution_clock::now();
    const auto& render_settings = json_doc.GetRenderSettings()->AsMap();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "const auto& render_settings = json_doc.GetRenderSettings().AsMap() :" << duration.count() << "\n";

    
        start = std::chrono::high_resolution_clock::now();
    const auto& renderer = json_doc.FillRenderSettings(render_settings);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "const auto& renderer = json_doc.FillRenderSettings(render_settings) :" << duration.count() << "\n";

    
        start = std::chrono::high_resolution_clock::now();
    const auto& stat_requests = json_doc.GetStatRequests()->AsArray();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "const auto& stat_requests = json_doc.GetStatRequests().AsArray() :" << duration.count() << "\n";
    

    
        start = std::chrono::high_resolution_clock::now();
    RequestHandler rh(catalogue, renderer);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "RequestHandler rh(catalogue, renderer) :" << duration.count() << "\n";

    
        start = std::chrono::high_resolution_clock::now();
    //rh.ProcessRequests(stat_requests);
    rh.ProcessRequests(stat_requests, outf);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        f << "rh.ProcessRequests(stat_requests) :" << duration.count() << "\n";

        f << "\n\n\n";
        f.close();
    inpf.close();
    outf.close();
    return 0;
}