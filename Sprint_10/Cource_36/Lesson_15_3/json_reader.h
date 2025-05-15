#pragma once

#include <iostream>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport {

class JsonReader {
public:
    JsonReader (std::istream& input) 
        : input_(json::Load(input)) {}

    void FillCatalogue (transport::Catalogue& catalogue);
    renderer::RenderSettings GetRenderSettings () const;
    json::Document ProcessRequests (const transport::Catalogue& catalogue) const;

private:
    json::Document input_;

    Stop ParseStop (const json::Dict& stop_dict) const;
    Bus ParseBus (const json::Dict& bus_dict, const transport::Catalogue& catalogue) const;
    void ParseDistances (transport::Catalogue& catalogue) const;
    
    json::Node ProcessBusRequest (const json::Dict& request, const transport::Catalogue& catalogue) const;
    json::Node ProcessStopRequest (const json::Dict& request, const transport::Catalogue& catalogue) const;
    json::Node ProcessMapRequest (const json::Dict& request, const transport::Catalogue& catalogue) const;
};

} // namespace transport