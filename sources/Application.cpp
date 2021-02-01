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
	cubeMesh_ = std::make_unique<gee::Mesh>(gee::getCubeMesh());
	std::array<std::string, 6> skyboxPaths{ "../assets/skybox/space/right.png",
											"../assets/skybox/space/left.png",
											"../assets/skybox/space/top.png",
											"../assets/skybox/space/bottom.png",
											"../assets/skybox/space/front.png",
											"../assets/skybox/space/back.png" };
	skyboxTexture_ = std::make_unique<gee::Texture>(skyboxPaths, gee::Texture::ColorSpace::LINEAR);
	createContext();
	renderer_ = std::make_unique<vkn::Renderer>(*context_, window_);

	vkn::FrameGraph frameGraph{};
	auto colorAtt = frameGraph.addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
	auto depthAtt = frameGraph.addDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	frameGraph.setPresentAttachment(colorAtt);
	
	auto& skyboxPass = frameGraph.addPass();
	skyboxPass.addColorAttachment(colorAtt);
	skyboxPass.addDepthStencilAttachment(depthAtt);
	
	auto& colorPass = frameGraph.addPass();
	colorPass.addColorAttachment(colorAtt);
	colorPass.addDepthStencilAttachment(depthAtt);

	auto& guiPass = frameGraph.addPass();
	guiPass.addColorAttachment(colorAtt);

	renderTarget_ = std::make_unique<vkn::RenderTarget>(frameGraph.createRenderTarget(*context_, renderer_->swapchain()));
	createPipeline();

	imguiContext_ = std::make_unique<vkn::ImGuiContext>(window_, *context_, *renderTarget_, guiPass);

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
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				onMouseScrollEvent(x, y);
			}
		});
	eventDispatcher_.addMouseButtonCallback([&](uint32_t button, uint32_t action, uint32_t mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				onMouseButtonEvent(button, action, mods);
			}
			else
			{
				firstMouseUse_ = true;
			}
		});
	eventDispatcher_.addMouseMoveCallback([&](double x, double y)
		{
			onMouseMoveEvent(x, y);
		});

	initPixelPerfect();
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
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (activeDrawable_)
	{
		auto& drawable = activeDrawable_.value().get();
		ImGui::Begin(drawable.name.c_str());
		auto position = drawable.getPosition();
		ImGui::SliderFloat3("position", &position.x, -500.0_m, 500.0_m, "%.1f");

		auto color = drawable.getColor();
		ImGui::ColorEdit4("color", &color.x);

		//the engine uses radians units in internal but degrees for display
		auto rotation = drawable.getRotation();
		glm::vec3 rotationDeg = glm::degrees(rotation);
		ImGui::SliderFloat3("rotation", &rotationDeg.x, 0.0f, 360.0f, "%.1f");
		rotation = glm::radians(rotationDeg);

		auto lastScaleFactor = drawable.scaleFactor;
		ImGui::SliderFloat("scale factor", &drawable.scaleFactor, 0.0f, 10.0f, "%.1f");

		auto size = drawable.getSize();
		auto sizeStr = std::string{ "x = " } + std::to_string(size.x) + std::string{ " y = " } + std::to_string(size.y) + std::string{ " z = " } + std::to_string(size.z);
		ImGui::LabelText("size", sizeStr.c_str());
		if (drawable.hasLightComponent())
		{
			auto& light = drawable.light();
			ImGui::ColorEdit3("ambient", &light.ambient.x);
			ImGui::ColorEdit3("diffuse", &light.diffuse.x);
			ImGui::SliderFloat("specular", &light.specular.x, 0.0f, 256.0f, "%f");
			light.specular.y = light.specular.x;
			light.specular.z = light.specular.x;
			ImGui::SliderFloat("linear", &light.linear, 0.0014f, 0.7, "%.4f");
			ImGui::SliderFloat("quadratic", &light.quadratic, 0.0000007f, 1.8, "%.7f");
		}
		ImGui::End();

		drawable.setPosition(position);
		drawable.setRotation(rotation);
		drawable.setColor(color);
		if (drawable.scaleFactor == 0.0f)
		{
			drawable.scaleFactor = lastScaleFactor;
		}
		drawable.setSize(drawable.getSize() * drawable.scaleFactor / lastScaleFactor);
	}
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
		
		renderer_->begin(*pixelPerfectRenderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
		renderer_->usePipeline(*pixelPerfectPipeline_);
		ShaderCamera camera{};
		camera.position = glm::vec4{ camera_.position_, 1.0f };
		camera.viewProj = camera_.viewProjMatrix(window_.aspectRatio());
		renderer_->updateSmallBuffer("camera", camera);
		renderer_->updateBuffer("Model_Matrix", modelMatrices_);
		for (const auto& [mesh, count] : geometryCount_)
		{
			renderer_->draw(mesh, count);
		}
		renderer_->end(*pixelPerfectRenderTarget_);
		auto value = pixelPerfectRenderTarget_->rawContextAt(x, y);
		auto index = static_cast<size_t>(value * 255);
		activeDrawable_ = std::nullopt;
		if (index > 0)
		{
			if (index != lastDrawableIndex_)
			{
				activeDrawable_.emplace(std::ref(drawables_[index - 1].get()));
				lastDrawableIndex_ = index;
			}
			else
			{
				lastDrawableIndex_ = 0;
			}
		}
		else
		{
			lastDrawableIndex_ = 0;
		}
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
	vkn::PipelineBuilder builder{ "../assets/shaders/skybox/vert.spv", "../assets/shaders/skybox/frag.spv" };
	builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder.addDepthStage(VK_COMPARE_OP_LESS);
	builder.addColorBlendStage();
	builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder.renderpass = renderTarget_->renderpass->renderpass();
	builder.subpass = 0;
	skyboxPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));

	
	vkn::PipelineBuilder builder2{ "../assets/shaders/vert.spv", "../assets/shaders/frag.spv" };
	builder2.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder2.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder2.addDepthStage(VK_COMPARE_OP_LESS);
	builder2.addColorBlendStage();
	builder2.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder2.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder2.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder2.renderpass = renderTarget_->renderpass->renderpass();
	builder2.subpass = 1;
	colorPipeline_ = std::make_unique<vkn::Pipeline>(builder2.get(*context_));
}

