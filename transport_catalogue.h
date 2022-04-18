#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <deque>
#include <unordered_map>

namespace transport_catalogue {
	using namespace geo;
	using namespace std::literals;

	// список, отображающий зацикленность маршрута
	enum class RouteType {
		IS_LOOPED, // маршрут круговой
		NOT_LOOPED // маршрут прямой
	};

	// структура остановки: имя(по стандарту "Error") - ширина - долгота
	struct Stop {
		Stop(std::string name, double latitude, double longitude)
			: name_(name)
		{
			coordinates.lat = latitude;
			coordinates.lng = longitude;
		}

		Stop(std::string name = "Error"s)
			: name_(name)
		{
		}
		// для поиска остановки
		bool operator==(const Stop& other) const {
			return this->name_ == other.name_;
		}

		std::string name_;
		Coordinates coordinates;
	};

	// структура маршрута: номер автобуса(по стандарту "Error") - остановки - тип(прямой/кольцевой)
	struct Bus {
		Bus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop)
			: name_(name), stops_of_bus_(stops_of_bus), loop_(loop)
		{
		}

		Bus(std::string name = "Error"s)
			: name_(name)
		{
		}
		// для поиска маршрута
		bool operator==(const Bus& other) const {
			return this->name_ == other.name_;
		}

		std::string name_;
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

	// класс каталога маршрутов
	class TransportCatalogue {
	public:
		// хешер пары остановок
		struct StopsHasher {
			size_t operator()(const std::pair<const Stop*, const Stop*>&two_stops) const {
				size_t h_1 = hasher_(two_stops.first);
				size_t h_2 = hasher_(two_stops.second);
				return h_2 * 42 + h_1 * (42 * 42);
			}
		private:
			std::hash<const void*> hasher_;
		};

		// добавление остановки
		void AddStop(std::string name, Coordinates point);
		// добавление маршрута
		void AddBus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop);

		// поиск остановки по имени
		const Stop& FindStop(const std::string& name);
		// поиск маршрута по номеру
		const Bus& FindBus(const std::string& name);
		// поиск информации о маршруте по номеру
		BusInfo GetBusInfo(const std::string& name);

		// получение всех маршрутов с их остановками
		std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesToStops();
		// получение информации о дистанции между остановками
		double GetDistanceBetweenStops(std::string stop_name, std::string next_stop_name);
		double GetDistanceBetweenStops(const Stop* stop, const Stop* next_stop);

		// получение доступа к маршрутам
		const std::deque<const Bus*> GetBuses() const;
		// получение координат каждой остановки из маршрутов
		const std::vector<Coordinates> GetAllStopsCoordinates() const;

		// заполнение информации о дистанции между остановками
		void SetDistanceBetweenStops(std::string stop_name, std::string next_stop_name, double distance);

	private:
		// хранилища:
		// остановок
		std::deque<Stop> stops_;
		// маршрутов
		std::deque<Bus> buses_;
		// маршрутов с их остановками
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops_;
		// информации о дистанции между остановками из маршрутов
		std::unordered_map<const std::pair<const Stop*, const Stop*>, double, StopsHasher> stop_pair_to_distance_;

		// подсчет коэффициента для работы с маршрутом
		double GetBusLoopCoeff(Bus& bus);

		// подсчет кол-ва остановок
		size_t GetBusLoopStopNum(Bus& bus);

		// подсчет количества уникальных остановок
		size_t GetBusInfoUniqueStops(std::vector<const Stop*> stops_of_bus);

		// подсчет длины маршурута 
		std::pair<double, double> GetBusInfoLoopDistance(Bus& bus, double& looped_coeff);
	};
} // конец пространства имен transport_catalogue