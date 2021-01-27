#include "../headers/Application.h"
#include  "../headers/Gpu.h"
#include <iostream>
#include <sstream>
#include <future>
#include <functional>
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/vulkanFrameGraph.h"
#include "../headers/vulkanContextBuilder.h"
#include "../headers/PipelineBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, eventDispatcher_{ window_.window() }
{
	std::cout << "The application has been launched.\n";
	createContext();
	renderer_ = std::make_unique<vkn::Renderer>(*context_, window_);

	vkn::FrameGraph frameGraph{};
	auto colorAtt = frameGraph.addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
	auto depthAtt = frameGraph.addDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	auto& colorPass = frameGraph.addPass();
	colorPass.addColorAttachment(colorAtt);
	colorPass.addDepthStencilAttachment(depthAtt);
	frameGraph.setPresentAttachment(colorAtt);

	renderTarget_ = std::make_unique<vkn::RenderTarget>(frameGraph.createRenderTarget(*context_, renderer_->swapchain()));
	createPipeline();



	window_.setTitle(name);
	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			if (w != 0 && h != 0)
			{
				window_.resize();
				firstMouseUse_ = true;
			}
			renderTarget_->resize(glm::u32vec2{ w, h });
		});
	eventDispatcher_.addMouseScrollCallback([&](double x, double y)
		{
			onMouseScrollEvent(x, y);
		});
	eventDispatcher_.addMouseButtonCallback([&](uint32_t button, uint32_t action, uint32_t mods)
		{
			onMouseButtonEvent(button, action, mods);
			firstMouseUse_ = true;
		});
	eventDispatcher_.addMouseMoveCallback([&](double x, double y)
		{
			onMouseMoveEvent(x, y);
		});

}

gee::Application::~Application()
{
	context_->device->idle();
}

void gee::Application::setCameraPosition(const glm::vec3& position)
{
	camera_.position_ = position;
}

void gee::Application::setSkybox(Drawable& skybox)
{
}

void gee::Application::addDrawable(Drawable& drawable)
{
	auto exist = std::find_if(std::begin(drawables_), std::end(drawables_), [&](auto& drawableRef) { return drawableRef.get().name == drawable.name; });
	if (exist == std::end(drawables_))
	{
		drawables_.emplace_back(std::ref(drawable));

		auto geometryExist = std::find_if(std::begin(geometryCount_), std::end(geometryCount_), [&](const auto& pair) {return pair.first.get().hash() == drawable.mesh.hash(); });
		if (geometryExist != std::end(geometryCount_))
		{
			++geometryExist->second;
		}
		else
		{
			geometryCount_.emplace_back(std::make_pair(std::ref(drawable.mesh), 1));
				bool diffuseTexExist{ false };
				bool normalTexExist{ false };
				bool specularTexExist{ false };

				ShaderMaterial material{};
				for (auto i = 0u; i < std::size(textures_); ++i)
				{
					const auto& texture = textures_[i].get();
					if (texture.paths_[0] == drawable.mesh.material().diffuseTex.paths_[0])
					{
						diffuseTexExist = true;
						material.diffuseTex = i;
					}
					else if (texture.paths_[0] == drawable.mesh.material().normalTex.paths_[0])
					{
						normalTexExist = true;
						material.normalTex = i;
					}
					else if (texture.paths_[0] == drawable.mesh.material().specularTex.paths_[0])
					{
						specularTexExist = true;
						material.specularTex = i;
					}

					if (diffuseTexExist && normalTexExist && specularTexExist)
						break;
				}
				if (!diffuseTexExist)
				{
					material.diffuseTex = std::size(textures_);
					textures_.emplace_back(std::ref(drawable.mesh.material().diffuseTex));
				}
				if (!normalTexExist)
				{
					material.normalTex = std::size(textures_);
					textures_.emplace_back(std::ref(drawable.mesh.material().normalTex));
				}
				if (!specularTexExist)
				{
					material.specularTex = std::size(textures_);
					textures_.emplace_back(std::ref(drawable.mesh.material().specularTex));
				}
				materials_.emplace(std::ref(drawable.mesh), material);
		}
	}
}

void gee::Application::addCamera(const Camera& camera)
{
}

void gee::Application::updateGui()
{

}

