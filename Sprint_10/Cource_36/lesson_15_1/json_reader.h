#pragma once

#include <fstream>
#include <iostream>
#include <string_view>
#include <tuple>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input)) {}

    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;

    void FillCatalogue(transport::Catalogue& catalogue);
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;

private:
    json::Document input_;
    json::Node dummy_ = nullptr;

    std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> FillStop(const json::Dict& request_map) const;
    void FillStopDistances(transport::Catalogue& catalogue) const;
    std::tuple<std::string_view, std::vector<const transport::Stop*>, bool> FillRoute(const json::Dict& request_map, transport::Catalogue& catalogue) const;
};