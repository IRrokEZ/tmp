#include "map_renderer.h"

#include <utility>

using std::literals::string_literals::operator""s;

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document MapRenderer::GetSVG(const std::map<std::string_view, const transport::Bus*>& buses) const {
    svg::Document result;
    
    std::vector<geo::Coordinates> route_stops_coord;
    std::map<std::string_view, const transport::Stop*> all_stops;
    
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) {
            continue;
        }
        for (const auto& stop : bus->stops) {
            route_stops_coord.push_back(stop->coordinates);
            all_stops[stop->name] = stop;
        }
    }
    
    SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), 
                        render_settings_.width, render_settings_.height, 
                        render_settings_.padding);
    
    for (const auto& line : GetRouteLines(buses, sp)) {
        result.Add(line);
    }
    
    for (const auto& text : GetBusLabel(buses, sp)) {
        result.Add(text);
    }
    
    for (const auto& circle : GetStopsSymbols(all_stops, sp)) {
        result.Add(circle);
    }
    
    for (const auto& text : GetStopsLabels(all_stops, sp)) {
        result.Add(text);
    }

    return result;
}

std::vector<svg::Polyline> MapRenderer::GetRouteLines(const std::map<std::string_view,
                                                      const transport::Bus*>& buses, 
                                                      const SphereProjector& sp) const {
    std::vector<svg::Polyline> result;
    size_t color_num = 0;
    
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) {
            continue;
        }
        
        std::vector<const transport::Stop*> route_stops{bus->stops.begin(), bus->stops.end()};
        if (!bus->is_circle) {
            route_stops.insert(route_stops.end(), 
                              std::next(bus->stops.rbegin()), 
                              bus->stops.rend());
        }
        
        svg::Polyline line;
        for (const auto& stop : route_stops) {
            line.AddPoint(sp(stop->coordinates));
        }
        
        line.SetStrokeColor(render_settings_.color_palette[color_num])
            .SetFillColor("none"s)
            .SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        color_num = (color_num + 1) % render_settings_.color_palette.size();
        
        result.push_back(std::move(line));
    }
    
    return result;
}

std::vector<svg::Text> MapRenderer::GetBusLabel(const std::map<std::string_view,
                                                const transport::Bus*>& buses,
                                                const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    std::size_t color_num = 0;
    
    for (const auto& [bus_number, bus] : buses) {
        if (bus->stops.empty()) {
            continue;
        }
        
        const svg::Color& color = render_settings_.color_palette[color_num];
        color_num = (color_num + 1) % render_settings_.color_palette.size();
        
        auto create_bus_label = [&](const transport::Stop* stop) {
            svg::Text underlayer;
            underlayer.SetPosition(sp(stop->coordinates))
                     .SetOffset(render_settings_.bus_label_offset)
                     .SetFontSize(render_settings_.bus_label_font_size)
                     .SetFontFamily("Verdana"s)
                     .SetFontWeight("bold"s)
                     .SetData(bus->number)
                     .SetFillColor(render_settings_.underlayer_color)
                     .SetStrokeColor(render_settings_.underlayer_color)
                     .SetStrokeWidth(render_settings_.underlayer_width)
                     .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                     .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            svg::Text text;
            text.SetPosition(sp(stop->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus->number)
                .SetFillColor(color);
            
            return std::make_pair(underlayer, text);
        };
        
        auto [start_underlayer, start_text] = create_bus_label(bus->stops[0]);
        result.push_back(std::move(start_underlayer));
        result.push_back(std::move(start_text));
        
        if (!(bus->is_circle) && (bus->stops[0] != bus->stops.back())) {
            auto [end_underlayer, end_text] = create_bus_label(bus->stops.back());
            result.push_back(std::move(end_underlayer));
            result.push_back(std::move(end_text));
        }
    }
    
    return result;
}

std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const std::map<std::string_view,
                                                      const transport::Stop*>& stops,
                                                      const SphereProjector& sp) const {
    std::vector<svg::Circle> result;
    result.reserve(stops.size());
    
    for (const auto& [stop_name, stop] : stops) {
        result.emplace_back()
            .SetCenter(sp(stop->coordinates))
            .SetRadius(render_settings_.stop_radius)
            .SetFillColor("white"s);
    }
    
    return result;
}

std::vector<svg::Text> MapRenderer::GetStopsLabels(const std::map<std::string_view,
                                                   const transport::Stop*>& stops,
                                                   const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    result.reserve(stops.size() * 2);
    
    for (const auto& [stop_name, stop] : stops) {
        result.emplace_back()
            .SetPosition(sp(stop->coordinates))
            .SetOffset(render_settings_.stop_label_offset)
            .SetFontSize(render_settings_.stop_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetData(stop->name)
            .SetFillColor(render_settings_.underlayer_color)
            .SetStrokeColor(render_settings_.underlayer_color)
            .SetStrokeWidth(render_settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        result.emplace_back()
            .SetPosition(sp(stop->coordinates))
            .SetOffset(render_settings_.stop_label_offset)
            .SetFontSize(render_settings_.stop_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetData(stop->name)
            .SetFillColor("black"s);
    }
    
    return result;
}

} // namespace renderer