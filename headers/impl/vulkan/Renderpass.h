#pragma once
#include <vector>

#include "Image.h"
#include "Pass.h"
#include "Swapchain.h"
#include "vulkanContext.h"
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"

#include "../../headers/Renderpass.h"
#include "../../headers/RenderTarget.h"
#include "../../headers/ResourceHolder.h"
namespace gee
{
	namespace vkn
	{
		struct SubpassAttachmentUsage
		{
			uint32_t subpassIndex{};
			VkPipelineStageFlags stageFlag{};
			VkAccessFlagBits accessFlag{};
		};

		class Renderpass
		{
		public:
			Renderpass(Context& context, const VkExtent2D& extent, std::vector<Pass>&& passes, bool usesGui = false, const uint32_t framebufferCount = 2);
			Renderpass(Renderpass&&);
			~Renderpass();
			void resize(const glm::u32vec2& newSize);
			void begin(vkn::CommandBuffer& cb, const VkRect2D& renderArea, const VkSubpassContents subpassContent = VK_SUBPASS_CONTENTS_INLINE);
			void end(vkn::CommandBuffer& cb);
			VkRenderPass get() const;
			void render_gui(vkn::CommandBuffer& cb);
			void next_pass(vkn::CommandBuffer& cb);
		private:
			Context& context_;
			VkRenderPass renderpass_{ VK_NULL_HANDLE };
			bool usesGui_;
			uint32_t currentFramebuffer{};
			uint32_t currentPass_{};
			std::vector<VkFramebuffer> framebuffers_;
			std::vector<Pass> passes_;
			std::vector<VkClearValue> clearValues_;
			std::vector<VkWriteDescriptorSet> shaderWrites_;
			VkDescriptorPool imGuiDescriptorPool_{ VK_NULL_HANDLE };

			std::vector<RenderTargetRef> getUniqueRenderTargets();
			void createRenderpass();
			struct SubpassDatas
			{
				std::vector<VkAttachmentReference> inputAttachments;
				std::vector<VkAttachmentReference> colorAttachments;
				std::vector<VkAttachmentReference> depthStencilAttachments;
				std::vector<uint32_t> preservedAttachments;
			};
			using AttachmentDescriptions = std::vector<VkAttachmentDescription>;
			using RenderTargetAttachmentMap = std::unordered_map<size_t, size_t>;
			const std::tuple<AttachmentDescriptions, RenderTargetAttachmentMap> getAttachments(const std::vector<RenderTargetRef>& targets) const;
			const std::vector<SubpassDatas> getSubpassesDatas(const std::vector<RenderTargetRef>& targets, const RenderTargetAttachmentMap& renderTargetAttachmentMap, const uint32_t attachmentCount);
			const std::vector<VkSubpassDependency> findDependencies(const std::vector<SubpassDatas>& subpasses, const std::vector<RenderTargetRef>& renderTargets);
			void createFramebuffers(const uint32_t framebufferCount, const VkExtent2D& extent);
			void create_imgui_context();
		};
		MAKE_REFERENCE(Renderpass);
	}

	template <>
	struct ResourceLoader<vkn::Renderpass>
	{
		static vkn::Renderpass load(vkn::Context& context, ResourceHolder<vkn::RenderTarget, ID<RenderTarget>::Type>& renderTargets, vkn::Swapchain& swapchain, const Renderpass& rp)
		{
			VkExtent2D frameSize{.width = std::numeric_limits<decltype(VkExtent2D::width)>::max(), .height = std::numeric_limits<decltype(VkExtent2D::height)>::max() };
			std::vector<vkn::Pass> passes;
			for (const auto& pass : rp.passes_)
			{
				vkn::Pass p{};
				for (const auto& target : pass.colorTargets_)
				{
					p.usesColorTarget(renderTargets.get(ID<RenderTarget>::get(target), context, target));
					frameSize = min(frameSize, VkExtent2D{.width = target.size.x, .height = target.size.y});
				}
				if (pass.useSwapchainTarget_)
				{
					auto& swapchainTargets = swapchain.renderTargets();
					uint32_t swapchainIndex{};
					for (auto& t : swapchainTargets)
					{
						p.usesColorTarget(t, pass.screenTargetIndex_ + swapchainIndex);
						++swapchainIndex;
					}
					
					frameSize = min(frameSize, swapchain.extent());
				}
				for (const auto& target : pass.depthTargets_)
				{
					p.usesDepthStencilTarget(renderTargets.get(ID<RenderTarget>::get(target), context, target));
					frameSize = min(frameSize, VkExtent2D{ .width = target.size.x, .height = target.size.y });
				}
				for (const auto& target : pass.inputTargets_)
				{
					p.consumesTarget(renderTargets.get(ID<RenderTarget>::get(target), context, target));
				}
				passes.emplace_back(std::move(p));
			}
			if (rp.usesGui_)
			{
				vkn::Pass guiPass{};
				auto & swapchainTargets = swapchain.renderTargets();
				uint32_t swapchainIndex{};
				for (auto& t : swapchainTargets)
				{
					guiPass.usesColorTarget(t, swapchainIndex);
					++swapchainIndex;
				};
				passes.emplace_back(std::move(guiPass));
			}
			return { context, frameSize, std::move(passes), rp.usesGui_ };
		}
	private:
		static VkExtent2D min(const VkExtent2D& lhs, const VkExtent2D rhs)
		{
			return VkExtent2D{ .width = std::min(lhs.width, rhs.width), .height = std::min(lhs.height, rhs.height) };
		}
	};
}