void gee::Application::initPixelPerfect()
{
	vkn::FrameGraph frameGraph{};
	auto colorAtt = frameGraph.addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	auto depthAtt = frameGraph.addDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	frameGraph.setRenderArea(window_.size().x, window_.size().y);
	frameGraph.setPresentAttachment(colorAtt);
	auto& pixelPerfectPass = frameGraph.addPass();
	pixelPerfectPass.addColorAttachment(colorAtt);
	pixelPerfectPass.addDepthStencilAttachment(depthAtt);

	pixelPerfectRenderTarget_ = std::make_unique<vkn::RenderTarget>(frameGraph.createRenderTarget(*context_));

	vkn::PipelineBuilder builder{ "../assets/shaders/pixelPerfect/vert.spv", "../assets/shaders/pixelPerfect/frag.spv" };
	builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder.addDepthStage(VK_COMPARE_OP_LESS);
	builder.addColorBlendStage();
	builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder.renderpass = pixelPerfectRenderTarget_->renderpass->renderpass();
	builder.subpass = 0;
	pixelPerfectPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));
}

void gee::Application::getTransforms()
{
	modelMatrices_.clear();
	normalMatrices_.clear();
	pointLights_.clear();
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
	updateGui();

	getTransforms();
	renderer_->begin(*renderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
	renderer_->usePipeline(*skyboxPipeline_);
	auto proj = camera_.perspectiveProjection(window_.aspectRatio());
	proj[1][1] *= -1;
	auto view = glm::mat4{ glm::mat3{camera_.pointOfView()} };
	renderer_->updateSmallBuffer("camera", proj * view);
	renderer_->setTexture("skybox", *skyboxTexture_, VK_IMAGE_VIEW_TYPE_CUBE, 6);
	renderer_->draw(*cubeMesh_);
	renderTarget_->clearDepthAttachment();

	renderer_->usePipeline(*colorPipeline_);
	ShaderCamera camera{};
	camera.position = glm::vec4{ camera_.position_, 1.0f };
	camera.viewProj = camera_.viewProjMatrix(window_.aspectRatio());
	renderer_->updateSmallBuffer("camera", camera);
	renderer_->setTextures("textures", textures_);
	renderer_->updateBuffer("Model_Matrix", modelMatrices_);
	renderer_->updateBuffer("Normal_Matrix", normalMatrices_);
	renderer_->updateBuffer("PointLights", pointLights_);
	for (const auto& [mesh, count] : geometryCount_)
	{
		renderer_->updateSmallBuffer("material", materials_[mesh]);
		renderer_->draw(mesh, count);
	}
	imguiContext_->render();
	renderer_->end(*renderTarget_);
	return window_.isOpen();
}