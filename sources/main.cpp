#include <iostream>
#include <vector>
#include "../headers/Application.h"
#include "../headers/Drawable.h"

int main()
{
	glfwInit();
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });

	auto& phongMaterial = app.get_material("phong material", "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv");
	auto& floorColorTex = app.load_texture("floor_color", "../assets/textures/floor.jpg");
	auto& floorNormalTex = app.load_texture("floor_normal", "../assets/textures/Floor/normal.png");
	auto& floorSpecTex = app.load_texture("floor_spec", "../assets/textures/Floor/specular.jpg");
	auto& cubeGeometry = app.get_geometry("cube", gee::getCubeMesh());
	
	auto& materialInstance = app.get_materialInstance(phongMaterial);
	materialInstance.set_property(TEXTURE_SLOT::COLOR, floorNormalTex);

	auto& materialInstance2 = app.get_materialInstance(phongMaterial);
	materialInstance2.set_property(TEXTURE_SLOT::COLOR, floorColorTex);
	
	auto& materialInstance3 = app.get_materialInstance(phongMaterial);
	materialInstance3.set_property(TEXTURE_SLOT::COLOR, floorSpecTex);


	gee::Drawable cube{ cubeGeometry, materialInstance};
	gee::Drawable cube2{ cubeGeometry, materialInstance2};
	gee::Drawable cube3{ cubeGeometry, materialInstance3};
	app.add_drawable(cube);
	app.add_drawable(cube2);
	app.add_drawable(cube3);

	cube.size = glm::vec3{ 0.5f };
	cube2.size = glm::vec3{ 0.5f };
	cube.position += glm::vec3{ 0.0f, 2.0f, 0.0f };
	cube2.position += glm::vec3{ 0.0f, -2.0f, 0.0f };
	cube3.position += glm::vec3{ -2.0f, 0.0f, 0.0f };
	while (app.isRunning())
	{
	}
	return 0;
}