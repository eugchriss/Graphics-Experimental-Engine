#include <iostream>
#include <vector>

#include "../headers/Application.h"
#include "../headers/Drawable.h"
#include "../headers/Renderpass.h"
#include "../headers/utility.h"
#include "../headers/material.h"

int main()
{
	glfwInit();
	std::string s{ "Graphics's Experimental Engine" };
	gee::Application app{ s, 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });

	while (app.isRunning())
	{

	}

	//std::cin.get();
	return 0;
}