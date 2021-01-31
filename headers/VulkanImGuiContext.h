#pragma once
#include "Window.h"
#include "vulkanContext.h"
#include "RenderTarget.h"
#include "vulkanFrameGraph.h"

namespace vkn
{
	class ImGuiContext
	{
	public:
		ImGuiContext(gee::Window& window, Context& context, RenderTarget& renderTarget, const Pass& guiPass);
		~ImGuiContext();
		void render();
	private:
		Context& context_;
		RenderTarget& renderTarget_;
		uint32_t passIndex_;
		VkDescriptorPool descriptorPool_{ VK_NULL_HANDLE };

		void loadFontsTextures();
	};

}
