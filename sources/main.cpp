#include <iostream>
#include <vector>
#include "../headers/Application.h"
#include "../headers/ResourceHolder.h"
#include "../headers/Drawable.h"
#include "../headers/PointLight.h"

int main()
{
	glfwInit();
	ResourceHolder<gee::Mesh> meshHolder;
	auto& cubeMesh = meshHolder.get("box", "../assets/Meshs/box/box.obj");
	auto& backpackMesh = meshHolder.get("headphone", "../assets/Meshs/backpack/backpack.obj");
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };

	gee::Drawable cube{"cube 0", cubeMesh };
	gee::Drawable cube2{"cube 1", cubeMesh, glm::vec3{-10.0f, 0.0f, 0.0f} };
	gee::Drawable backpack{"backpack", backpackMesh, glm::vec3{10.0f, -10.0f, 10.0f} };
	gee::Drawable backpack2{"backpack2", backpackMesh, glm::vec3{10.0f, 10.0f, 10.0f} };

	gee::PointLight light{ "light 0", cubeMesh };
	light.diffuse = glm::vec3{ 1.0f, 0.0f, 0.0f };
	light.size = glm::vec3{ 0.2f, 0.2f, 0.2f };
	app.addDrawable(cube);
	app.addDrawable(backpack);
	app.addDrawable(cube2);
	app.addDrawable(backpack2);
	app.addDrawable(backpack2);
	app.addLight(light);
	while (app.isRunning())
	{

	}
	return 0;
}