#include "../headers/Renderpass.h"

void gee::Renderpass::add_pass(const gee::Pass& pass)
{
	const_cast<Pass&>(pass).set_index(std::size(passes_));
	passes_.emplace_back(pass);
}
