#include "../headers/VulkanImGuiContext.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/imgui_impl_glfw.h"
#include "../headers/CommandPool.h"

vkn::ImGuiContext::ImGuiContext(gee::Window& window, Context& context, RenderTarget& renderTarget, const Pass& guiPass): context_{context}, renderTarget_{renderTarget}, passIndex_{ guiPass.index() }
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.maxSets = 1000 * std::size(pool_sizes);
	poolInfo.poolSizeCount = std::size(pool_sizes);
	poolInfo.pPoolSizes = std::data(pool_sizes);
	vkn::error_check(vkCreateDescriptorPool(context_.device->device, &poolInfo, nullptr, &descriptorPool_), "Unabled to create imgui descriptor pool");

	const auto imageCount = 3;
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window.window(), true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = context_.instance->instance;
	initInfo.PhysicalDevice = context_.gpu->device;
	initInfo.Device = context_.device->device;
	initInfo.QueueFamily = context_.queueFamily->familyIndex();
	initInfo.Queue = context_.graphicsQueue->queue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = descriptorPool_;
	initInfo.Allocator = nullptr;
	initInfo.MinImageCount = imageCount;
	initInfo.ImageCount = imageCount;
	initInfo.CheckVkResultFn = nullptr;
	initInfo.Subpass = passIndex_;

	ImGui_ImplVulkan_Init(&initInfo, renderTarget.renderpass->renderpass());
	loadFontsTextures();
}

vkn::ImGuiContext::~ImGuiContext()
{
	if (descriptorPool_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(context_.device->device, descriptorPool_, nullptr);
	}
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void vkn::ImGuiContext::render(Renderer& renderer, const RenderTarget& target)
{
	if (target.isBound());
	{
		render(renderer.currentCmdBuffer());
	}
}


void vkn::ImGuiContext::render(CommandBuffer& cb)
{
	assert(cb.isRecording() && "Command buffer needs to be in recording state");
	if (passIndex_ != 0)
	{
		vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
	}
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.commandBuffer());
}

void vkn::ImGuiContext::loadFontsTextures()
{
	CommandPool pool{ context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto cb = pool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	ImGui_ImplVulkan_CreateFontsTexture(cb.commandBuffer());
	cb.end();

	context_.transferQueue->submit(cb);
	context_.transferQueue->idle();
}
