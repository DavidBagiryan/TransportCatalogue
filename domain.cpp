#include "domain.h"

using namespace domain;

Stop::Stop(std::string name, Coordinates coordinates)
	: name_(name)
	, coordinates_(coordinates)
{
}
Bus::Bus(std::string name, std::vector<const Stop*> stops_of_bus, RouteType loop)
	: name_(name), stops_of_bus_(stops_of_bus), loop_(loop)
{
}
// ��� ������ ���������
bool Stop::operator==(const Stop& other) const {
	return this->name_ == other.name_;
}
// ��� ������ ��������
bool Bus::operator==(const Bus& other) const {
	return this->name_ == other.name_;
}