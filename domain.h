#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace domain {
	using namespace geo;
	using namespace std::literals;

	// список, отображающий зацикленность маршрута
	enum class RouteType {
		IS_LOOPED, // маршрут круговой
		NOT_LOOPED // маршрут прямой
	};

	// структура остановки: имя(по стандарту "Error") - ширина - долгота
	struct Stop {
		Stop() = default;
		Stop(std::string name, Coordinates coordinates);

		// для поиска остановки
		bool operator==(const Stop& other) const;

		std::string name_ = "Error"s;
		Coordinates coordinates_;
	};

	// структура маршрута: номер автобуса(по стандарту "Error") - остановки - тип(прямой/кольцевой)
	struct Bus {
		Bus() = default;
		~Bus() = default;
		Bus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop);

		// для поиска маршрута
		bool operator==(const Bus& other) const;

		std::string name_ = "Error"s;
		std::vector<const Stop*> stops_of_bus_;
		RouteType loop_;
	};

	// структура информации о маршруте: кол-во остановок - кол-во уник. остановок - прямая длина маршрута - фактическая длина маршрута - коэффициент извилистости
	struct BusInfo {
		size_t stops_num_;
		size_t unique_stops_num_;
		int distance_length_;
		double real_distance_length_;
		double curvature_;
	};
}// конец пространства имен domain