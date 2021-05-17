#include <cmath>

#include "../headers/units.h"

float gee::units::length::operator""_m(long double x)
{
	return x;
}

float gee::units::length::operator""_mm(long double x)
{
	return x / 1000.0f;
}

float gee::units::length::operator""_km(long double x)
{
	return x * 1000.0f;
}

float gee::units::angle::operator""_deg(long double x)
{
	return x * std::acos(-1) / 180.0f;
}

float gee::units::angle::operator""_rad(long double x)
{
	return x;
}

uint64_t gee::units::datas::operator""_Go(uint64_t x)
{
	return x * 1000000000;
}

uint64_t gee::units::datas::operator""_Mo(uint64_t x)
{
	return x * 1000000;
}

uint64_t gee::units::datas::operator""_Ko(uint64_t x)
{
	return x * 1000;
}