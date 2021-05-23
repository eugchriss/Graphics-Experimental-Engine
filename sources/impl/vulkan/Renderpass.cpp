#include <numeric>
#include <optional>
#include <unordered_map>

#include "../../headers/impl/vulkan/Renderpass.h"

using namespace gee;
vkn::Renderpass::Renderpass(Context& context, const VkExtent2D& extent, std::vector<Pass>&& passes, const uint32_t framebufferCount) : 
	context_{ context }, passes_{std::move(passes)}
{
	createRenderpass();
	createFramebuffers(framebufferCount, extent);
	auto& uniqueRenderTargets = getUniqueRenderTargets();
	bool presentClearInserted{ false };
	for (const auto& renderTargetRef : uniqueRenderTargets)
	{
		const auto& renderTarget = renderTargetRef.get();
		if (renderTarget.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && !presentClearInserted)
		{
			presentClearInserted = true;
			clearValues_.emplace_back(renderTarget.get_clear_value());
		}
		else if (renderTarget.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			clearValues_.emplace_back(renderTarget.get_clear_value());
		}
	}
}

void vkn::Renderpass::begin(vkn::CommandBuffer& cb, const VkRect2D& renderArea, const VkSubpassContents subpassContent)
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.framebuffer = framebuffers_[currentFramebuffer];
	beginInfo.renderPass = renderpass_;
	beginInfo.renderArea = renderArea;
	//There is a bug with the clear values: each present attachment clear value should be seperated
	beginInfo.clearValueCount = std::size(clearValues_);
	beginInfo.pClearValues = std::data(clearValues_);
	vkCmdBeginRenderPass(cb.commandBuffer(), &beginInfo, subpassContent);

	VkViewport viewport{};
	viewport.x = renderArea.offset.x;
	viewport.y = renderArea.offset.y;
	viewport.width = renderArea.extent.width;
	viewport.height = renderArea.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cb.commandBuffer(), 0, 1, &viewport);
	vkCmdSetScissor(cb.commandBuffer(), 0, 1, &renderArea);

	assert(std::size(passes_) > 0 && "The renderpass has 0 passes");
}

void vkn::Renderpass::end(vkn::CommandBuffer& cb)
{
	vkCmdEndRenderPass(cb.commandBuffer());
	++currentFramebuffer;
	currentFramebuffer %= std::size(framebuffers_);
}

std::vector<vkn::RenderTargetRef> vkn::Renderpass::getUniqueRenderTargets()
{
	std::vector<RenderTargetRef> uniqueTargets;
	std::vector<std::pair<uint64_t, RenderTargetRef>> targets;
	for (auto& pass : passes_)
	{
		for (const auto& target : pass.colorTargets())
		{
			targets.emplace_back(target.get().id(), std::ref(target));
		}
		for (const auto& target : pass.depthStencilTargets())
		{
			targets.emplace_back(target.get().id(), std::ref(target));
		}
		for (const auto& target : pass.inputTargets())
		{
			targets.emplace_back(target.get().id(), std::ref(target));
		}
	}
	for (const auto& [id, target] : targets)
	{
		uniqueTargets.emplace_back(std::ref(target));
	}
	return uniqueTargets;
}

void vkn::Renderpass::createRenderpass()
{
	auto& uniqueRenderTargets = getUniqueRenderTargets();
	auto& [attachments, renderTargetAttachmentMap] = getAttachments(uniqueRenderTargets);
	auto subpassesDatas = getSubpassesDatas(uniqueRenderTargets, renderTargetAttachmentMap, std::size(attachments));
	auto& dependencies = findDependencies(uniqueRenderTargets);

	std::vector<VkSubpassDescription> subpasses;
	for (const auto& subpassData : subpassesDatas)
	{
		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = std::size(subpassData.inputAttachments);
		subpass.pInputAttachments = std::data(subpassData.inputAttachments);
		subpass.colorAttachmentCount = std::size(subpassData.colorAttachments);
		subpass.pColorAttachments = std::data(subpassData.colorAttachments);
		subpass.pDepthStencilAttachment = std::data(subpassData.depthStencilAttachments);
		subpass.pResolveAttachments = nullptr;
		subpass.preserveAttachmentCount = std::size(subpassData.preservedAttachments);
		subpass.pPreserveAttachments = std::data(subpassData.preservedAttachments);

		subpasses.emplace_back(subpass);
	}

	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.attachmentCount = std::size(attachments);
	info.pAttachments = std::data(attachments);
	info.subpassCount = std::size(subpasses);
	info.pSubpasses = std::data(subpasses);
	info.dependencyCount = std::size(dependencies);
	info.pDependencies = std::data(dependencies);

	vkn::error_check(vkCreateRenderPass(context_.device->device, &info, nullptr, &renderpass_), "Failed to create the render pass");
}

