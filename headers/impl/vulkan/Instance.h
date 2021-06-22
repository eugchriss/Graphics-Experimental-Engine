#pragma once
#include <vector>
#include <string>
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		struct Instance
		{
			Instance(const std::vector<std::string>& requestedLayers, const std::vector<std::string>& extensions = {});
			Instance(Instance&& other);
			~Instance();
			std::vector<const char*> availableLayers;
			VkInstance instance{ VK_NULL_HANDLE };

		private:
			void checkLayerPresence(const std::vector<const char*>& requested) const;
		};
	}
}