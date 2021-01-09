
#pragma once
#include "Device.h"
#include "CommandBuffer.h"
#include <vector>

namespace vkn
{
	class Query;
	class QueryPool
	{
	public:
		QueryPool(vkn::Device& device, const VkQueryType type, const uint32_t queryCount, const VkQueryPipelineStatisticFlags pipelineStatisticFlags = 0);
		QueryPool(QueryPool&& other);
		~QueryPool();
		Query getQuery();
		VkQueryPool pool() const;
		void reset(CommandBuffer& cb);
		const uint64_t getResult(Query& query, const float timestampPeriod);
		const uint64_t getResult(const uint32_t queryIndex, const VkQueryResultFlags resultFlag = VK_QUERY_RESULT_64_BIT) const;
		const std::vector<uint64_t> getResults(const VkQueryResultFlags resultFlag = VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
	private:
		vkn::Device& device_;
		VkQueryPool pool_{ VK_NULL_HANDLE };
		uint32_t queryCount_{};
		uint32_t queryIndex_{-1u};
		std::vector<uint64_t> results_{};
	};
}
