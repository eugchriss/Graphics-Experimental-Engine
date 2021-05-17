#pragma once
#include <chrono>
#include <string>

namespace gee
{
	class Timer
	{
	public:
		Timer(const std::string& name);
		const std::string& name() const;
		void reset();
		const float ellapsedMs() const;
		const long long ellapsedSec() const;
	private:
		std::string name_;
		std::chrono::system_clock::time_point referenceTime_{};
	};
}
