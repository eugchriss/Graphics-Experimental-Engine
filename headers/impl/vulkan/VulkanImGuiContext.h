#pragma once
#include "../../Window.h"
#include "Renderpass.h"
#include "vulkanContext.h"
namespace vkn
{
	class ImGuiContext
	{
	public:
		ImGuiContext(gee::Window& window, Context& context, const Renderpass& renderpass);
		~ImGuiContext();
		void render(CommandBuffer& cb);
	private:
		Context& context_;
		VkDescriptorPool descriptorPool_{ VK_NULL_HANDLE };

		void loadFontsTextures();
	};

}
