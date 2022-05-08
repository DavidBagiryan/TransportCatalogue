#include "json_reader.h"

using namespace transport_catalogue;
using namespace json_reader;
using namespace map_renderer;
using namespace json;

void JsonReader::Reader() {
    Node document_json = Load(std::cin).GetRoot();
    Dict root = document_json.AsMap();
    Array content_base;
    Array content_state;
    Dict render_settings;
    try {
        content_base = root.at("base_requests"s).AsArray();
        content_state = root.at("stat_requests"s).AsArray();
        render_settings = root.at("render_settings"s).AsMap();
    }
    catch (...) {};

    FillCatalog(content_base);

    svg::Document map_svg;
    if (!render_settings.empty()) {
        RenderSettings settings = SetSettingsMap(render_settings);
        map_catalog_.SetBuses(catalog_.GetBuses())
            .SetStopCoordinates(catalog_.GetAllStopsCoordinates())
            .SetRenderSettings(settings)
            .MapRendering(map_svg);
    }

    request_handler::RequestHandler processing(catalog_, map_catalog_);
    processing.RequestProcess(content_state, map_svg, output_);
}

////////// base_requests //////////
void JsonReader::FillCatalog(Array& value) {

    std::vector<json::Node> bus_descriptions;
    std::map<std::string, Dict> road_distances;
    std::vector<std::string> bus_names;
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            AddStop(description.AsMap());
            road_distances[description.AsMap().at("name"s).AsString()] = description.AsMap().at("road_distances"s).AsMap();
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            bus_descriptions.push_back(description.AsMap());
            bus_names.push_back(description.AsMap().at("name"s).AsString());
        }
    }

    for (Node& description : bus_descriptions) {
        AddBus(description.AsMap());
    }

    for (auto& [stop_one, other] : road_distances) {
        for (auto& [stop_two, distance] : other) {
            catalog_.SetDistanceBetweenStops(stop_one, stop_two, distance.AsInt());
        }
    }

}

void JsonReader::AddStop(const Dict& stop) {
    std::string name;
    double latitude;
    double longitude;

    name = stop.at("name"s).AsString();
    latitude = stop.at("latitude"s).AsDouble();
    longitude = stop.at("longitude"s).AsDouble();

    catalog_.AddStop(name, { latitude, longitude });
}

void JsonReader::AddBus(const Dict& bus) {
    std::string name;
    Array stope;
    transport_catalogue::RouteType is_roundtrip;

    name = bus.at("name"s).AsString();;

    stope = bus.at("stops"s).AsArray();
    std::vector<const Stop*> stops;
    for (auto& kek : stope) {
        stops.push_back(&catalog_.FindStop(kek.AsString()));
    }

    bus.at("is_roundtrip"s).AsBool() ? is_roundtrip = transport_catalogue::RouteType::IS_LOOPED : is_roundtrip = transport_catalogue::RouteType::NOT_LOOPED;

    catalog_.AddBus(name, stops, is_roundtrip);
}
//-------- base_requests //--------

////////// render_settings //////////
RenderSettings JsonReader::SetSettingsMap(Dict& render_settings) {
    RenderSettings settings;

    settings.width = render_settings.at("width").AsDouble();
    settings.height = render_settings.at("height").AsDouble();
    settings.padding = render_settings.at("padding").AsDouble();
    settings.line_width = render_settings.at("line_width").AsDouble();
    settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    settings.bus_label_offset = { render_settings.at("bus_label_offset").AsArray()[0].AsDouble(),
                                 render_settings.at("bus_label_offset").AsArray()[1].AsDouble() };
    settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    settings.stop_label_offset = { render_settings.at("stop_label_offset").AsArray()[0].AsDouble(),
                                 render_settings.at("stop_label_offset").AsArray()[1].AsDouble() };
    settings.underlayer_color = GetColor(render_settings.at("underlayer_color"));
    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();
    for (const auto& color : render_settings.at("color_palette").AsArray()) {
        settings.color_palette.emplace_back(GetColor(color));
    }

    return settings;
}

const svg::Color JsonReader::GetColor(const Node& color) {
    if (color.IsString()) {
        return svg::Color{ color.AsString() };
    }
    else if (color.IsArray()) {
        if (color.AsArray().size() == 3) {
            return svg::Rgb{ static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[2].AsInt()) };
        }
        else if (color.AsArray().size() == 4) {
            return svg::Rgba{ static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[2].AsInt()),
                             color.AsArray()[3].AsDouble() };
        }
    }
    return svg::Color();
}