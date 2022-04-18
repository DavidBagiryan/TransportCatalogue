#include "map_renderer.h"
#include "svg.h"

#include <algorithm>

using namespace map_renderer;
using namespace std::literals;

MapRender& MapRender::SetBuses(const std::deque<const transport_catalogue::Bus*> buses) {
    buses_ = buses;
    return *this;
}
MapRender& MapRender::SetStopCoordinates(const std::vector<geo::Coordinates> stops_coordinates) {
    stops_coordinates_ = stops_coordinates;
    return *this;
}
MapRender& MapRender::SetRenderSettings(const RenderSettings& settings) {
    settings_ = settings;
    return *this;
}

MapRender& MapRender::MapRendering(svg::Document& map_svg) {
    if (stops_coordinates_.empty()) return *this;
    
    SetMaxMinCoordinate();
    SetZoomCoeff();

    DrawPolylineRoute(map_svg);
    DrawNameRoute(map_svg);
    DrawCircleStop(map_svg);
    DrawNameStop(map_svg);

    return *this;
}

void MapRender::DrawPolylineRoute(svg::Document& map_svg) {
    std::vector<const transport_catalogue::Stop*> uniq_stop;

    for (size_t i = 0; i < buses_.size(); ++i) {
        svg::Polyline route_line;
        size_t number_color = i % settings_.color_palette.size();
        uniq_stop.reserve(uniq_stop.size() + buses_[i]->stops_of_bus_.size());
        for (const auto& stop : buses_[i]->stops_of_bus_) {
            route_line.AddPoint(CoordinateCalculation(stop->coordinates));

            uniq_stop.push_back(stop);
        }
        if (buses_[i]->loop_ == transport_catalogue::RouteType::NOT_LOOPED) {
            for (auto rev_it = ++buses_[i]->stops_of_bus_.rbegin(); rev_it != buses_[i]->stops_of_bus_.rend(); ++rev_it)
                route_line.AddPoint(CoordinateCalculation((*rev_it)->coordinates));
        }
        route_line.SetFillColor(svg::NoneColor)
            .SetStrokeColor(settings_.color_palette[number_color])
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        map_svg.Add(route_line);
    }

    std::sort(uniq_stop.begin(), uniq_stop.end(), [](auto lhs, auto rhs) {
        return lhs->name_ < rhs->name_;
        });
    uniq_stop.erase(unique(uniq_stop.begin(), uniq_stop.end()), uniq_stop.end());
    
    uniq_stop_ = std::move(uniq_stop);
}

void MapRender::DrawNameRoute(svg::Document& map_svg) {
    for (size_t i = 0; i < buses_.size(); ++i) {
        svg::Point first_stop = CoordinateCalculation(buses_[i]->stops_of_bus_[0]->coordinates);
        svg::Text route_substrate;
        svg::Text route_name;
        size_t number_color = i % settings_.color_palette.size();
        
        route_substrate.SetFillColor(settings_.underlayer_color);
        FillText(PointType::ROUTE, TextType::SUBSTRATE, route_substrate, first_stop, buses_[i]->name_);
        route_name.SetFillColor(settings_.color_palette[number_color]);
        FillText(PointType::ROUTE, TextType::NAME, route_name, first_stop, buses_[i]->name_);

        map_svg.Add(route_substrate);
        map_svg.Add(route_name);

        if (buses_[i]->loop_ == transport_catalogue::RouteType::NOT_LOOPED && 
           buses_[i]->stops_of_bus_[0] != buses_[i]->stops_of_bus_.back()) {
            svg::Point last_stop = CoordinateCalculation(buses_[i]->stops_of_bus_.back()->coordinates);
            
            route_substrate.SetFillColor(settings_.underlayer_color);
            FillText(PointType::ROUTE, TextType::SUBSTRATE, route_substrate, last_stop, buses_[i]->name_);
            route_name.SetFillColor(settings_.color_palette[number_color]);
            FillText(PointType::ROUTE, TextType::NAME, route_name, last_stop, buses_[i]->name_);
            
            map_svg.Add(route_substrate);
            map_svg.Add(route_name);
        }
    }
}

void MapRender::DrawCircleStop(svg::Document& map_svg) {
    for (const auto& stop : uniq_stop_) {
        svg::Circle stop_symbol;
        stop_symbol.SetCenter(CoordinateCalculation(stop->coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"s);
        map_svg.Add(stop_symbol);
    }
}

void MapRender::DrawNameStop(svg::Document& map_svg) {
    for (const auto& stop : uniq_stop_) {
        svg::Text stop_substrate;
        svg::Text stop_name;
        
        stop_substrate.SetFillColor(settings_.underlayer_color);
        FillText(PointType::STOP, TextType::SUBSTRATE, stop_substrate, CoordinateCalculation(stop->coordinates), stop->name_);
        stop_name.SetFillColor("black"s);
        FillText(PointType::STOP, TextType::NAME, stop_name, CoordinateCalculation(stop->coordinates), stop->name_);
        
        map_svg.Add(stop_substrate);
        map_svg.Add(stop_name);
    }
}

void MapRender::SetMaxMinCoordinate() {
    const auto [left_it, right_it] = std::minmax_element(stops_coordinates_.begin(), stops_coordinates_.end(),
        [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
        });

    const auto [bottom_it, top_it] = std::minmax_element(stops_coordinates_.begin(), stops_coordinates_.end(),
        [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
        });
    geo::Coordinates max{top_it->lat, right_it->lng};
    geo::Coordinates min{bottom_it->lat, left_it->lng};
    max_min_ = std::make_pair(max, min);
}

void MapRender::SetZoomCoeff() {
    std::optional<double> width_zoom_coeff;
    if (!(std::abs(max_min_.first.lng - max_min_.second.lng) < 1e-6)) {
        width_zoom_coeff = (settings_.width - 2 * settings_.padding) / (max_min_.first.lng - max_min_.second.lng);
    }

    std::optional<double> height_zoom_coeff;
    if (!(std::abs(max_min_.first.lat - max_min_.second.lat) < 1e-6)) {
        height_zoom_coeff = (settings_.height - 2 * settings_.padding) / (max_min_.first.lat - max_min_.second.lat);
    }

    if (width_zoom_coeff && height_zoom_coeff) {
        zoom_coeff_ = std::min(*width_zoom_coeff, *height_zoom_coeff);
    }
    else if (width_zoom_coeff) {
        zoom_coeff_ = *width_zoom_coeff;
    }
    else if (height_zoom_coeff) {
        zoom_coeff_ = *height_zoom_coeff;
    }
}

svg::Point MapRender::CoordinateCalculation(geo::Coordinates coordinate) {
    return {(coordinate.lng - max_min_.second.lng) * zoom_coeff_ + settings_.padding,
            (max_min_.first.lat - coordinate.lat) * zoom_coeff_ + settings_.padding};
}

void MapRender::FillText(PointType point_type, TextType text_type, svg::Text& text_svg, svg::Point stop_coordinate, std::string data) {
    text_svg.SetPosition(stop_coordinate)
        .SetFontFamily("Verdana"s)
        .SetData(data);
    
    if (point_type == PointType::ROUTE) {
        text_svg.SetFontWeight("bold"s)
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size);
    } else {
        text_svg.SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size);
    }
    if (text_type == TextType::SUBSTRATE) {
        text_svg.SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
}