#include <iostream>
#include <vector>

#include "../headers/Application.h"
#include "../headers/Drawable.h"
#include "../headers/utility.h"
#include "../headers/material.h"

int main()
{
	glfwInit();
 
	gee::Material phongMaterial{ "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv" };
	gee::Texture floorColorTex{ "../assets/textures/floor.jpg" };
	gee::Texture floorNormalTex{ "../assets/textures/Floor/normal.png" };
	gee::Texture floorSpecTex{ "../assets/textures/Floor/specular.jpg" };
	
	gee::MaterialInstance matInstance1{ phongMaterial };
	matInstance1.set_property(gee::TEXTURE_SLOT::COLOR, floorColorTex);

	gee::MaterialInstance matInstance2{ phongMaterial };
	matInstance2.set_property(gee::TEXTURE_SLOT::COLOR, floorNormalTex);

	gee::MaterialInstance matInstance3{ phongMaterial };
	matInstance3.set_property(gee::TEXTURE_SLOT::COLOR, floorSpecTex);

	auto& cubeGeom = gee::getCubeGeometry();
	gee::Drawable cube{ cubeGeom, matInstance1};
	gee::Drawable cube2{ cubeGeom, matInstance2 };
	gee::Drawable cube3{ cubeGeom, matInstance3 };

	cube.size = glm::vec3{ 0.5f };
	cube2.size = glm::vec3{ 0.5f };
	cube.position += glm::vec3{ 0.0f, 2.0f, 0.0f };
	cube2.position += glm::vec3{ 0.0f, -2.0f, 0.0f };
	cube3.position += glm::vec3{ -2.0f, 0.0f, 0.0f };
	
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });
	app.draw(cube);
	app.draw(cube2);
	app.draw(cube3);

	while (app.isRunning())
	{
	}
	return 0;
}