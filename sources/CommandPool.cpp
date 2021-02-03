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

std::vector<vkn::CommandBuffer> vkn::CommandPool::getCommandBuffers(const VkCommandBufferLevel level, const uint32_t count)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = pool_;
	allocInfo.level = level;
	allocInfo.commandBufferCount = count;

	std::vector<VkCommandBuffer> cbs(count);
	vkn::error_check(vkAllocateCommandBuffers(context_.device->device, &allocInfo, std::data(cbs)), "Unable to allocate command buffer");
	std::copy(std::begin(cbs), std::end(cbs), std::back_inserter(cbs_));
	
	std::vector<vkn::CommandBuffer> buffers;
	buffers.reserve(count);
	for (const auto& cb : cbs)
	{
		buffers.emplace_back(context_, cb);
	}
	return std::move(buffers);
}

vkn::CommandBuffer vkn::CommandPool::getCommandBuffer(const VkCommandBufferLevel level)
{
	auto& cbs = getCommandBuffers(level, 1);
	return std::move(cbs[0]);
}