#pragma once
#include "QueryPool.h"
#include "CommandBuffer.h"

namespace vkn
{
	class Query
	{
	public:
		friend QueryPool;
		void writeTimeStamp(vkn::CommandBuffer& cb, const VkPipelineStageFlagBits stage);
	private:
		Query(QueryPool& pool, const uint32_t queryIndex);
		QueryPool& pool_;
		uint32_t queryIndex_;
	};
}