const std::tuple<vkn::Renderpass::AttachmentDescriptions, vkn::Renderpass::RenderTargetAttachmentMap> vkn::Renderpass::getAttachments(const std::vector<RenderTargetRef>& targets) const
{
	std::vector<VkAttachmentDescription> attachments;
	std::unordered_map<size_t, size_t> renderTargetAttachmentMap;
	for (const auto& targetRef : targets)
	{
		const auto& target = targetRef.get();
		VkAttachmentDescription attachment{};
		attachment.flags = 0;
		attachment.format = target.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = target.finalLayout;
		attachment.loadOp = target.loadOperation;
		attachment.storeOp = target.storeOperation;
		attachment.stencilLoadOp = target.stencilLoadOperation;
		attachment.stencilStoreOp = target.stencilStoreOperation;
		if (attachment.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) //avoid duplicating swapchain images as attachment
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [](const auto& attachmentDesc) {return attachmentDesc.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; });
			if (result == std::end(attachments))
			{
				renderTargetAttachmentMap[target.id()] = std::size(attachments);
				attachments.emplace_back(attachment);
			}
			else
			{
				renderTargetAttachmentMap[target.id()] = std::distance(std::begin(attachments), result);
			}
		}
		else
		{
			renderTargetAttachmentMap[target.id()] = std::size(attachments);
			attachments.emplace_back(attachment);
		}
	}
	return { attachments, renderTargetAttachmentMap };
}

