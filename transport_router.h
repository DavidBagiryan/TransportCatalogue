#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport_router {
	struct RouterSettings {
		size_t bus_wait_time = 1; // время ожидания автобуса на остановке, в минутах (целое число от 1 до 1000)
		float bus_velocity = 1.;  // скорость автобуса, в км/ч (вещественное число от 1 до 1000)
	};

	struct RouteWeight {
		std::string_view bus_name;
		double total_time = 0;
		int stop_count = 0;

		bool operator<(const RouteWeight& other) const;
		bool operator>(const RouteWeight& other) const;
		RouteWeight operator+(const RouteWeight& other) const;
	};

	struct RouterEdge {
		std::string_view bus_name;
		std::string_view stop_name_from;
		std::string_view stop_name_to;
		double total_time = 0;
		int stop_count = 0;
	};

	class TransportRouter {
	public:
		TransportRouter(const transport_catalogue::TransportCatalogue& transport_catalogue_, const RouterSettings& route_settings);

		std::optional<std::vector<RouterEdge>> BuildRoute(const std::string& from, const std::string& to);

		const RouterSettings& GetSettings() const;
		RouterSettings& GetSettings();

	private:
		const transport_catalogue::TransportCatalogue& transport_catalogue_;
		RouterSettings route_settings_;
		std::unordered_map<size_t, const domain::Stop*> stops_by_id_;
		std::unordered_map<std::string_view, size_t> id_by_stop_name_;
		graph::DirectedWeightedGraph<RouteWeight> graph_;
		std::unique_ptr<graph::Router<RouteWeight>> router_;

		size_t SetStopsGetCount();

		void BuildEdges();
		void AddEdge(const domain::Bus* bus, const int8_t direction_factor, const size_t stop, const size_t stop_next, double& total_time);
		graph::Edge<RouteWeight> MakeEdge(const domain::Bus* bus, const size_t stop_id_from, const size_t stop_id_to);
		double ComputeRouteTime(const domain::Bus* bus, const size_t stop_id_from, const size_t stop_id_to);

	};
} // namespace transport_router