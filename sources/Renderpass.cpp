#include "../headers/Renderpass.h"

void gee::Renderpass::add_pass(const gee::Pass& pass)
{
	passes_.emplace_back(pass);
}
