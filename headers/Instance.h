#pragma once
#include <vector>
#include <string>
#include <initializer_list>
#include "vulkan/vulkan.hpp"
namespace vkn
{
	struct Instance
	{
		Instance(const std::initializer_list<const char*>& requestedLayers);
		~Instance();
		std::vector<const char*> availableLayers;
		VkInstance instance{ VK_NULL_HANDLE };

	private:
		void checkLayerPresence(const std::vector<const char*>& requested) const;
	};
}