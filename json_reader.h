#pragma once

#include <iostream>
#include <map>
#include <string_view>
#include <tuple>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

class JsonReader {
public:
    explicit JsonReader(std::istream& input)
        : input_(json::Load(input)) {}

    const json::Node* GetBaseRequests() const;
    const json::Node* GetRenderSettings() const;
    const json::Node* GetStatRequests() const;

    void FillCatalogue(transport::Catalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

private:
    json::Document input_;

    void FillStop(const json::Dict& request_map, transport::Catalogue& catalogue) const;
    void FillStopDistances(const json::Dict& request_map, transport::Catalogue& catalogue) const;
    void FillRoute(const json::Dict& request_map, transport::Catalogue& catalogue) const;
};

svg::Color ParseColor(const json::Node& node);