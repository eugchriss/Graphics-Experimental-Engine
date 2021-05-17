cmake_minimum_required(VERSION 3.0)

project(gee LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKEBUILD_TYPE})	
set(LIBS_DIR "${gee_SOURCE_DIR}/libs")
add_subdirectory(${LIBS_DIR})

find_package(Vulkan REQUIRED FATAL_ERROR)

set(SOURCES	"sources/application.cpp"
			"sources/camera.cpp"
			"sources/eventDispatcher.cpp"
			"sources/drawable.cpp"
			"sources/main.cpp"
			"sources/mesh.cpp"
			"sources/modelLoader.cpp"
			"sources/optics.cpp"
			"sources/resourceHolder.cpp"
			"sources/SpatialTransformation.cpp"
			"sources/timer.cpp"
			"sources/texture.cpp"
			"sources/units.cpp"
			"sources/utility.cpp"
			"sources/window.cpp"
			
			"sources/impl/vulkan/buffer.cpp"
			"sources/impl/vulkan/commandbuffer.cpp"
			"sources/impl/vulkan/commandpool.cpp"
			"sources/impl/vulkan/debugMessenger.cpp"
			"sources/impl/vulkan/device.cpp"
			"sources/impl/vulkan/deviceMemory.cpp"
			"sources/impl/vulkan/gpu.cpp"
			"sources/impl/vulkan/instance.cpp"
			"sources/impl/vulkan/image.cpp"
			"sources/impl/vulkan/imgui.cpp"
			"sources/impl/vulkan/imgui_demo.cpp"
			"sources/impl/vulkan/imgui_draw.cpp"
			"sources/impl/vulkan/imgui_widgets.cpp"
			"sources/impl/vulkan/imgui_impl_glfw.cpp"
			"sources/impl/vulkan/imgui_impl_vulkan.cpp"
			"sources/impl/vulkan/imgui_tables.cpp"
			"sources/impl/vulkan/material.cpp"
			"sources/impl/vulkan/meshMemoryLocation.cpp"
			"sources/impl/vulkan/pass.cpp"
			"sources/impl/vulkan/pipeline.cpp" 
			"sources/impl/vulkan/pipelineBuilder.cpp"
			"sources/impl/vulkan/pipelineLayout.cpp"
			"sources/impl/vulkan/query.cpp"
			"sources/impl/vulkan/queryPool.cpp"
			"sources/impl/vulkan/queue.cpp"
			"sources/impl/vulkan/queueFamily.cpp"
			"sources/impl/vulkan/renderTarget.cpp"
			"sources/impl/vulkan/renderpass.cpp"
			"sources/impl/vulkan/signal.cpp"
			"sources/impl/vulkan/shader.cpp"
			"sources/impl/vulkan/swapchain.cpp"
			"sources/impl/vulkan/textureImageFactory.cpp"
			"sources/impl/vulkan/vulkanContext.cpp"
			"sources/impl/vulkan/vulkanContextBuilder.cpp"
			"sources/impl/vulkan/vulkan_utils.cpp"
			"sources/impl/vulkan/vulkanImGuiContext.cpp"
			)

set(HEADERS "headers/application.h"
			"headers/camera.h"
			"headers/eventDispatcher.h"
			"headers/drawable.h"
			"headers/materialInstance.h"
			"headers/mesh.h"
			"headers/modelLoader.h"
			"headers/optics.h"
			"headers/resourceHolder.h"
			"headers/SpatialTransformation.h"
			"headers/texture.h"
			"headers/timer.h" 
			"headers/units.h"
			"headers/utility.h"
			"headers/vertex.h"
			"headers/window.h"
			
			"headers/impl/imstb_rectpack.h"
			"headers/impl/imstb_textedit.h"
			"headers/impl/imstb_truetype.h"

			"headers/impl/vulkan/buffer.h"
			"headers/impl/vulkan/commandbuffer.h"
			"headers/impl/vulkan/commandpool.h"
			"headers/impl/vulkan/debugMessenger.h"
			"headers/impl/vulkan/device.h"
			"headers/impl/vulkan/deviceMemory.h"
			"headers/impl/vulkan/gpu.h"
			"headers/impl/vulkan/image.h"
			"headers/impl/vulkan/imgui.h"
			"headers/impl/vulkan/imgui_impl_glfw.h"
			"headers/impl/vulkan/imgui_impl_vulkan.h"
			"headers/impl/vulkan/imgui_internal.h"
			"headers/impl/vulkan/instance.h"
			"headers/impl/vulkan/material.h"
			"headers/impl/vulkan/meshMemoryLocation.h"
			"headers/impl/vulkan/pass.h"
			"headers/impl/vulkan/pipeline.h"
			"headers/impl/vulkan/pipelineBuilder.h"
			"headers/impl/vulkan/pipelineLayout.h"
			"headers/impl/vulkan/queue.h"
			"headers/impl/vulkan/queueFamily.h"
			"headers/impl/vulkan/query.h"
			"headers/impl/vulkan/queryPool.h"
			"headers/impl/vulkan/renderpass.h"
			"headers/impl/vulkan/renderTarget.h"
			"headers/impl/vulkan/signal.h"
			"headers/impl/vulkan/shader.h"
			"headers/impl/vulkan/swapchain.h"
			"headers/impl/vulkan/textureImageFactory.h"
			"headers/impl/vulkan/vulkan_utils.h"
			"headers/impl/vulkan/vulkanContext.h"
			"headers/impl/vulkan/vulkanContextBuilder.h"
			"headers/impl/vulkan/vulkanImGuiContext.h"
			)
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