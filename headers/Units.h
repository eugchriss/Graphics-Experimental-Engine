#pragma once
#include <cstdint>

namespace gee
{
	namespace units
	{
		namespace length
		{
			float operator"" _m(long double x);
			float operator"" _mm(long double x);
			float operator"" _km(long double x);
		}
		namespace angle
		{
			float operator"" _deg(long double x);
			float operator"" _rad(long double x);
		}
		namespace datas
		{
			uint64_t operator"" _Go(uint64_t x);
			uint64_t operator"" _Mo(uint64_t x);
			uint64_t operator"" _Ko(uint64_t x);
		}
	}
}