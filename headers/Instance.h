#pragma once
#include <vector>
#include <string>
#include <vector>
#include "vulkan/vulkan.hpp"
namespace vkn
{
	struct Instance
	{
		Instance(const std::vector<const char*>& requestedLayers);
		~Instance();
		std::vector<const char*> availableLayers;
		VkInstance instance{ VK_NULL_HANDLE };

	private:
		void checkLayerPresence(const std::vector<const char*>& requested) const;
	};
}