#include "..\headers\Window.h"
#include <iostream>

gee::Window::Window(const std::string& title, const uint32_t width, const uint32_t height)
{
	/*
	if (glfwVulkanSupported())
	{
		std::cout << "vulkan supported\n";
	}
	else
	{
		std::cout << "vulkan is not supported \n";
	}*/
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	aspectRatio_ = width / static_cast<float>(height);
	isVisible_ = width != 0 && height != 0;
}

const bool gee::Window::isOpen() const
{
	glfwPollEvents();
	return !glfwWindowShouldClose(window_);
}

GLFWwindow* gee::Window::window() const
{
	return window_;
}

void gee::Window::setTitle(const std::string& title)
{
	glfwSetWindowTitle(window_, title.c_str());
}

void gee::Window::resize()
{
	const auto& sz = size();
	aspectRatio_ = sz.x / static_cast<float>(sz.y);
	isVisible_ = sz.x != 0 && sz.y != 0;
}

const glm::u32vec2 gee::Window::size() const
{
	int width{}, height{};
	glfwGetWindowSize(window_, &width, &height);
	return {width, height};
}

const float gee::Window::aspectRatio() const
{
	return aspectRatio_;
}

const bool gee::Window::isVisible() const
{
	return isVisible_;
}
