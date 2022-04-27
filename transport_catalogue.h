#pragma once

#include "domain.h"

#include <set>
#include <deque>
#include <unordered_map>

namespace transport_catalogue {
	using namespace domain;

	// класс каталога маршрутов
	class TransportCatalogue {
	public:
		// хешер пары остановок
		struct StopsHasher {
			size_t operator()(const std::pair<const Stop*, const Stop*>& two_stops) const;
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