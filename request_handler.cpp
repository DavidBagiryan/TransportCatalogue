#include "request_handler.h"

using namespace request_handler;
using namespace transport_catalogue;
using namespace json;
using namespace std::literals;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& catalog, const map_renderer::MapRender& map_catalog)
	: catalog_(catalog)
	, map_catalog_(map_catalog)
{
}

void RequestHandler::RequestProcess(json::Array& value, svg::Document& map_svg, json::Array& print) {
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            print.emplace_back(StopInfoPrint(description.AsMap()));
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            print.emplace_back(BusInfoPrint(description.AsMap()));
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            print.emplace_back(MapPrint(description.AsMap().at("id"s).AsInt(), map_svg));
        }
    }
}

Dict RequestHandler::StopInfoPrint(const Dict& value) {
    int id;
    std::string name;
    Array bus_names;

    id = value.at("id"s).AsInt();
    name = value.at("name"s).AsString();

    std::set<std::string_view> buses;

    if (catalog_.FindStop(name).name_ == "Error"s) {
        return Dict{{"request_id"s, id}, {"error_message"s, "not found"s}};
    }
    else {
        try {
            buses = catalog_.GetBusesToStops().at(name);
            for (auto& bus : buses) {
                bus_names.push_back(std::string(bus));
            }
        }
        catch (...) {}
        return Dict{{"buses"s, bus_names}, {"request_id"s, id}};
    }

}

Dict RequestHandler::BusInfoPrint(const Dict& value) {
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
        return Dict{{"request_id"s, id}, {"error_message"s, "not found"s}};
    }
    else {
        BusInfo info = catalog_.GetBusInfo(value.at("name"s).AsString());
        curvature = info.curvature_;
        route_length = info.real_distance_length_;
        stop_count = info.stops_num_;
        unique_stop_count = info.unique_stops_num_;
        return Dict{{"curvature"s, curvature}, {"request_id"s, id}, {"route_length"s, route_length}, {"stop_count"s, int(stop_count)}, {"unique_stop_count"s, int(unique_stop_count)}};
    }
}

Dict RequestHandler::MapPrint(int id, svg::Document& map_svg) {
    std::ostringstream svg;
    map_svg.Render(svg);

    return Dict{{"map"s, svg.str()}, {"request_id"s, id}};
}