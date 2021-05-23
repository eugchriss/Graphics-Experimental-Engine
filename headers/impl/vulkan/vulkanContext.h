#pragma once
#include <memory>

#include "DebugMessenger.h"
#include "Device.h"
#include "Gpu.h"
#include "Instance.h"
#include "Queue.h"
#include "vulkan/vulkan.hpp"

#define MAKE_REFERENCE(object) using object##Ref = std::reference_wrapper<object>;
#define MAKE_CONST_REFERENCE(object) using object##ConstRef = std::reference_wrapper<const object>;
#define MAKE_UNIQUE_PTR(object) using object##Ptr = std::unique_ptr<object>;

namespace gee
{
	namespace vkn
	{
		class Context
		{
		public:
			Context(Context&&);
			Context(const Context&) = default;
			~Context();
			const bool pushDescriptorEnabled() const;
			std::shared_ptr<vkn::Instance> instance;
			std::unique_ptr<vkn::DebugMessenger> debugMessenger;
			VkSurfaceKHR surface{ VK_NULL_HANDLE };
			std::shared_ptr<vkn::Gpu> gpu;
			std::shared_ptr<vkn::QueueFamily> queueFamily;
			std::unique_ptr<vkn::Device> device;
			std::unique_ptr<vkn::Queue> graphicsQueue;
			std::unique_ptr<vkn::Queue> transferQueue;
		private:
			friend class ContextBuilder;
			Context(std::shared_ptr<vkn::Instance>& _instance, std::unique_ptr<vkn::DebugMessenger>& _debugMessenger, const VkSurfaceKHR _surface, std::shared_ptr<vkn::Gpu>& _gpu, std::shared_ptr<vkn::QueueFamily>& _queueFamily, std::unique_ptr<vkn::Device>& _device);
		};
	}
}