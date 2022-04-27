#pragma once

#include "transport_catalogue.h"
#include "svg.h"

namespace map_renderer {
    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset{ 0.0, 0.0 };
        int stop_label_font_size = 0;
        svg::Point stop_label_offset{ 0.0, 0.0 };
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };

    enum class PointType {
        ROUTE,
        STOP
    };

    enum class TextType {
        SUBSTRATE,
        NAME
    };

    class MapRender {
    public:
        MapRender() = default;

        MapRender& SetBuses(const std::deque<const transport_catalogue::Bus*> buses);
        MapRender& SetStopCoordinates(const std::vector<geo::Coordinates> stops_coordinates);
        MapRender& SetRenderSettings(const RenderSettings& settings);

        MapRender& MapRendering(svg::Document& map_svg);

    private:
        RenderSettings settings_;
        std::deque<const transport_catalogue::Bus*> buses_;
        std::vector<const transport_catalogue::Stop*> uniq_stop_;
        std::vector<geo::Coordinates> stops_coordinates_;
        std::pair<geo::Coordinates, geo::Coordinates> max_min_;
        double zoom_coeff_ = 0.0;

        void DrawPolylineRoute(svg::Document& map_svg);
        void DrawNameRoute(svg::Document& map_svg);
        void DrawCircleStop(svg::Document& map_svg);
        void DrawNameStop(svg::Document& map_svg);

        void SetMaxMinCoordinate();
        void SetZoomCoeff();
        svg::Point CoordinateCalculation(geo::Coordinates coordinate);
        void FillText(PointType point_type, TextType text_type, svg::Text& text_svg, svg::Point stop_coordinate, std::string data);
    };
} // конец пространства имен map_renderer