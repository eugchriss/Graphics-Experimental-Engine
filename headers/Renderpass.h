#pragma once
#include <vector>

#include "Pass.h"
namespace gee
{
	class Renderpass
	{
	public:
		Renderpass() = default;
		void add_pass(const Pass& pass);
	private:
		std::vector<Pass> passes_;
	};

}
