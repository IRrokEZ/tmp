#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>
#include <sstream>

#include "stat_reader.h"
#include "input_reader.h"

using std::literals::string_view_literals::operator""sv;
using std::literals::string_literals::operator""s;

namespace print_detail {

void PrintBusStat (TransportCatalogue& tansport_catalogue,
                   std::string_view route_id,
                   std::ostream& output) {
    if (!tansport_catalogue.IsRouteExists(route_id)) {
        output << "not found"s << std::endl;
        return;
    }

    ExtendedRouteInfo info = tansport_catalogue.GetExtendedRouteInfo(route_id);

    output << info.all_stops_counter << " stops on route, "s
           << info.unique_stops_counter << " unique stops, "s
           << info.route_length << " route length, "s
           << info.curvature << " curvature" << std::endl;
}

void PrintStopStat (TransportCatalogue& tansport_catalogue,
                    std::string_view stop_name,
                    std::ostream& output) {
    if (!tansport_catalogue.IsStopExists(stop_name)) {
        output << "not found"s << std::endl;
        return;
    }

    auto result = tansport_catalogue.GetAllBusesByStop(stop_name);

    if (result.empty()) {
        output << "no buses"s << std::endl;
        return;
    }

    std::set<std::string_view> rez(result.begin(), result.end());

    output << "buses"s;
    for (const auto& el : rez) {
        output << " " << el;
    }
    
    output << std::endl;
}

} // namespace print_detail

void ParseAndPrintStat(TransportCatalogue& tansport_catalogue,
                       const std::string &request,
                       std::ostream& output) {
    std::string_view corrected_request = input_reader::Trim(request);
    std::size_t space_pos = corrected_request.find_first_of(' ');
    if (space_pos == 0) {
        throw std::logic_error("Incorrect request: unknown format"s);
        return;
    }

    std::string_view command = input_reader::Trim(corrected_request.substr(0, space_pos));
    if ((command != "Bus"sv) && (command != "Stop"sv)) {
        throw std::logic_error("Incorrect request: incorrect command"s);
        return;
    }

    std::string_view object_name = input_reader::Trim(corrected_request.substr(space_pos + 1));

    output << request << ": "s;

    if (command == "Bus"sv) {
        print_detail::PrintBusStat(tansport_catalogue, object_name, output);
        return;
    }

    if (command == "Stop"sv) {
        print_detail::PrintStopStat(tansport_catalogue, object_name, output); 
        return;
    }
}
