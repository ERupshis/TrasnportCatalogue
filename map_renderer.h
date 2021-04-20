#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <map>


namespace transport_db {
    using namespace std::literals;

    struct RenderSettings {
        using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
        double width = 1200.0;
        double height = 1200.0;
        double padding = 50.0;
        double line_width = 14.0;
        double stop_radius = 5.0;
        int bus_label_font_size = 20;
        double bus_label_offset[2] = { 7.0, 15.0 };
        int stop_label_font_size = 20;
        double stop_label_offset[2] = { 7.0, -3.0 };
        Color underlayer_color = svg::Rgba{255, 255, 255, 0.85};
        double underlayer_width = 3.0;
        std::vector<Color> color_palette /*= { "green"s, svg::Rgb{255,160, 0 }, "red"s }*/;
    };
}
namespace map_renderer {
    using namespace std::literals;
    using namespace transport_db;   
    using namespace domain;

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding);

        svg::Point operator()(transport_db::geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
/////MapRenderer Class part//////////////////////////////////////////////////////////  
    class MapRenderer {
    public:
        MapRenderer() = default;

        void SetSettings(const RenderSettings& settings);
        
        void SetStops(const std::map<std::string_view, const Stop*> stops);

        void SetRoutes(const std::map<std::string_view, const Bus*> routes);

        void Render(std::ostream& out_stream);
    private:
        RenderSettings settings_;
        svg::Document doc_;
        std::map<std::string_view, const Stop*> stops_;
        std::map<std::string_view, const Bus*> routes_;
        //////////////////////////////////////////////////////////////////////////////////////
        struct BusSort {
            bool operator()(const Bus* lhs, const Bus* rhs) const {
                return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(),
                    rhs->name.begin(), rhs->name.end());
            }
        };

        void RenderBusRoutes(SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render);
        void RenderRoutesNames(const SphereProjector& projector, const std::set<const Bus*, BusSort>& routes_to_render);
        void RenderStops(const SphereProjector& projector);
        void RenderStopsNames(const SphereProjector& projector);        
    };
} // namespace map_renderer


/////SphereProjector template methods/////////////////////////////////
template <typename PointInputIt>
map_renderer::SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
    double max_height, double padding)
    : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
        return lhs.lng < rhs.lng;
            });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
        return lhs.lat < rhs.lat;
            });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}
 
