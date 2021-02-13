#include "../headers/Application.h"
#include <iostream>
#include <sstream>
#include <future>
#include <numeric>
#include <algorithm>
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/vulkanFrameGraph.h"
#include "../headers/vulkanContextBuilder.h"
#include "../headers/PipelineBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, eventDispatcher_{ window_.window() }
{
	std::cout << "The application has been launched.\n";
	cubeMesh_ = std::make_unique<gee::Mesh>(gee::getCubeMesh());
	quadMesh_ = std::make_unique<gee::Mesh>(gee::getQuadMesh());
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
	auto colorAtt = frameGraph.addColorAttachment("color", VK_FORMAT_R32G32B32A32_SFLOAT);
	auto inputAtt = frameGraph.addColorAttachment("input", VK_FORMAT_R32G32B32A32_SFLOAT);
	auto depthAtt = frameGraph.addDepthAttachment("depth", VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	frameGraph.setAttachmentColorDepthContent(inputAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	frameGraph.setPresentAttachment(inputAtt);

	auto& skyboxPass = frameGraph.addPass();
	skyboxPass.addColorAttachment(colorAtt);
	skyboxPass.addDepthStencilAttachment(depthAtt);

	auto& colorPass = frameGraph.addPass();
	colorPass.addColorAttachment(colorAtt);
	colorPass.addDepthStencilAttachment(depthAtt);

	auto& gammaCorrectionPass = frameGraph.addPass();
	gammaCorrectionPass.addColorAttachment(inputAtt);
	gammaCorrectionPass.addInputAttachment(depthAtt);

	auto& guiPass = frameGraph.addPass();
	guiPass.addColorAttachment(inputAtt);

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
			pixelPerfectRenderTarget_->resize(glm::u32vec2{ w, h });
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
	queryPool_ = std::make_unique<vkn::QueryPool>(*context_, VK_QUERY_TYPE_TIMESTAMP, 2);
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
	auto& drawables = drawablesGeometries_[std::ref(drawable.mesh)];
	auto result = std::find_if(std::begin(drawables), std::end(drawables), [&](const auto& drawableRef) { return drawableRef.get().name == drawable.name; });
	if (result == std::end(drawables))
	{
		drawables.emplace_back(std::ref(drawable));
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

	ImGui::Begin("Loop time");
	ImGui::LabelText("cpu time (ms)", std::to_string(cpuTime_).c_str(), "0.3f");
	ImGui::LabelText("gpu time (ms)", std::to_string(gpuTime_).c_str(), "0.3f");
	ImGui::End();

	ImGui::Begin("Postprocess effects");
	ImGui::Checkbox("use hdr", &useHdr_);
	ImGui::SliderFloat("exposure", &exposure_, 0.1, 5.0f, "%.1f");
	ImGui::Checkbox("use gamma correction", &useGammaCorrection_);
	ImGui::End();
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

		renderer_->begin();
		renderer_->beginTarget(*pixelPerfectRenderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
		renderer_->usePipeline(*pixelPerfectPipeline_);
		ShaderCamera camera{};
		camera.position = glm::vec4{ camera_.position_, 1.0f };
		camera.viewProj = camera_.viewProjMatrix(window_.aspectRatio());
		renderer_->updateSmallBuffer("camera", camera);
		for (const auto& [mesh, drawableRefs] : drawablesGeometries_)
		{
			renderer_->updateBuffer("Model_Matrix", modelMatrices_);
			renderer_->draw(mesh, std::size(drawableRefs));
		}
		renderer_->endTarget(*pixelPerfectRenderTarget_);
		renderer_->end();
		auto value = pixelPerfectRenderTarget_->rawContextAt("color", x, y);
		auto index = static_cast<size_t>(value * 255);
		activeDrawable_ = std::nullopt;
		if (index > 0)
		{
			--index;
			if ((lastDrawableIndex_.has_value() && (lastDrawableIndex_.value() != index)) || !lastDrawableIndex_.has_value())
			{
				size_t i{};
				for (const auto& [mesh, drawableRefs] : drawablesGeometries_)
				{
					if ((i <= index) && (i + std::size(drawableRefs)) > index)
					{
						activeDrawable_.emplace(std::ref(drawableRefs[index - i]));
						break;
					}
					i += std::size(drawableRefs);
				}
				lastDrawableIndex_ = index;
			}
			else
			{
				lastDrawableIndex_ = std::nullopt;
			}
		}
		else
		{
			lastDrawableIndex_ = std::nullopt;
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
	contextBuilder.addDeviceExtention("VK_KHR_push_descriptor");
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


	builder.setShaderVertexStage("../assets/shaders/vert.spv");
	builder.setShaderFragmentStage("../assets/shaders/frag.spv");
	builder.subpass = 1;
	colorPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));


	builder.setShaderVertexStage("../assets/shaders/gamma Correction/vert.spv");
	builder.setShaderFragmentStage("../assets/shaders/gamma Correction/frag.spv");
	builder.subpass = 2;
	gammaCorrectionPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));
}

void gee::Application::initPixelPerfect()
{
	vkn::FrameGraph frameGraph{};
	auto colorAtt = frameGraph.addColorAttachment("color", VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	auto depthAtt = frameGraph.addDepthAttachment("depth", VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	frameGraph.setRenderArea(window_.size().x, window_.size().y);
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
	for (auto& [mesh, drawableRefs] : drawablesGeometries_)
	{
		for (const auto& drawableRef : drawableRefs)
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
				shaderLight.specular = glm::vec4{ light.specular, 1.0f };
				shaderLight.position = glm::vec4{ light.position, 1.0f };
				shaderLight.linear = light.linear;
				shaderLight.quadratic = light.quadratic;
			}
		}
	}
}

bool gee::Application::isRunning()
{
	if ((renderingtimer_.ellapsedMs() >= 100 / 6.0f))
	{
		renderingtimer_.reset();
		cpuTimer_.reset();
		updateGui();
		getTransforms();

		renderer_->begin();
		auto beginQuery = renderer_->writeTimestamp(*queryPool_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		renderer_->beginTarget(*renderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
		renderer_->usePipeline(*skyboxPipeline_);
		auto proj = camera_.perspectiveProjection(window_.aspectRatio());
		proj[1][1] *= -1;
		auto view = glm::mat4{ glm::mat3{camera_.pointOfView()} };
		renderer_->updateSmallBuffer("camera", proj * view);
		renderer_->setTexture("skybox", *skyboxTexture_, VK_IMAGE_VIEW_TYPE_CUBE, 6);
		renderer_->draw(*cubeMesh_);
		renderer_->clearDepthAttachment(*renderTarget_);

		renderer_->usePipeline(*colorPipeline_);
		ShaderCamera camera{};
		camera.position = glm::vec4{ camera_.position_, 1.0f };
		camera.viewProj = camera_.viewProjMatrix(window_.aspectRatio());
		renderer_->updateSmallBuffer("camera", camera);
		renderer_->updateBuffer("Model_Matrix", modelMatrices_);
		renderer_->updateBuffer("Normal_Matrix", normalMatrices_);
		renderer_->updateBuffer("PointLights", pointLights_);
		renderer_->updateBuffer("LightCount", std::size(pointLights_));
		for (const auto& [mesh, drawableRefs] : drawablesGeometries_)
		{
			renderer_->setTexture("diffuseTex", mesh.get().material().diffuseTex);
			renderer_->setTexture("normalTex", mesh.get().material().normalTex);
			renderer_->setTexture("specularTex", mesh.get().material().specularTex);
			renderer_->draw(mesh, std::size(drawableRefs));
		}


		renderer_->usePipeline(*gammaCorrectionPipeline_);
		renderer_->setTexture("outputTexture", renderTarget_->attachmentImage("color"));
		renderer_->updateBuffer("Exposure", exposure_);
		renderer_->updateBuffer("HDR", useHdr_);
		renderer_->updateBuffer("GammaCorrection", useGammaCorrection_);
		renderer_->draw(*quadMesh_);

		imguiContext_->render(*renderer_, *renderTarget_);
		renderer_->endTarget(*renderTarget_);

		auto endQuery = renderer_->writeTimestamp(*queryPool_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		renderer_->end();

		cpuTime_ = cpuTimer_.ellapsedMs();
		auto e = endQuery.results();
		auto b = beginQuery.results();
		if (b < e)
		{
			gpuTime_ = (e - b) / 1000000.0f;
		}
	}
	return window_.isOpen();
}