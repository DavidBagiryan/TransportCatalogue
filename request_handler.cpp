#include "request_handler.h"

using namespace request_handler;
using namespace transport_catalogue;
using namespace json;
using namespace std::literals;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& catalog, const map_renderer::MapRender& map_catalog, const transport_router::RouterSettings& settings_router)
    : catalog_(catalog)
    , map_catalog_(map_catalog)
    , transport_router_(catalog, settings_router)
{
}

void RequestHandler::RequestProcess(json::Array& value, svg::Document& map_svg, std::ostream& output) {
    Builder request{};
    request.StartArray();
    for (auto& description : value) {
        if (description.AsMap().at("type"s) == "Stop"s) {
            StopInfoPrint(description.AsMap(), request);
        }
        else if (description.AsMap().at("type"s) == "Bus"s) {
            BusInfoPrint(description.AsMap(), request);
        }
        else if (description.AsMap().at("type"s) == "Map"s) {
            MapPrint(description.AsMap().at("id"s).AsInt(), map_svg, request);
        }
        else if (description.AsMap().at("type"s) == "Route"s) {
            RoutePrint(description.AsMap(), request);
        }
    }
    request.EndArray();
    Print(Document{ request.Build() }, output);
}

void RequestHandler::StopInfoPrint(const Dict& value, Builder& request) {
    int id = value.at("id"s).AsInt();
    std::string name = value.at("name"s).AsString();

    std::set<std::string_view> buses;

    if (catalog_.FindStop(name).name_ == "Error"s) {
        request.StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s);
    }
    else {
        request.StartDict()
            .Key("buses"s).StartArray();
        try {
            buses = catalog_.GetBusesToStops().at(name);
            for (auto& bus : buses) {
                request.Value(std::string(bus));
            }
        }
        catch (...) {}
        request.EndArray()
            .Key("request_id"s).Value(id);
    }
    request.EndDict();
}

void RequestHandler::BusInfoPrint(const Dict& value, Builder& request) {
    std::string name = value.at("name"s).AsString();
    int id = value.at("id"s).AsInt();
    double curvature;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

    Bus route = catalog_.FindBus(name);

    request.StartDict();
    if (route.name_ == "Error"s) {
        request.Key("request_id"s).Value(id)
               .Key("error_message"s).Value("not found"s);
    }
    else {
        BusInfo info = catalog_.GetBusInfo(value.at("name"s).AsString());
        curvature = info.curvature_;
        route_length = info.real_distance_length_;
        stop_count = info.stops_num_;
        unique_stop_count = info.unique_stops_num_;
        request.Key("curvature"s).Value(curvature)
            .Key("request_id"s).Value(id)
            .Key("route_length"s).Value(route_length)
            .Key("stop_count"s).Value(int(stop_count))
            .Key("unique_stop_count"s).Value(int(unique_stop_count));
    }
    request.EndDict();
}

void RequestHandler::MapPrint(int id, svg::Document& map_svg, Builder& request) {
    std::ostringstream svg;
    map_svg.Render(svg);

    request.StartDict()
        .Key("map"s).Value(svg.str())
        .Key("request_id"s).Value(id)
        .EndDict();
}

void RequestHandler::RoutePrint(const Dict& value, Builder& request) {
    const std::string from = value.at("from"s).AsString();
    const std::string to = value.at("to"s).AsString();
    int id = value.at("id"s).AsInt();

    std::optional<std::vector<transport_router::RouterEdge>> route = transport_router_.BuildRoute(from, to);

    if (!route.has_value()) {
        request.StartDict()
                  .Key("request_id"s).Value(id)
                  .Key("error_message"s).Value("not found"s)
               .EndDict();
        return;
    }

    double total_time = 0;
    int wait_time = transport_router_.GetSettings().bus_wait_time;
    json::Array items;
    for (const transport_router::RouterEdge& edge : route.value()) {
        total_time += edge.total_time;

        json::Dict item_wait = json::Builder{}
            .StartDict()
                .Key("type"s).Value("Wait"s)
                .Key("stop_name"s).Value(std::string(edge.stop_name_from))
                .Key("time"s).Value(wait_time)
            .EndDict()
            .Build().AsMap();
        json::Dict item_move = json::Builder{}
            .StartDict()
                .Key("type"s).Value("Bus"s)
                .Key("bus"s).Value(std::string(edge.bus_name))
                .Key("span_count"s).Value(edge.stop_count)
                .Key("time"s).Value(edge.total_time - wait_time)
            .EndDict()
            .Build().AsMap();

        items.push_back(item_wait);
        items.push_back(item_move);
    }
     request.StartDict()
                .Key("request_id"s).Value(id)
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value(items)
            .EndDict();
}