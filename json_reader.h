#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace json_reader {

void ProcessBaseRequests(const json::Array& base_requests, transport_catalogue::TransportCatalogue& catalogue);
json::Array ProcessStatRequests(const transport_catalogue::TransportCatalogue& catalogue, const json::Array& stat_requests);

}  // namespace json_reader