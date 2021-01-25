#include "../headers/CommandPool.h"
#include "../headers/vulkan_utils.h"

vkn::CommandPool::CommandPool(Context& _context, const VkCommandPoolCreateFlagBits type) : context_{ _context }
{
	VkCommandPoolCreateInfo commandPoolCI{};

	commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCI.pNext = nullptr;
	commandPoolCI.flags = type;
	commandPoolCI.queueFamilyIndex = context_.queueFamily->familyIndex();

	vkn::error_check(vkCreateCommandPool(context_.device->device, &commandPoolCI, nullptr, &pool_), "Unable to create the command buffer pool");
}

vkn::CommandPool::CommandPool(CommandPool&& other): context_{other.context_}
{
	pool_ = other.pool_;
	cbs_ = std::move(other.cbs_);
	other.pool_ = VK_NULL_HANDLE;
}

vkn::CommandPool::~CommandPool()
{
	if (!std::empty(cbs_))
	{
		vkFreeCommandBuffers(context_.device->device, pool_, std::size(cbs_), std::data(cbs_));
	}
	if (pool_ != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(context_.device->device, pool_, nullptr);
	}
}

vkn::CommandBuffer vkn::CommandPool::getCommandBuffer(const VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = pool_;
	allocInfo.level = level;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cb{ VK_NULL_HANDLE };
	vkn::error_check(vkAllocateCommandBuffers(context_.device->device, &allocInfo, &cb), "Unable to allocate command buffer");
	cbs_.push_back(cb);
	return CommandBuffer{context_, cb };
}