#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

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

    ProcessingRequest(content_state, map_svg);
    PrintResult();
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

////////// stat_requests //////////
void JsonReader::ProcessingRequest(Array& value, svg::Document& map_svg) {
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            StopInformationPrinting(description.AsMap());
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            BusInformationPrinting(description.AsMap());
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            MapPrinting(description.AsMap(), map_svg);
        }
    }
}

void JsonReader::StopInformationPrinting(const Dict& value) {
    int id;
    std::string name;
    Array bus_names;

    id = value.at("id"s).AsInt();
    name = value.at("name"s).AsString();

    std::set<std::string_view> buses;

    if (catalog_.FindStop(name).name_ == "Error"s) {
        print_.emplace_back(Dict{ {"request_id"s, id}, {"error_message"s, "not found"s} });
    }
    else {
        try {
            buses = catalog_.GetBusesToStops().at(name);
            for (auto& bus : buses) {
                bus_names.push_back(std::string(bus));
            }
        }
        catch (...) {}
        print_.emplace_back(Dict{ {"buses"s, bus_names}, {"request_id"s, id} });
    }

}

void JsonReader::BusInformationPrinting(const Dict& value) {
    std::string name;
    double curvature;
    int id;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

    name = value.at("name"s).AsString();
    id = value.at("id"s).AsInt();

    Bus route = catalog_.FindBus(name);
    if (route.name_ == "Error"s) {
        print_.emplace_back(Dict{ {"request_id"s, id}, {"error_message"s, "not found"s} });
    }
    else {
        BusInfo info = catalog_.GetBusInfo(value.at("name"s).AsString());
        curvature = info.curvature_;
        route_length = info.real_distance_length_;
        stop_count = info.stops_num_;
        unique_stop_count = info.unique_stops_num_;
        print_.emplace_back(Dict{ {"curvature"s, curvature}, {"request_id"s, id}, {"route_length"s, route_length}, {"stop_count"s, int(stop_count)}, {"unique_stop_count"s, int(unique_stop_count)} });
    }
}

void JsonReader::MapPrinting(const Dict& value, svg::Document& map_svg) {
    int id = value.at("id"s).AsInt();

    std::ostringstream svg;
    map_svg.Render(svg);

    print_.emplace_back(Dict{ {"map"s, svg.str()}, {"request_id"s, id} });
}

void JsonReader::PrintResult() {
    Print(Document{ print_ }, output_);
}

std::ostringstream& JsonReader::Result() {
    return output_;
}

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
            return svg::Rgb{static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                            static_cast<uint8_t>(color.AsArray()[2].AsInt())};
        }
        else if (color.AsArray().size() == 4) {
            return svg::Rgba{static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                             static_cast<uint8_t>(color.AsArray()[2].AsInt()),
                             color.AsArray()[3].AsDouble()};
        }
    }
    return svg::Color();
}