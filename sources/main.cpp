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

	gee::Texture floorTex{ "../assets/textures/floor.jpg" };
	gee::Texture floorNormalTex{ "../assets/textures/Floor/normal.png" };
	gee::Material floorMaterial{};
	floorMaterial.set_property(gee::MaterialProperty::COLOR, floorTex);
	floorMaterial.set_property(gee::MaterialProperty::NORMAL, floorNormalTex);

	auto cubeGeometry = gee::getCubeGeometry();
	gee::Drawable cube{cubeGeometry, floorMaterial};
	do
	{
		app.draw(cube);
	} while (app.isRunning());

	//std::cin.get();
	return 0;
}