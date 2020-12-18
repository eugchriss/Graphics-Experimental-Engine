#include "../headers/Timer.h"

gee::Timer::Timer(const std::string& name): name_{name}
{
	reset();
}

const std::string& gee::Timer::name() const
{
	return name_;
}

void gee::Timer::reset()
{
	referenceTime_ = std::chrono::system_clock::now();
}

const float gee::Timer::ellapsedMs() const
{
	auto now = std::chrono::system_clock::now();
	auto ellapsed = now - referenceTime_;
	return std::chrono::duration_cast<std::chrono::nanoseconds>(ellapsed).count() / 1000000.0f;
}

const long long gee::Timer::ellapsedSec() const
{
	auto now = std::chrono::system_clock::now();
	auto ellapsed = now - referenceTime_;
	return std::chrono::duration_cast<std::chrono::seconds>(ellapsed).count();
}