void gee::Application::onMouseMoveEvent(double x, double y)
{
	if (leftButtonPressed_)
	{
		if (firstMouseUse_)
		{
			lastPos_.x = x;
			lastPos_.y = y;
			firstMouseUse_ = false;
		}
		auto windowSize = window_.size();
		auto yaw = (x - lastPos_.x) * 2.0f * 360.0f / windowSize.x;
		auto pitch = (lastPos_.y - y) * 2.0f * 360.0f / windowSize.y;
		lastPos_ = glm::vec2{ x, y };

		camera_.rotate(pitch, yaw);
	}
	if (rightButtonPressed_)
	{

	}
}

void gee::Application::onMouseScrollEvent(double x, double y)
{
	if (y > 0)
	{
		camera_.move(FORWARD);
	}
	else if (y < 0)
	{
		camera_.move(BACKWARD);
	}
}

void gee::Application::onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		leftButtonPressed_ = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		rightButtonPressed_ = true;
		double x, y;
		glfwGetCursorPos(window_.window(), &x, &y);
		//auto drawableIndex = renderer_->objectAt(drawables_, x, y);
		/*if (drawableIndex.has_value())
		{
			if (drawableIndex.value() != lastDrawableIndex_)
			{
				activeDrawable_.emplace(std::ref(drawables_[drawableIndex.value()].get()));
				lastDrawableIndex_ = drawableIndex.value();
			}
			else
			{
				lastDrawableIndex_ = -1;
			}
		}
		else
		{
			lastDrawableIndex_ = -1;
		}*/
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		leftButtonPressed_ = false;
		firstMouseUse_ = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rightButtonPressed_ = false;
	}
}

void gee::Application::createContext()
{

	VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	vkn::ContextBuilder contextBuilder{ severityFlags, messageTypeFlags };
	contextBuilder.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	contextBuilder.addDeviceExtention("VK_KHR_swapchain");
	contextBuilder.addDeviceExtention("VK_EXT_descriptor_indexing");
	contextBuilder.addDeviceExtention("VK_KHR_maintenance3");
	contextBuilder.addQueueFlag(VK_QUEUE_GRAPHICS_BIT);
	contextBuilder.addQueueFlag(VK_QUEUE_TRANSFER_BIT);
	contextBuilder.setQueueCount(2);

	context_ = std::make_unique<vkn::Context>(std::move(contextBuilder.build(window_)));
}

void gee::Application::createPipeline()
{
	vkn::PipelineBuilder builder{ "../assets/shaders/vert.spv", "../assets/shaders/frag.spv" };
	builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder.addDepthStage(VK_COMPARE_OP_LESS);
	builder.addColorBlendStage();
	builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder.renderpass = renderTarget_->renderpass->renderpass();
	builder.subpass = 0;
	colorPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));
}

void gee::Application::getTransforms()
{
	for (auto& drawableRef : drawables_)
	{
		auto& drawable = drawableRef.get();
		modelMatrices_.emplace_back(drawable.getTransform());
		normalMatrices_.emplace_back(drawable.getNormalMatrix());
		if (drawable.hasLightComponent())
		{
			auto& light = drawable.light();

			auto& shaderLight = pointLights_.emplace_back(gee::ShaderPointLight{});
			shaderLight.ambient = glm::vec4{ light.ambient, 1.0f };
			shaderLight.diffuse = glm::vec4{ light.diffuse, 1.0 };
			shaderLight.specular = glm::vec4{light.specular, 1.0f};
			shaderLight.position = glm::vec4{light.position, 1.0f};
			shaderLight.linear = light.linear;
			shaderLight.quadratic = light.quadratic;
		}
	}
}

bool gee::Application::isRunning()
{
	ShaderCamera camera{};
	camera.position = glm::vec4{ camera_.position_, 1.0f };
	camera.viewProj = camera_.viewProjMatrix(window_.aspectRatio());

	glm::mat4 model{ 1.0f };

	renderer_->begin(*renderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
	renderer_->usePipeline(*colorPipeline_);
	renderer_->updateBuffer("Camera", camera);
	getTransforms();
	renderer_->setTextures("textures", textures_);
	renderer_->updateBuffer("Model_Matrix", modelMatrices_);
	renderer_->updateBuffer("Normal_Matrix", normalMatrices_);
	renderer_->updateBuffer("PointLights", pointLights_);
	for (const auto& [mesh, count] : geometryCount_)
	{
		renderer_->updateSmallBuffer("material", materials_[mesh]);
		renderer_->draw(mesh, count);
	}

	modelMatrices_.clear();
	normalMatrices_.clear();
	pointLights_.clear();
	renderer_->end(*renderTarget_);
	return window_.isOpen();
}