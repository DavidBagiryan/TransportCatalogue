#include "transport_catalogue.h"

#include <algorithm>

using namespace transport_catalogue;

// ���������� ���������
void TransportCatalogue::AddStop(std::string name, Coordinates point) {
	stops_.emplace_back(name, point);
	buses_to_stops_[name];
}

size_t TransportCatalogue::StopsHasher::operator()(const std::pair<const Stop*, const Stop*>& two_stops) const {
	size_t h_1 = hasher_(two_stops.first);
	size_t h_2 = hasher_(two_stops.second);
	return h_2 * 42 + h_1 * (42 * 42);
}

// ���������� ��������
void TransportCatalogue::AddBus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop) {
	buses_.emplace_back(name, stops_of_bus, loop );
	const Bus* from_buses_ = &FindBus(name);
	for (const Stop* stop : from_buses_->stops_of_bus_) {
		buses_to_stops_[stop->name_].insert(from_buses_->name_);
	}
}

// ����� ��������� �� �����
const Stop& TransportCatalogue::FindStop(const std::string& name) {
	for (const Stop& stop : stops_) {
		if (stop.name_ == name) {
			return stop;
		}
	}
	static Stop stop; // ����������� ��� ���������
	return stop;
}

// ����� �������� �� ������
const Bus& TransportCatalogue::FindBus(const std::string& name) {
	for (const Bus& bus : buses_) {
		if (bus.name_ == name) {
			return bus;
		}
	}
	static Bus bus; // ����������� ����� ��������
	return bus;
}

// ����� ���������� � �������� �� ������
BusInfo TransportCatalogue::GetBusInfo(const std::string& name) {
	// ��������� ���������� � ��������
	BusInfo info;
	// ������� ����� ���������� � �������� (�����, ���������, ���[������/���������])
	Bus bus = FindBus(name);
	// ������� ���������� ���������� ���������
	info.unique_stops_num_ = GetBusInfoUniqueStops(bus.stops_of_bus_);
	// ������ ����������� ��� ������ � ���������
	double looped_coeff = GetBusLoopCoeff(bus);
	// ������� ���-�� ���������
	info.stops_num_ = GetBusLoopStopNum(bus);
	// ������� ����� ��������� 
	std::pair<double, double> direct_distance_real_distance = GetBusInfoLoopDistance(bus, looped_coeff);
	// ��������� ������ ����� ��������
	info.distance_length_ = direct_distance_real_distance.first;
	// ��������� ����������� ����� ��������
	info.real_distance_length_ = direct_distance_real_distance.second;
	// ��������� ����������� ������������
	info.curvature_ = direct_distance_real_distance.second / direct_distance_real_distance.first;
	return info;
}

// ������� ������������ ��� ������ � ���������
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

// ������� ���-�� ���������
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

// ������� ����� ��������� 
std::pair<double, double> TransportCatalogue::GetBusInfoLoopDistance(Bus& bus, double& looped_coeff) {
	// ��������� ����� ��������
	double distance = 0;
	// ��������� ����������� ����� ��������
	double real_distance = 0;
	// stop_it - �������� �� ���������, i - ���������� ����� ��������� 
	for (auto [stop_it, i] = std::tuple(bus.stops_of_bus_.begin(), 0); ; ++i) {
		if (i == int(bus.stops_of_bus_.size() - 1)) {
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
		distance += ComputeDistance(from.coordinates_, to.coordinates_);
		if (bus.loop_ == transport_catalogue::RouteType::IS_LOOPED) {
			// ��������� ������ ��������
			if (stop_it == bus.stops_of_bus_.end() - 1) {
				Stop loop_end = **bus.stops_of_bus_.begin();
				distance += ComputeDistance(to.coordinates_, loop_end.coordinates_);
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

// ������� ���������� ���������� ���������
size_t TransportCatalogue::GetBusInfoUniqueStops(std::vector<const Stop*> stops_of_bus) {
	size_t unique_stops;
	std::sort(stops_of_bus.begin(), stops_of_bus.end());
	stops_of_bus.erase(unique(stops_of_bus.begin(), stops_of_bus.end()), stops_of_bus.end());
	unique_stops = stops_of_bus.size();
	return unique_stops;
}

// ��������� ���� ��������� � �� �����������
std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::GetBusesToStops() {
	return buses_to_stops_;
}

// ��������� ���������� � ��������� ����� �����������
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

// ��������� ������� � ���������
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

// ��������� ��������� ������ ��������� �� ���������
const std::vector<Coordinates> TransportCatalogue::GetAllStopsCoordinates() const {
	std::vector<Coordinates> stops_coordinates;
	for (const auto& bus : buses_) {
		for (const auto& stop : bus.stops_of_bus_) {
			stops_coordinates.push_back(stop->coordinates_);
		}
	}
	return stops_coordinates;
}

// ���������� ���������� � ��������� ����� �����������
void TransportCatalogue::SetDistanceBetweenStops(std::string stop_name, std::string next_stop_name, double distance) {
	const Stop* stop = &FindStop(stop_name);
	const Stop* next_stop = &FindStop(next_stop_name);
	stop_pair_to_distance_[std::make_pair(stop, next_stop)] = distance;
}