#pragma once

#include "domain.h"

#include <set>
#include <deque>
#include <unordered_map>

namespace transport_catalogue {
	using namespace domain;

	// ����� �������� ���������
	class TransportCatalogue {
	public:
		// ����� ���� ���������
		struct StopsHasher {
			size_t operator()(const std::pair<const Stop*, const Stop*>& two_stops) const;
		private:
			std::hash<const void*> hasher_;
		};

		// ���������� ���������
		void AddStop(std::string name, Coordinates point);
		// ���������� ��������
		void AddBus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop);

		// ����� ��������� �� �����
		const Stop& FindStop(const std::string& name);
		// ����� �������� �� ������
		const Bus& FindBus(const std::string& name);
		// ����� ���������� � �������� �� ������
		BusInfo GetBusInfo(const std::string& name);

		// ��������� ���� ��������� � �� �����������
		std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesToStops();
		// ��������� ���������� � ��������� ����� �����������
		double GetDistanceBetweenStops(std::string stop_name, std::string next_stop_name);
		double GetDistanceBetweenStops(const Stop* stop, const Stop* next_stop);

		// ��������� ������� � ���������
		const std::deque<const Bus*> GetBuses() const;
		// ��������� ��������� ������ ��������� �� ���������
		const std::vector<Coordinates> GetAllStopsCoordinates() const;

		// ���������� ���������� � ��������� ����� �����������
		void SetDistanceBetweenStops(std::string stop_name, std::string next_stop_name, double distance);

	private:
		// ���������:
		// ���������
		std::deque<Stop> stops_;
		// ���������
		std::deque<Bus> buses_;
		// ��������� � �� �����������
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stops_;
		// ���������� � ��������� ����� ����������� �� ���������
		std::unordered_map<const std::pair<const Stop*, const Stop*>, double, StopsHasher> stop_pair_to_distance_;

		// ������� ������������ ��� ������ � ���������
		double GetBusLoopCoeff(Bus& bus);

		// ������� ���-�� ���������
		size_t GetBusLoopStopNum(Bus& bus);

		// ������� ���������� ���������� ���������
		size_t GetBusInfoUniqueStops(std::vector<const Stop*> stops_of_bus);

		// ������� ����� ��������� 
		std::pair<double, double> GetBusInfoLoopDistance(Bus& bus, double& looped_coeff);
	};
} // ����� ������������ ���� transport_catalogue