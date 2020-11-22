#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "pipeline.h"
#include "Renderpass.h"
#include "CommandBuffer.h"
#include "MemoryLocation.h"
#include "Drawable.h"

namespace vkn
{
	template <typename T> using Ptr = std::unique_ptr<T>;
	using Hash_t = size_t;
	class ShaderEffect
	{
	public:
		enum class Requirement
		{
			Camera,
			Model_matrice,
			Material,
			Light
		};
		using Function_t = std::function<void(vkn::CommandBuffer&, const VkViewport&, const VkRect2D&, const std::unordered_map<Hash_t, vkn::MemoryLocation>&, const std::unordered_map<Hash_t, uint32_t>&)>;
		ShaderEffect(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, Function_t& function, const VkImageLayout finalLayout);
		void execute(vkn::CommandBuffer& cb, const VkViewport& viewport, const VkRect2D& renderArea, const std::unordered_map<Hash_t, vkn::MemoryLocation>& memoryLocations, const std::unordered_map<Hash_t, uint32_t>& drawables);
		void setPolygonMode(const VkPolygonMode mode);
		void setLineWidth(const float width);
		void setSampleCount(const VkSampleCountFlagBits count);
		void setLoadOp(const VkAttachmentLoadOp op);
		void setStoreOp(const VkAttachmentStoreOp op);
		void updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage);
		void updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage);
		const Requirement getRequirement() const;
		void begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent);
		void end(vkn::CommandBuffer& cb);
		template<class T>
		void pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas);
		template<class T>
		void updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage);

	private:
		Ptr<vkn::Pipeline> pipeline_;
		Ptr<vkn::Renderpass> renderpass_;
		Function_t drawCall_;
		const std::string vertexShaderPath_;
		const std::string fragmentShaderPath_;
		VkPolygonMode polyMode_{ VK_POLYGON_MODE_FILL };
		VkSampleCountFlagBits sampleCount_{ VK_SAMPLE_COUNT_1_BIT };
		float lineWidth_{ 1.0f };
		VkAttachmentLoadOp loadOp_{ VK_ATTACHMENT_LOAD_OP_CLEAR };
		VkAttachmentStoreOp storeOp_{ VK_ATTACHMENT_STORE_OP_DONT_CARE };
		VkImageLayout finalLayout_{};
		std::vector<VkFormat> attachmentFormats_;
		Requirement requirement{};
		void create(vkn::Gpu& gpu, vkn::Device& device, const VkImageLayout initialLayout);
	};
	template<class T>
	inline void ShaderEffect::pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas)
	{
		assert(pipeline_ && "The pipeline should be created first");
		pipeline_->pushConstant(cb, name, datas);
	}
	template<class T>
	inline void ShaderEffect::updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage)
	{
		assert(pipeline_ && "The pipeline should be created first");
		pipeline_->updateBuffer(resourceName, datas, stage);
	}
}