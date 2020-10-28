cmake_minimum_required(VERSION 3.0)

project(gee LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKEBUILD_TYPE})	
set(LIBS_DIR "${gee_SOURCE_DIR}/libs")
add_subdirectory(${LIBS_DIR})

find_package(Vulkan REQUIRED FATAL_ERROR)

set(SOURCES	"sources/main.cpp"
			"sources/imgui.cpp"
			"sources/imgui_demo.cpp"
			"sources/imgui_draw.cpp"
			"sources/imgui_widgets.cpp"
			"sources/imgui_impl_glfw.cpp"
			"sources/imgui_impl_vulkan.cpp"
			"sources/gpu.cpp"
			"sources/instance.cpp"
			"sources/window.cpp"
			"sources/application.cpp"
			"sources/vulkan_utils.cpp"
			"sources/queueFamily.cpp"
			"sources/queue.cpp"
			"sources/signal.cpp"
			"sources/commandbuffer.cpp"
			"sources/commandpool.cpp"
			"sources/debugMessenger.cpp"
			"sources/device.cpp"
			"sources/vulkanRenderer.cpp"
			"sources/swapchain.cpp"
			"sources/image.cpp"
			"sources/deviceMemory.cpp"
			"sources/buffer.cpp"
			"sources/renderpassBuilder.cpp"
			"sources/renderpass.cpp"
			"sources/framebuffer.cpp"
			"sources/shader.cpp"
			"sources/pipelineBuilder.cpp"
			"sources/pipeline.cpp"
			"sources/pipelineLayout.cpp"
			"sources/mesh.cpp"
			"sources/memoryLocation.cpp"
			"sources/timer.cpp"
			"sources/modelLoader.cpp"
			"sources/resourceHolder.cpp"
			"sources/texture.cpp"
			"sources/camera.cpp"
			"sources/units.cpp"
			"sources/eventDispatcher.cpp"
			"sources/drawable.cpp"
			"sources/shaderTechnique.cpp"
			"sources/material.cpp"
			"sources/pointLight.cpp"
			"sources/skybox.cpp")
			

set(HEADERS	"headers/gpu.h"
			"headers/imgui.h"
			"headers/imgui_impl_glfw.h"
			"headers/imgui_impl_vulkan.h"
			"headers/imgui_internal.h"
			"headers/imstb_rectpack.h"
			"headers/imstb_textedit.h"
			"headers/imstb_truetype.h"
			"headers/instance.h"
			"headers/window.h"
			"headers/application.h"
			"headers/vulkan_utils.h"
			"headers/queueFamily.h"
			"headers/queue.h"
			"headers/signal.h"
			"headers/commandbuffer.h"
			"headers/commandpool.h"
			"headers/debugMessenger.h"
			"headers/device.h"
			"headers/vulkanRenderer.h"
			"headers/swapchain.h"
			"headers/image.h"
			"headers/deviceMemory.h"
			"headers/buffer.h"
			"headers/renderpassBuilder.h"
			"headers/renderpass.h"
			"headers/framebuffer.h"
			"headers/shader.h"
			"headers/pipelineBuilder.h"
			"headers/pipeline.h"
			"headers/pipelineLayout.h"
			"headers/mesh.h"
			"headers/memoryLocation.h"
			"headers/timer.h"
			"headers/modelLoader.h"
			"headers/resourceHolder.h"
			"headers/texture.h"
			"headers/camera.h"
			"headers/units.h"
			"headers/eventDispatcher.h"
			"headers/drawable.h"
			"headers/shaderTechnique.h"
			"headers/material.h"
			"headers/pointLight.h"
			"headers/skybox.h")

add_executable(gee ${SOURCES} ${HEADERS})

target_include_directories(gee PRIVATE ${LIBS_DIR}/assimp)
target_include_directories(gee PRIVATE ${LIBS_DIR}/eventpp/include)
target_include_directories(gee PUBLIC ${LIBS_DIR}/glm)
target_include_directories(gee PRIVATE ${LIBS_DIR}/SPIRV-Cross)
target_include_directories(gee PRIVATE ${LIBS_DIR}/stb)
target_include_directories(gee PRIVATE ${LIBS_DIR}/imgui)
target_include_directories(gee PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(gee assimp)
target_link_libraries(gee glfw)
target_link_libraries(gee ${Vulkan_LIBRARY})
target_link_libraries(gee spirv-cross-cpp)


set_target_properties(gee PROPERTIES CXX_STANDARD 17)