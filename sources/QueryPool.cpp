#include "..\headers\QueryPool.h"
#include "../headers/Query.h"
#include "../headers/vulkan_utils.h"
vkn::QueryPool::QueryPool(vkn::Device& device, const VkQueryType type, const uint32_t queryCount, const VkQueryPipelineStatisticFlags pipelineStatisticFlags): device_{device}, queryCount_{queryCount}
{
	VkQueryPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.queryType = type;
	poolInfo.queryCount = queryCount_;
	poolInfo.pipelineStatistics = pipelineStatisticFlags;
	vkn::error_check(vkCreateQueryPool(device_.device, &poolInfo, nullptr, &pool_), "Failed to create the query pool"); 
	results_.resize(queryCount_);
}
vkn::QueryPool::QueryPool(QueryPool&& other): device_{other.device_}
{
	queryCount_ = other.queryCount_;
	queryIndex_ = other.queryIndex_;
	pool_ = other.pool_;
	other.pool_ = VK_NULL_HANDLE;
}
vkn::QueryPool::~QueryPool()
{
	if (pool_ != VK_NULL_HANDLE)
	{
		vkDestroyQueryPool(device_.device, pool_, nullptr);
	}
}

vkn::Query vkn::QueryPool::getQuery()
{
	++queryIndex_;
	assert(queryIndex_ <= queryCount_ && "The pool reached its maximum query count");
	return Query{ *this, queryIndex_ };
}

VkQueryPool vkn::QueryPool::pool() const
{
	return pool_;
}

void vkn::QueryPool::reset(CommandBuffer& cb)
{
	vkCmdResetQueryPool(cb.commandBuffer(), pool_, 0, queryCount_);
}

const uint64_t vkn::QueryPool::getResult(Query& query, const float timestampPeriod)
{
	assert(timestampPeriod != 0 && "the gpu doesn't support timestamp");
	return getResult(query.queryIndex_) * static_cast<uint64_t>(timestampPeriod);
}

const uint64_t vkn::QueryPool::getResult(const uint32_t queryIndex, const VkQueryResultFlags resultFlag) const
{
	assert(std::size(results_) > queryIndex);
	return results_[queryIndex];
}

const std::vector<uint64_t> vkn::QueryPool::getResults(const VkQueryResultFlags resultFlag)
{
	vkGetQueryPoolResults(device_.device, pool_, 0, queryCount_, std::size(results_) * sizeof(uint64_t), std::data(results_), sizeof(uint64_t), resultFlag);
	return results_;
}
