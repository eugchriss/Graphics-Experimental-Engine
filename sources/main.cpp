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
	gee::Texture floorSpecularTex{ "../assets/textures/Floor/specular.jpg" };
	
	gee::Material floorMaterial{};
	floorMaterial.set_property(gee::MaterialProperty::COLOR, floorTex);
	floorMaterial.set_property(gee::MaterialProperty::NORMAL, floorNormalTex);

	gee::Material floorNormalMaterial{};
	floorNormalMaterial.set_property(gee::MaterialProperty::COLOR, floorNormalTex);
	floorNormalMaterial.set_property(gee::MaterialProperty::NORMAL, floorTex);

	gee::Material floorSpecularMaterial{};
	floorSpecularMaterial.set_property(gee::MaterialProperty::COLOR, floorSpecularTex);
	floorSpecularMaterial.set_property(gee::MaterialProperty::NORMAL, floorTex);

	auto cubeGeometry = gee::getCubeGeometry();
	gee::Drawable cube{cubeGeometry, floorMaterial};
	gee::Drawable cube2{ cubeGeometry, floorNormalMaterial };
	gee::Drawable cube4{ cubeGeometry, floorNormalMaterial };
	gee::Drawable cube3{ cubeGeometry, floorSpecularMaterial };
	gee::Drawable cube5{ cubeGeometry, floorSpecularMaterial };
	cube2.position -= 10.0f;
	cube4.position -= 5.0f;
	cube3.position += 10.0f;
	cube5.position += 5.0f;
	do
	{
		app.draw(cube);
		app.draw(cube2);
		app.draw(cube4);
		app.draw(cube3);
		app.draw(cube5);
	} while (app.isRunning());

	//std::cin.get();
	return 0;
}