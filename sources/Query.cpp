#include "..\headers\Query.h"

vkn::Query::Query(vkn::QueryPool& pool, const uint32_t queryIndex) : pool_{ pool }, queryIndex_ { queryIndex }
{
}

void vkn::Query::writeTimeStamp(vkn::CommandBuffer& cb, const VkPipelineStageFlagBits stageFlag)
{
	vkCmdWriteTimestamp(cb.commandBuffer(), stageFlag, pool_.pool(), queryIndex_);
}