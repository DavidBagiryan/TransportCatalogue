#include "transport_router.h"

using namespace transport_router;
using namespace graph;
using namespace transport_catalogue;

// операторы структуры RouteWeight
bool RouteWeight::operator<(const RouteWeight& other) const {
    return this->total_time < other.total_time;
}
RouteWeight RouteWeight::operator+(const RouteWeight& other) const {
    RouteWeight result;
    result.total_time = this->total_time + other.total_time;
    return result;
}
bool RouteWeight::operator>(const RouteWeight& other) const {
    return !(*this < other);
}

TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue,
    const RouterSettings& route_settings)
    : transport_catalogue_(transport_catalogue)
    , route_settings_(route_settings)
{
    DirectedWeightedGraph<RouteWeight> graph(SetStopsGetCount());
    graph_ = std::move(graph);
    BuildEdges();
    router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
}

// построение маршрута
std::optional<std::vector<RouterEdge>> TransportRouter::BuildRoute(const std::string& from, const std::string& to) {
    if (from == to) {
        return std::vector<RouterEdge>{};
    }

    size_t from_id = id_by_stop_name_.at(from);
    size_t to_id = id_by_stop_name_.at(to);
    std::optional<Router<RouteWeight>::RouteInfo> route = router_->BuildRoute(from_id, to_id);

    if (!route) {
        return std::nullopt;
    }

    std::vector<RouterEdge> result;
    for (EdgeId edge_id : route->edges) {
        const Edge<RouteWeight>& edge = graph_.GetEdge(edge_id);
        RouterEdge route_edge;
        route_edge.bus_name = edge.weight.bus_name;
        route_edge.stop_name_from = stops_by_id_.at(edge.from)->name_;
        route_edge.stop_name_to = stops_by_id_.at(edge.to)->name_;
        route_edge.stop_count = edge.weight.stop_count;
        route_edge.total_time = edge.weight.total_time;
        result.push_back(route_edge);
    }
    return result;
}

const RouterSettings& TransportRouter::GetSettings() const {
    return route_settings_;
}
RouterSettings& TransportRouter::GetSettings() {
    return route_settings_;
}

// заполнение полей остановками и возвращение их количества
size_t TransportRouter::SetStopsGetCount() {
    size_t stops_counter = 0;
    const std::deque<const Stop*>& stops = transport_catalogue_.GetStops();
    id_by_stop_name_.reserve(stops.size());
    stops_by_id_.reserve(stops.size());
    for (const Stop* stop : stops) {
        id_by_stop_name_.insert({ stop->name_, stops_counter });
        stops_by_id_.insert({ stops_counter, stop });
        ++stops_counter;
    }
    return stops_counter;
}

// построение граней графа
void TransportRouter::BuildEdges() {
    const int8_t there = -1, back = 1;
    for (const Bus* bus : transport_catalogue_.GetBuses()) {
        size_t stops_count = bus->stops_of_bus_.size();
        for (size_t i = 0; i < stops_count - 1; ++i) {
            double time_there, time_back;
            time_there = time_back = route_settings_.bus_wait_time;
            for (size_t j = i + 1; j < stops_count; ++j) {
                AddEdge(bus, there, i, j, time_there);
                if (bus->loop_ == RouteType::NOT_LOOPED) {
                    AddEdge(bus, back, stops_count - 1 - i, stops_count - 1 - j, time_back);
                }
            }
        }
    }
}
// добавление грани в граф
void TransportRouter::AddEdge(const Bus* bus, const int8_t direction_factor, const size_t stop, const size_t stop_next, double& total_time) {
    Edge<RouteWeight> edge = MakeEdge(bus, stop, stop_next);
    total_time += ComputeRouteTime(bus, stop_next + direction_factor, stop_next);
    edge.weight.total_time = total_time;
    graph_.AddEdge(edge);
}
// создание грани
Edge<RouteWeight> TransportRouter::MakeEdge(const Bus* bus, const size_t stop_id_from, const size_t stop_id_to) {
    Edge<RouteWeight> edge;
    edge.from = id_by_stop_name_.at(bus->stops_of_bus_.at(stop_id_from)->name_);
    edge.to = id_by_stop_name_.at(bus->stops_of_bus_.at(stop_id_to)->name_);
    edge.weight.bus_name = bus->name_;
    edge.weight.stop_count = stop_id_to - stop_id_from;
    return edge;
}
// вычисление времени поездки
double TransportRouter::ComputeRouteTime(const Bus* bus, const size_t stop_id_from, const size_t stop_id_to) {
    double distance = transport_catalogue_.GetDistanceBetweenStops(
        bus->stops_of_bus_.at(stop_id_from), bus->stops_of_bus_.at(stop_id_to)
    );
    return distance / route_settings_.bus_velocity;
}