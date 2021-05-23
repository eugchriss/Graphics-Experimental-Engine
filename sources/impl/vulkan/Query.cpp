#include "../../headers/impl/vulkan/Query.h"

using namespace gee;
vkn::Query::Query(vkn::QueryPool& pool, const uint32_t queryIndex) : pool_{ pool }, queryIndex_ { queryIndex }
{
}

vkn::Query::~Query()
{
	if (shouldDelete_)
	{
		pool_.deleteQuery(queryIndex_);
	}
}

vkn::Query::Query(Query& other): pool_{other.pool_}
{
	queryIndex_ = other.queryIndex_;
	other.shouldDelete_ = false;
}

void vkn::Query::writeTimeStamp(vkn::CommandBuffer& cb, const VkPipelineStageFlagBits stageFlag)
{
	vkCmdWriteTimestamp(cb.commandBuffer(), stageFlag, pool_.pool(), queryIndex_);
}

void vkn::Query::begin(CommandBuffer& cb)
{
	vkCmdBeginQuery(cb.commandBuffer(), pool_.pool(), queryIndex_, 0);
}

void vkn::Query::end(CommandBuffer& cb)
{
	vkCmdEndQuery(cb.commandBuffer(), pool_.pool(), queryIndex_);
}

void vkn::Query::reset()
{
	pool_.reset(queryIndex_);
}

uint64_t vkn::Query::results(const VkQueryResultFlags resultFlag) const
{
	return pool_.results(queryIndex_, resultFlag);
}
