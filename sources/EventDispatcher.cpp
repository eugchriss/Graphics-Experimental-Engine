#include <assert.h>
#include "../headers/EventDispatcher.h"
#ifndef NDEBUG
uint8_t gee::EventDispatcher::count_{ 0 };
#endif // !NDEBUG

gee::EventDispatcher::EventDispatcher(GLFWwindow* window)
{
#ifndef NDEBUG
	++count_; 
	assert(count_ < 2 && "You can t have 2 differents event dispatchers");
#endif // !NDEBUG

	glfwSetFramebufferSizeCallback(window, onWindowResizeCallback);
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);
	glfwSetCursorPosCallback(window, onMouseMoveCallback);
	glfwSetScrollCallback(window, onMouseScrollCallback);

	glfwSetWindowUserPointer(window, this);
}

gee::EventDispatcher::~EventDispatcher()
{
#ifndef NDEBUG
	--count_;  //basic singleton
#endif // !NDEBUG
}

void gee::EventDispatcher::addWindowResizeCallback(std::function<void(const uint32_t, const uint32_t)> callback)
{
	resizeCallbacks_.append(callback);
}

void gee::EventDispatcher::addMouseButtonCallback(std::function<void(const uint32_t, const uint32_t, const uint32_t)> callback)
{
	mouseButtonCallbacks_.append(callback);
}

void gee::EventDispatcher::addMouseMoveCallback(std::function<void(const double, const double)> callback)
{
	mouseMoveCallbacks_.append(callback);
}

void gee::EventDispatcher::addMouseScrollCallback(std::function<void(const double, const double)> callback)
{
	mouseScrollCallbacks_.append(callback);
}

void gee::EventDispatcher::onWindowResizeCallback(GLFWwindow* window, int width, int height)
{
	auto dispatcher = static_cast<gee::EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->resizeCallbacks_(width, height);
}

void gee::EventDispatcher::onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto dispatcher = static_cast<gee::EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->mouseButtonCallbacks_(button, action, mods);
}

void gee::EventDispatcher::onMouseMoveCallback(GLFWwindow* window, double x, double y)
{
	auto dispatcher = static_cast<gee::EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->mouseMoveCallbacks_(x, y);
}

void gee::EventDispatcher::onMouseScrollCallback(GLFWwindow* window, double x, double y)
{
	auto dispatcher = static_cast<gee::EventDispatcher*>(glfwGetWindowUserPointer(window));
	dispatcher->mouseScrollCallbacks_(x, y);
}