const std::vector<vkn::Renderpass::SubpassDatas> vkn::Renderpass::getSubpassesDatas(const std::vector<RenderTargetRef>& targets, const RenderTargetAttachmentMap& renderTargetAttachmentMap, const uint32_t attachmentCount)
{
	std::vector<vkn::Renderpass::SubpassDatas> subpassesDatas;
	for (auto& pass : passes_)
	{
		SubpassDatas subpassData{};
		auto& inputAttachments = subpassData.inputAttachments;
		auto& colorAttachments = subpassData.colorAttachments;
		auto& depthStencilAttachments = subpassData.depthStencilAttachments;
		auto& preservedAttachments = subpassData.preservedAttachments;

		preservedAttachments.resize(attachmentCount);
		std::iota(std::begin(preservedAttachments), std::end(preservedAttachments), 0);

		for (const auto& inputTarget : pass.inputTargets())
		{
			auto& attachmentIndex = renderTargetAttachmentMap.find(inputTarget.get().id());
			assert(attachmentIndex != std::end(renderTargetAttachmentMap) && "missing match between render target and renderpass attachment");

			VkAttachmentReference ref;
			ref.attachment = attachmentIndex->second;
			ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			inputAttachments.emplace_back(ref);
			auto it = std::remove(std::begin(preservedAttachments), std::end(preservedAttachments), ref.attachment);
			if(it != std::end(preservedAttachments))
				preservedAttachments.erase(it, std::end(preservedAttachments));
		}
		for (const auto& colorTarget : pass.colorTargets())
		{
			auto& attachmentIndex = renderTargetAttachmentMap.find(colorTarget.get().id());
			assert(attachmentIndex != std::end(renderTargetAttachmentMap) && "missing match between render target and renderpass attachment");

			VkAttachmentReference ref;
			ref.attachment = attachmentIndex->second;
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//Make sure there is only 1 present attachment used per pass
			if (colorTarget.get().finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			{
				auto presentExist = std::find_if(std::begin(colorAttachments), std::end(colorAttachments), [&](const auto& attachmentRef) {return targets[attachmentRef.attachment].get().finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; });
				if (presentExist == std::end(colorAttachments))
				{
					colorAttachments.emplace_back(ref);
				}
			}
			else
			{
				colorAttachments.emplace_back(ref);
			}

			auto it = std::remove(std::begin(preservedAttachments), std::end(preservedAttachments), ref.attachment);
			if (it != std::end(preservedAttachments))
				preservedAttachments.erase(it, std::end(preservedAttachments));
		}
		for (const auto& depthStencilTarget : pass.depthStencilTargets())
		{
			auto& attachmentIndex = renderTargetAttachmentMap.find(depthStencilTarget.get().id());
			assert(attachmentIndex != std::end(renderTargetAttachmentMap) && "missing match between render target and renderpass attachment");

			VkAttachmentReference ref;
			ref.attachment = attachmentIndex->second;
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthStencilAttachments.emplace_back(ref);
			auto it = std::remove(std::begin(preservedAttachments), std::end(preservedAttachments), ref.attachment);
			if (it != std::end(preservedAttachments))
				preservedAttachments.erase(it, std::end(preservedAttachments));
		}
		subpassesDatas.emplace_back(std::move(subpassData));
	}
	return subpassesDatas;
}

const std::vector<VkSubpassDependency> vkn::Renderpass::findDependencies(const std::vector<RenderTargetRef>& targets)
{
	std::vector<VkSubpassDependency> dependencies;

	return dependencies;
}

void vkn::Renderpass::createFramebuffers(const uint32_t framebufferCount, const VkExtent2D& extent)
{
	auto& renderTargets = getUniqueRenderTargets();

	std::vector<RenderTargetRef> presentableTargets;
	std::vector<RenderTargetRef> nonPresentableTargets;
	for (auto& target : renderTargets)
	{
		if (target.get().finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			presentableTargets.emplace_back(target);
		}
		else
		{
			nonPresentableTargets.emplace_back(target);
		}
	}

	std::optional<size_t> presentAttachmentIndex{ std::nullopt };
	const auto& [renderpassAttachments, renderTargetAttachmentMap] = getAttachments(renderTargets);
	auto result = std::find_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto attachment) {return attachment.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; });
	if (result != std::end(renderpassAttachments))
	{
		presentAttachmentIndex.emplace(std::distance(std::begin(renderpassAttachments), result));
	}
	
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = extent.width;
	fbInfo.height = extent.height;
	fbInfo.renderPass = renderpass_;

	uint32_t presentableTargetsIndex{};
	for (auto i = 0u; i < framebufferCount; ++i)
	{
		std::vector<VkImageView> views;
		for (auto& renderTargetRef : nonPresentableTargets)
		{
			views.emplace_back(renderTargetRef.get().view());
		}

		if (presentAttachmentIndex)
		{
			views.insert(std::begin(views) + *presentAttachmentIndex, presentableTargets[presentableTargetsIndex].get().view());
			++presentableTargetsIndex;
			presentableTargetsIndex %= std::size(presentableTargets);
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);
		VkFramebuffer fb{ VK_NULL_HANDLE };
		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &fb), "Failed to create the framebuffer");
		framebuffers_.emplace_back(fb);
	}
}

vkn::Renderpass::Renderpass(Renderpass&& other) : context_{ other.context_ }
{
	renderpass_ = other.renderpass_;
	passes_ = std::move(other.passes_);
	currentFramebuffer = other.currentFramebuffer;
	framebuffers_ = std::move(framebuffers_);
	clearValues_ = clearValues_;
	other.renderpass_ = VK_NULL_HANDLE;
}

vkn::Renderpass::~Renderpass()
{
	if (renderpass_ != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(context_.device->device, renderpass_, nullptr);
	}
	for (auto& fb : framebuffers_)
	{
		vkDestroyFramebuffer(context_.device->device, fb, nullptr);
	}
}

const VkRenderPass& vkn::Renderpass::operator()() const
{
	return renderpass_;
}

const uint32_t vkn::Renderpass::passesCount() const
{
	return static_cast<uint32_t>(std::size(passes_));
}

void vkn::Renderpass::resize(const glm::u32vec2& newSize)
{
	for (auto& fb : framebuffers_)
	{
		vkDestroyFramebuffer(context_.device->device, fb, nullptr);
	};
	const auto fbCount = std::size(framebuffers_);
	framebuffers_.clear();
	createFramebuffers(fbCount, VkExtent2D{ newSize.x, newSize.y });
}