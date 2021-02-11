#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <utility>
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "image.h"

namespace vkn
{
	class Framebuffer
	{
	public:
		Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, const VkExtent2D& dimensions, const uint32_t frameCount = 2);
		Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, Swapchain& swapchain, const uint32_t presentAttachment);
		Framebuffer(Framebuffer&&) = default;
		~Framebuffer();
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		const float rawContentAt(const std::string& attachmentName, const VkDeviceSize offset, const uint32_t imageIndex = 0u);
		const size_t frameCount() const;
		VkFramebuffer frame(const uint32_t index) const;
		void resize(const glm::u32vec2& size);
		const VkExtent2D& dimensions() const;
		const bool isOffscreen() const;
		std::unordered_map<std::string, vkn::Image>& attachments(const uint32_t imageIndex);
	private:
		vkn::Context& context_;
		VkExtent2D dimensions_;
		std::vector<VkFramebuffer> framebuffers_;
		std::shared_ptr<Renderpass> renderpass_;
		std::optional<std::reference_wrapper<Swapchain>> swapchain_;
		std::optional<uint32_t> presentAttachment_;

		VkImageAspectFlags getAspectFlag(const VkAttachmentDescription& attachment);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<VkAttachmentDescription>& renderpassAttachments, const uint32_t frameCount);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<VkAttachmentDescription>& renderpassAttachments, const uint32_t presentAttachmentIndex, const uint32_t frameCount);
		
		//Wrapper class is needed to work around MVC design choice on noexcept move ctor
		//https://www.reddit.com/r/cpp/comments/6q94ai/chromium_windows_builds_now_use_clangcl_by_default/dkwdd8l/
		template <class T>
		struct Wrapper : public T
		{
			Wrapper() = default;
			Wrapper(const Wrapper&) = delete;
			Wrapper(Wrapper&&) = default;
			Wrapper& operator=(const Wrapper&) = delete;
			Wrapper& operator=(Wrapper&&) = default;
		};
		std::vector<Wrapper<std::unordered_map<std::string, vkn::Image>>> images_;
	};
}