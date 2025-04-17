#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace print_detail {

void PrintBusStat (TransportCatalogue& tansport_catalogue,
                   const std::string& route_id,
                   std::ostream& output);

void PrintStopStat (TransportCatalogue& tansport_catalogue,
                   const std::string& stop_name,
                   std::ostream& output);

} // namespace print_detail

void ParseAndPrintStat (TransportCatalogue& tansport_catalogue,
                        const std::string &request,
                        std::ostream& output);