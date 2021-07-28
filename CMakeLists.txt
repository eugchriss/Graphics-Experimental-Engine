cmake_minimum_required(VERSION 3.8)

project(gee LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKEBUILD_TYPE})
set(LIBS_DIR "${gee_SOURCE_DIR}/libs")
add_subdirectory(${LIBS_DIR})

find_package(Vulkan REQUIRED FATAL_ERROR)

set(dear_imgui_files
			"${gee_SOURCE_DIR}/libs/imgui/imgui.h"
			"${gee_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.h"
			"${gee_SOURCE_DIR}/libs/imgui/backends/imgui_impl_vulkan.h"
			"${gee_SOURCE_DIR}/libs/imgui/imgui_internal.h"

			"${gee_SOURCE_DIR}/libs/imgui/imgui.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/imgui_demo.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/imgui_draw.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/backends/imgui_impl_vulkan.cpp"
			"${gee_SOURCE_DIR}/libs/imgui/imgui_tables.cpp"
			)

set(stb_files
		"${LIBS_DIR}/stb/imstb_rectpack.h"
		"${LIBS_DIR}/stb/imstb_textedit.h"
		"${LIBS_DIR}/stb/imstb_truetype.h")

set(libs_files ${dear_imgui_files} ${stb_files})
set(sources_impl_vulkan_files
			"sources/impl/vulkan/buffer.cpp"
			"sources/impl/vulkan/commandbuffer.cpp"
			"sources/impl/vulkan/commandpool.cpp"
			"sources/impl/vulkan/debugMessenger.cpp"
			"sources/impl/vulkan/device.cpp"
			"sources/impl/vulkan/deviceMemory.cpp"
			"sources/impl/vulkan/gpu.cpp"
			"sources/impl/vulkan/instance.cpp"
			"sources/impl/vulkan/image.cpp"
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

			)

set(headers_impl_vulkan_files
		"headers/impl/vulkan/buffer.h"
			"headers/impl/vulkan/commandbuffer.h"
			"headers/impl/vulkan/commandpool.h"
			"headers/impl/vulkan/debugMessenger.h"
			"headers/impl/vulkan/device.h"
			"headers/impl/vulkan/deviceMemory.h"
			"headers/impl/vulkan/gpu.h"
			"headers/impl/vulkan/image.h"
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
			"headers/impl/vulkan/sampler.h"
			"headers/impl/vulkan/signal.h"
			"headers/impl/vulkan/shader.h"
			"headers/impl/vulkan/swapchain.h"
			"headers/impl/vulkan/textureImageFactory.h"
			"headers/impl/vulkan/vulkan_utils.h"
			"headers/impl/vulkan/vulkanContext.h"
			"headers/impl/vulkan/vulkanContextBuilder.h")

set(SOURCES
			"sources/application.cpp"
			"sources/camera.cpp"
			"sources/eventDispatcher.cpp"
			"sources/drawable.cpp"
			"sources/geometry.cpp"
			"sources/main.cpp"
			"sources/material.cpp"
			"sources/mesh.cpp"
			"sources/modelLoader.cpp"
			"sources/pass.cpp"
			"sources/optics.cpp"
			"sources/renderer.cpp"
			"sources/renderpass.cpp"
			"sources/resourceHolder.cpp"
			"sources/shaderTechnique.cpp"
			"sources/SpatialTransformation.cpp"
			"sources/timer.cpp"
			"sources/texture.cpp"
			"sources/units.cpp"
			"sources/utility.cpp"
			"sources/window.cpp"

			${sources_impl_vulkan_files}
		)

set(HEADERS
			"headers/application.h"
			"headers/camera.h"
			"headers/drawable.h"
			"headers/enums.h"
			"headers/eventDispatcher.h"
			"headers/geometry.h"
			"headers/material.h"
			"headers/materialInstance.h"
			"headers/mesh.h"
			"headers/modelLoader.h"
			"headers/optics.h"
			"headers/pass.h"
			"headers/renderer.h"
			"headers/renderpass.h"
			"headers/renderTarget.h"
			"headers/resourceHolder.h"
			"headers/sampler.h"
			"headers/samplerLoader.h"
			"headers/shaderInterface.h"
			"headers/SpatialTransformation.h"
			"headers/shaderTechnique.h"
			"headers/texture.h"
			"headers/timer.h"
			"headers/units.h"
			"headers/utility.h"
			"headers/vertex.h"
			"headers/window.h"
			${headers_impl_vulkan_files}

		)
add_executable(gee ${SOURCES} ${HEADERS} ${libs_files})

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

set_target_properties(gee PROPERTIES CXX_STANDARD 20)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources_impl_vulkan_files})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers_impl_vulkan_files})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${libs_files})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${dear_imgui_files})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${stb_files})
