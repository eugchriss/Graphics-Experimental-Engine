#pragma once
#include "vulkanContext.h"
#include "CommandBuffer.h"
#include <vector>
#include <stack>

namespace vkn
{
	class Query;
	class QueryPool
	{
	public:
		QueryPool(vkn::Context& context, const VkQueryType type, const uint32_t queryCount, const VkQueryPipelineStatisticFlags pipelineStatisticFlags = 0);
		QueryPool(QueryPool&& other);
		~QueryPool();
		Query getQuery();
		Query getQuery(CommandBuffer& cb);
		void deleteQuery(const uint32_t queryIndex);
		VkQueryPool pool() const;
		void reset(CommandBuffer& cb);
		void reset(const uint32_t queryIndex);
		const uint64_t results(const uint32_t queryIndex, const VkQueryResultFlags resultFlag) const;

	private:
		Context& context_;
		VkQueryPool pool_{ VK_NULL_HANDLE };
		uint32_t queryCount_{};
		uint32_t queryIndex_{-1u};
		std::stack<uint32_t> reusableQueryIndexes_;
		const uint32_t newQueryIndex();
	};
}
