#include "transport_catalogue.h"

#include <algorithm>

using namespace transport_catalogue;

// добавление остановки
void TransportCatalogue::AddStop(std::string name, Coordinates point) {
	Stop stop(name, point.lat, point.lng);
	stops_.push_back(std::move(stop));
	buses_to_stops_[stop.name_];
}

// добавление маршрута
void TransportCatalogue::AddBus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop) {
	Bus bus(name, stops_of_bus, loop);
	buses_.push_back(std::move(bus));
	const Bus* from_buses_ = &FindBus(name);
	for (const Stop* stop : from_buses_->stops_of_bus_) {
		buses_to_stops_[stop->name_].insert(from_buses_->name_);
	}
}

// поиск остановки по имени
const Stop& TransportCatalogue::FindStop(const std::string& name) {
	for (const Stop& stop : stops_) {
		if (stop.name_ == name) {
			return stop;
		}
	}
	static Stop stop("Error"s); // стандартное имя остановки
	return stop;
}

// поиск маршрута по номеру
const Bus& TransportCatalogue::FindBus(const std::string& name) {
	for (const Bus& bus : buses_) {
		if (bus.name_ == name) {
			return bus;
		}
	}
	static Bus bus("Error"s); // стандартный номер маршрута
	return bus;
}

// поиск информации о маршруте по номеру
BusInfo TransportCatalogue::GetBusInfo(const std::string& name) {
	// хранилище информации о маршруте
	BusInfo info;
	// находим общую информацию о маршруте (номер, остановки, тип[прямой/кольцевой])
	Bus bus = FindBus(name);
	// подсчет количества уникальных остановок
	info.unique_stops_num_ = GetBusInfoUniqueStops(bus.stops_of_bus_);
	// введем коэффициент для работы с маршрутом
	double looped_coeff = GetBusLoopCoeff(bus);
	// подсчет кол-ва остановок
	info.stops_num_ = GetBusLoopStopNum(bus);
	// подсчет длины маршурута 
	std::pair<double, double> direct_distance_real_distance = GetBusInfoLoopDistance(bus, looped_coeff);
	// заполняем прямую длину маршрута
	info.distance_length_ = direct_distance_real_distance.first;
	// заполняем фактическую длину маршрута
	info.real_distance_length_ = direct_distance_real_distance.second;
	// заполняем коэффициент извилистости
	info.curvature_ = direct_distance_real_distance.second / direct_distance_real_distance.first;
	return info;
}

// подсчет коэффициента для работы с маршрутом
double TransportCatalogue::GetBusLoopCoeff(Bus& bus) {
	double looped_coeff;
	if (bus.loop_ == transport_catalogue::RouteType::IS_LOOPED) {
		looped_coeff = GetDistanceBetweenStops(bus.stops_of_bus_[bus.stops_of_bus_.size() - 1], bus.stops_of_bus_[0]);
	}
	else {
		looped_coeff = 0;
	}
	return looped_coeff;
}

// подсчет кол-ва остановок
size_t TransportCatalogue::GetBusLoopStopNum(Bus& bus) {
	size_t stops_num;
	if (bus.loop_ == transport_catalogue::RouteType::IS_LOOPED) {
		stops_num = bus.stops_of_bus_.size();
	}
	else {
		stops_num = (bus.stops_of_bus_.size() * 2) - 1;
	}
	return stops_num;
}

// подсчет длины маршурута 
std::pair<double, double> TransportCatalogue::GetBusInfoLoopDistance(Bus& bus, double& looped_coeff) {
	// хранилище длины маршрута
	double distance = 0;
	// хранилище фактической длины маршрута
	double real_distance = 0;
	// stop_it - итератор на остановку, i - порядковый номер остановки 
	for (auto [stop_it, i] = std::tuple(bus.stops_of_bus_.begin(), 0); ; ++i) {
		if (i == bus.stops_of_bus_.size() - 1) {
			real_distance += looped_coeff;
			break;
		}
		real_distance += GetDistanceBetweenStops(bus.stops_of_bus_[i], bus.stops_of_bus_[i + 1]);
		Stop from = **stop_it;
		++stop_it;
		if (stop_it == bus.stops_of_bus_.end()) {
			break;
		}
		Stop to = **stop_it;
		distance += ComputeDistance(from.coordinates, to.coordinates);
		if (bus.loop_ == transport_catalogue::RouteType::IS_LOOPED) {
			// заполняем кольцо маршрута
			if (stop_it == bus.stops_of_bus_.end() - 1) {
				Stop loop_end = **bus.stops_of_bus_.begin();
				distance += ComputeDistance(to.coordinates, loop_end.coordinates);
			}
		}
	}
	if (bus.loop_ == transport_catalogue::RouteType::NOT_LOOPED) {
		distance *= 2.;
		for (size_t i = bus.stops_of_bus_.size() - 1; i != 0; --i) {
			real_distance += GetDistanceBetweenStops(bus.stops_of_bus_[i], bus.stops_of_bus_[i - 1]);
		}
	}
	return { distance, real_distance };
}

// подсчет количества уникальных остановок
size_t TransportCatalogue::GetBusInfoUniqueStops(std::vector<const Stop*> stops_of_bus) {
	size_t unique_stops;
	std::sort(stops_of_bus.begin(), stops_of_bus.end());
	stops_of_bus.erase(unique(stops_of_bus.begin(), stops_of_bus.end()), stops_of_bus.end());
	unique_stops = stops_of_bus.size();
	return unique_stops;
}

// получение всех маршрутов с их остановками
std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetBusesToStops() {
	return buses_to_stops_;
}

// получение информации о дистанции между остановками
double TransportCatalogue::GetDistanceBetweenStops(std::string stop_name, std::string next_stop_name) {
	return GetDistanceBetweenStops(&FindStop(stop_name), &FindStop(next_stop_name));
}
double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop, const Stop* next_stop) {
	auto stops = std::make_pair(stop, next_stop);
	double result = 0;
	if (stop_pair_to_distance_.count(stops)) {
		result = stop_pair_to_distance_.at(stops);
	}
	else {
		try {
			result = stop_pair_to_distance_.at(std::make_pair(next_stop, stop));
		}
		catch (...) {}
	}
	return result;
}

// получение доступа к маршрутам
const std::deque<const Bus*> TransportCatalogue::GetBuses() const {
	std::deque<const Bus*> buses;
	for (const auto& bus : buses_) {
		buses.push_back(&bus);
	}

	std::sort(buses.begin(), buses.end(),
		[](const Bus* lhs, const Bus* rhs) {
			return lhs->name_ < rhs->name_;
		});

	return buses;
}

// получение координат каждой остановки из маршрутов
const std::vector<Coordinates> TransportCatalogue::GetAllStopsCoordinates() const {
	std::vector<Coordinates> stops_coordinates;
	for (const auto& bus : buses_) {
		for (const auto& stop : bus.stops_of_bus_) {
			stops_coordinates.push_back(stop->coordinates);
		}
	}
	return stops_coordinates;
}

// заполнение информации о дистанции между остановками
void TransportCatalogue::SetDistanceBetweenStops(std::string stop_name, std::string next_stop_name, double distance) {
	const Stop* stop = &FindStop(stop_name);
	const Stop* next_stop = &FindStop(next_stop_name);
	stop_pair_to_distance_[std::make_pair(stop, next_stop)] = distance;
}