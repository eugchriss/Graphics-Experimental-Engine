#pragma once
#include <string>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

namespace gee
{
	class Window
	{
	public:
		Window(const std::string& title, const uint32_t width, const uint32_t height);
		virtual ~Window() = default;
		const bool isOpen() const;
		GLFWwindow* window() const;
		void setTitle(const std::string& title);
		void resize();
		const glm::u32vec2 size() const;
		const float aspectRatio() const;
		const bool isVisible() const;

	private:
		GLFWwindow* window_{};
		float aspectRatio_{};
		bool isVisible_{ false };
	};
}