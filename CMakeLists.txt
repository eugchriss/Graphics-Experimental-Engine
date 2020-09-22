cmake_minimum_required(VERSION 3.0)

project(gee LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKEBUILD_TYPE})	
set(LIBS_DIR "${gee_SOURCE_DIR}/libs")

find_package(Vulkan REQUIRED FATAL_ERROR)

add_subdirectory(${LIBS_DIR})

add_executable(gee "sources/main.cpp")

target_include_directories(gee PRIVATE ${LIBS_DIR}/assimp)
target_include_directories(gee PUBLIC ${LIBS_DIR}/glm)
target_include_directories(gee PRIVATE ${LIBS_DIR}/imgui)
target_include_directories(gee PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(gee assimp)
target_link_libraries(gee glfw)
target_link_libraries(gee ${Vulkan_LIBRARY})