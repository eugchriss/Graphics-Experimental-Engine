#pragma once
#include <chrono>

namespace gee
{
	class Timer
	{
	public:
		Timer();
		void set_timestamp();
		void reset();
		const double ellapsed_time_ms() const;
		const double ellapsed_time_s() const;
		const double absolute_time_ms() const;
		const double absolute_time_s() const;
	private:
		std::chrono::system_clock::time_point referenceTime_{};
		std::chrono::system_clock::time_point timestampTime_{};
	};
}
