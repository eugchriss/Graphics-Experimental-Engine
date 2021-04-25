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
	auto& cubeGeometry = app.get_geometry("cube", gee::getCubeMesh());
	
	auto& materialInstance = app.get_materialInstance(phongMaterial);
	materialInstance.set_property(TEXTURE_SLOT::COLOR, floorColorTex);
	materialInstance.set_property(TEXTURE_SLOT::NORMAL, floorNormalTex);

	gee::Drawable cube{ cubeGeometry, materialInstance};
	app.add_drawable(cube);
	while (app.isRunning())
	{

	}
	return 0;
}