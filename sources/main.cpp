#include <iostream>
#include <vector>
#include "../headers/Application.h"

int main()
{
	glfwInit();
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });
	while (app.isRunning())
	{

	}
	return 0;
}