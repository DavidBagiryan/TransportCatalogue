#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace domain {
	using namespace geo;
	using namespace std::literals;

	// ������, ������������ ������������� ��������
	enum class RouteType {
		IS_LOOPED, // ������� ��������
		NOT_LOOPED // ������� ������
	};

	// ��������� ���������: ���(�� ��������� "Error") - ������ - �������
	struct Stop {
		Stop() = default;
		Stop(std::string name, Coordinates coordinates);

		// ��� ������ ���������
		bool operator==(const Stop& other) const;

		std::string name_ = "Error"s;
		Coordinates coordinates_;
	};

	// ��������� ��������: ����� ��������(�� ��������� "Error") - ��������� - ���(������/���������)
	struct Bus {
		Bus() = default;
		~Bus() = default;
		Bus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop);

		// ��� ������ ��������
		bool operator==(const Bus& other) const;

		std::string name_ = "Error"s;
		std::vector<const Stop*> stops_of_bus_;
		RouteType loop_;
	};

	// ��������� ���������� � ��������: ���-�� ��������� - ���-�� ����. ��������� - ������ ����� �������� - ����������� ����� �������� - ����������� ������������
	struct BusInfo {
		size_t stops_num_;
		size_t unique_stops_num_;
		int distance_length_;
		double real_distance_length_;
		double curvature_;
	};
}// ����� ������������ ���� domain