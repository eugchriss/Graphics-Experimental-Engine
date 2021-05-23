#include "../../headers/impl/vulkan/QueryPool.h"
#include "../../headers/impl/vulkan/Query.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;
vkn::QueryPool::QueryPool(vkn::Context& context, const VkQueryType type, const uint32_t queryCount, const VkQueryPipelineStatisticFlags pipelineStatisticFlags) : context_{ context }, queryCount_{ queryCount }
{
	VkQueryPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.queryType = type;
	poolInfo.queryCount = queryCount_;
	poolInfo.pipelineStatistics = pipelineStatisticFlags;
	vkn::error_check(vkCreateQueryPool(context_.device->device, &poolInfo, nullptr, &pool_), "Failed to create the query pool"); 
}
vkn::QueryPool::QueryPool(QueryPool&& other): context_{other.context_}
{
	queryCount_ = other.queryCount_;
	queryIndex_ = other.queryIndex_;
	pool_ = other.pool_;
	reusableQueryIndexes_ = std::move(other.reusableQueryIndexes_);
	other.pool_ = VK_NULL_HANDLE;
}

vkn::QueryPool::~QueryPool()
{
	if (pool_ != VK_NULL_HANDLE)
	{
		vkDestroyQueryPool(context_.device->device, pool_, nullptr);
	}
}

vkn::Query vkn::QueryPool::getQuery()
{
	auto queryIndex = newQueryIndex();
	vkResetQueryPool(context_.device->device, pool_, queryIndex, 1);
	return Query{ *this, queryIndex};
}

vkn::Query vkn::QueryPool::getQuery(CommandBuffer& cb)
{
	auto queryIndex = newQueryIndex();
	vkCmdResetQueryPool(cb.commandBuffer(), pool_, queryIndex, 1);
	return Query{ *this, queryIndex };
}

void vkn::QueryPool::deleteQuery(const uint32_t queryIndex)
{
	reusableQueryIndexes_.push(queryIndex);
}

VkQueryPool vkn::QueryPool::pool() const
{
	return pool_;
}

void vkn::QueryPool::reset(CommandBuffer& cb)
{
	vkCmdResetQueryPool(cb.commandBuffer(), pool_, 0, queryCount_);
}

void vkn::QueryPool::reset(const uint32_t queryIndex)
{
	vkResetQueryPool(context_.device->device, pool_, queryIndex, 1);
}

const uint64_t vkn::QueryPool::results(const uint32_t queryIndex, const VkQueryResultFlags resultFlag) const
{
	std::vector<uint64_t> results(queryCount_);
	vkGetQueryPoolResults(context_.device->device, pool_, 0, queryCount_, std::size(results) * sizeof(uint64_t), std::data(results), sizeof(uint64_t), resultFlag);
	return results[queryIndex];
}

const uint32_t vkn::QueryPool::newQueryIndex()
{
	uint32_t queryIndex{};
	if (std::empty(reusableQueryIndexes_))
	{
		++queryIndex_;
		assert(queryIndex_ <= queryCount_ && "The pool reached its maximum query count");
		queryIndex = queryIndex_;
	}
	else
	{
		queryIndex = reusableQueryIndexes_.top();
		reusableQueryIndexes_.pop();
	}
	return  queryIndex;
}
