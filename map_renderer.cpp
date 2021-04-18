#include "map_renderer.h"

namespace map_renderer { 
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }
    /////MapRenderer Class part//////////////////////////////////////////////////////////  
    void MapRenderer::SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }

    void MapRenderer::SetStops(const std::map<std::string_view, const Stop*> stops) {
        stops_ = stops;
    }

    void MapRenderer::SetRoutes(const std::map<std::string_view, const Bus*> routes) {
        routes_ = routes;
    }

    void MapRenderer::Render(std::ostream& out_stream) {
        std::vector<geo::Coordinates> coords;
        for (const auto& [k, v] : stops_) {
            if (!v->buses.empty()) {
                coords.push_back(v->geo);
            }
        }
        SphereProjector projector(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);

        std::set<const Bus*, BusSort> routes_to_render;
        for (const auto& route : routes_) {
            if (!route.second->stops.empty()) {
                routes_to_render.insert(route.second);
            }
        }
        DrawBusRoutes(projector, routes_to_render);
        DrawRoutesNames(projector, routes_to_render);
        DrawStops(projector);
        DrawStopsNames(projector);
        doc_.Render(out_stream);

    }    

    void MapRenderer::DrawBusRoutes(SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render) {
        int color_index = 0;
        for (const auto& route : routes_to_render) {
            svg::Polyline line;
            for (const auto& stop : route->stops) {
                line.AddPoint(projector(stops_.at(stop)->geo));
            }
            line.SetFillColor("none");
            line.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()]);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeWidth(settings_.line_width);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            color_index++;
            doc_.Add(line);
        }
    }

    void MapRenderer::DrawRoutesNames(const SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render) {
        int color_index = 0;
        svg::Text under_text, text;
        for (const auto& route : routes_to_render) {
            std::string route_name{ route->name };
            std::vector<std::string_view> stops{ route->stops.front() };
            if (!route->is_round_trip && route->stops.front() != route->end_stop) {
                stops.push_back(route->end_stop);
            }
            for (const auto& stop : stops) {
                under_text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData(route_name)
                    .SetPosition(projector(stops_.at(stop)->geo));
                doc_.Add(under_text);
                text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                    .SetData(route_name)
                    .SetPosition(projector(stops_.at(stop)->geo));
                doc_.Add(text);
            }
            color_index++;
        }

    }

    void MapRenderer::DrawStops(const SphereProjector& projector) {
        svg::Circle circle;
        for (const auto& stop : stops_) {
            if (!stop.second->buses.empty()) {
                circle.SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s)
                    .SetCenter(projector(stop.second->geo));
                doc_.Add(circle);
            }
        }
    }

    void MapRenderer::DrawStopsNames(const SphereProjector& projector) {
        svg::Text text, under_text;
        for (const auto& stop : stops_) {
            if (!stop.second->buses.empty()) {
                std::string stop_name{ stop.second->name };
                under_text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData(stop_name)
                    .SetPosition(projector(stop.second->geo));
                doc_.Add(under_text);
                text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFillColor("black")
                    .SetData(stop_name)
                    .SetPosition(projector(stop.second->geo));
                doc_.Add(text);
            }

        }
    }
    
} // namespace map_renderer

