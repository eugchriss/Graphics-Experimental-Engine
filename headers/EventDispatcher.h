#pragma once
#include <functional>

#include "eventpp/callbacklist.h"
#include "GLFW/glfw3.h"

namespace gee
{
	//Is extremly dependent on glfw for now
	class EventDispatcher
	{
	public: 
		EventDispatcher(GLFWwindow* window);
		~EventDispatcher();
		EventDispatcher(EventDispatcher&&) = delete;
		EventDispatcher(const EventDispatcher&) = delete;

		void addWindowResizeCallback(std::function<void(const uint32_t, const uint32_t)> callback);
		void addMouseButtonCallback(std::function<void(const uint32_t, const uint32_t, const uint32_t)> callback);
		void addMouseMoveCallback(std::function<void(const double, const double)> callback);
		void addMouseScrollCallback(std::function<void(const double, const double)> callback);
	private:
		eventpp::CallbackList<void(const uint32_t, const uint32_t)> resizeCallbacks_;
		eventpp::CallbackList<void(const uint32_t, const uint32_t, const uint32_t)> mouseButtonCallbacks_;
		eventpp::CallbackList<void(const double, const double)> mouseMoveCallbacks_;
		eventpp::CallbackList<void(const double, const double)> mouseScrollCallbacks_;

#ifndef NDEBUG
		static uint8_t count_;  //basic singleton
#endif // !NDEBUG

		static void onWindowResizeCallback(GLFWwindow* window, int width, int height);
		static void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void onMouseMoveCallback(GLFWwindow* window, double x, double y);
		static void onMouseScrollCallback(GLFWwindow* window, double x, double y);
	};

}
