#include "../headers/Renderpass.h"

bool gee::Renderpass::alreadyExistingGuiRenderpass{ false };

void gee::Renderpass::add_pass(const gee::Pass& pass)
{
	const_cast<Pass&>(pass).set_index(std::size(passes_));
	passes_.emplace_back(pass);
}

void gee::Renderpass::add_gui_pass()
{
	if (!alreadyExistingGuiRenderpass)
	{
		usesGui_ = true;
	}
	else
	{
		throw std::runtime_error{ "Should only be 1 renderpass using gui for the whole program" };
	}
}

bool gee::Renderpass::has_gui_pass() const
{
	return usesGui_;
}
