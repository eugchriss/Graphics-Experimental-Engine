#pragma once
#include "QueryPool.h"
#include "CommandBuffer.h"

namespace vkn
{
	class Query
	{
	public:
		friend QueryPool;
		~Query();
		Query(Query& other);
		void writeTimeStamp(vkn::CommandBuffer& cb, const VkPipelineStageFlagBits stage);
		void begin(CommandBuffer& cb);
		void end(CommandBuffer& cb);
		void reset();
		uint64_t results(const VkQueryResultFlags resultFlagendQuery = VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT) const;
	private:
		Query(QueryPool& pool, const uint32_t queryIndex);
		QueryPool& pool_;
		uint32_t queryIndex_;
		bool shouldDelete_{true};
	};
}