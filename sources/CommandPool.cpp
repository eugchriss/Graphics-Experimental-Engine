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

	commandBuffers_[VK_COMMAND_BUFFER_LEVEL_PRIMARY];
	commandBuffers_[VK_COMMAND_BUFFER_LEVEL_SECONDARY];
	for (auto& [level, cbs] : commandBuffers_)
	{
		allocateCommandBuffer(level, 10);
	}
}

vkn::CommandPool::CommandPool(CommandPool&& other) : context_{ other.context_ }
{
	pool_ = other.pool_;
	commandBuffers_ = std::move(other.commandBuffers_);
	other.pool_ = VK_NULL_HANDLE;
}

vkn::CommandPool::~CommandPool()
{
	for (const auto& [level, cbs] : commandBuffers_)
	{
		std::vector<VkCommandBuffer> buffers;
		for (const auto& cb : cbs.commandBuffers)
		{
			buffers.emplace_back(cb.commandBuffer());
		}
		vkFreeCommandBuffers(context_.device->device, pool_, std::size(buffers), std::data(buffers));
	}

	if (pool_ != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(context_.device->device, pool_, nullptr);
	}
}

vkn::CommandBuffer& vkn::CommandPool::getCommandBuffer(const VkCommandBufferLevel level)
{
	sortCompletedCommandBuffers();
	auto& cbs = commandBuffers_[level];
	if (std::empty(cbs.availableCommandBuffers))
	{
		allocateCommandBuffer(level, 10);
	}
	else
	{
		auto& cb = cbs.availableCommandBuffers.front();
		cbs.availableCommandBuffers.pop();
		cbs.pendingCommandBuffers.emplace_back(std::ref(cb));
		return cb.get();
	}
}

void vkn::CommandPool::allocateCommandBuffer(const VkCommandBufferLevel level, const uint32_t count)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = pool_;
	allocInfo.level = level;
	allocInfo.commandBufferCount = count;
	
	std::vector<VkCommandBuffer> cbs(count);
	vkn::error_check(vkAllocateCommandBuffers(context_.device->device, &allocInfo, std::data(cbs)), "Unable to allocate command buffer");
	
	auto& levelCbs = commandBuffers_[level];
	levelCbs.commandBuffers.reserve(std::size(levelCbs.commandBuffers) + count);
	for (auto& cb: cbs)
	{
		levelCbs.commandBuffers.emplace_back(context_, cb);
		levelCbs.availableCommandBuffers.push(std::ref(levelCbs.commandBuffers.back()));
	}
}

void vkn::CommandPool::sortCompletedCommandBuffers()
{
	for (auto& [level, cbs] : commandBuffers_)
	{
		for (const auto& cbRef : cbs.pendingCommandBuffers)
		{
			if (!cbRef.get().isPending())
			{
				cbs.availableCommandBuffers.push(cbRef);
			}
		}
		std::remove_if(std::begin(cbs.pendingCommandBuffers), std::end(cbs.pendingCommandBuffers), [](const auto& cbRef) { return !cbRef.get().isPending(); });
	}
}
