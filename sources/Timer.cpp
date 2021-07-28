#include "../headers/Timer.h"

gee::Timer::Timer()
{
	reset();
}
void gee::Timer::set_timestamp()
{
	timestampTime_ = std::chrono::system_clock::now();
}

void gee::Timer::reset()
{
	referenceTime_ = std::chrono::system_clock::now();
	timestampTime_ = referenceTime_;
}

const double gee::Timer::ellapsed_time_ms() const
{
	std::chrono::duration<double, std::milli> ellapsed = std::chrono::system_clock::now() - timestampTime_;
	return ellapsed.count();
}

const double gee::Timer::ellapsed_time_s() const
{
	std::chrono::duration<double> ellapsed = std::chrono::system_clock::now() - timestampTime_;
	return ellapsed.count();
}

const double gee::Timer::absolute_time_ms() const
{
	std::chrono::duration<double, std::milli> ellapsed = std::chrono::system_clock::now() - referenceTime_;
	return ellapsed.count();
}

const double gee::Timer::absolute_time_s() const
{
	std::chrono::duration<double> ellapsed = std::chrono::system_clock::now() - referenceTime_;
	return ellapsed.count();
}