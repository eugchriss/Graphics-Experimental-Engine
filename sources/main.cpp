#include <iostream>
#include <vector>
#include "../headers/Application.h"
#include "../headers/ResourceHolder.h"
#include "../headers/Drawable.h"
#include "../headers/utility.h"

int main()
{
	glfwInit();
	
	ResourceHolder<gee::Mesh> meshHolder;
	auto& cubeMesh = meshHolder.get("box", "../assets/Meshs/box/box.obj");
	auto& backpackMesh = meshHolder.get("backpack", "../assets/Meshs/backpack/backpack.obj");
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };

	gee::Drawable cube{"cube 0", cubeMesh };
	gee::Drawable cube2{"cube 1", cubeMesh, glm::vec3{-10.0f, 0.0f, 0.0f} };
	gee::Drawable backpack{"backpack", backpackMesh, glm::vec3{10.0f, -10.0f, 10.0f} };
	gee::Drawable backpack2{"backpack2", backpackMesh, glm::vec3{10.0f, 10.0f, 10.0f} };
	
	gee::Optics pointLight{ gee::Light::PointLight };
	pointLight.diffuse = glm::vec3{ 1.0f, 0.0f, 0.0f };
	gee::Drawable light{ "light 0", cubeMesh, pointLight };
	light.size = glm::vec3{ 0.2f, 0.2f, 0.2f };
	cube.size = glm::vec3{ 0.1f };
	cube2.size = glm::vec3{ 0.1f };
	backpack.size = glm::vec3{ 1.0f };
	backpack2.size = glm::vec3{ 1.0f };
	app.addDrawable(cube);
	app.addDrawable(cube2);
	app.addDrawable(backpack);
	app.addDrawable(backpack2);
	app.addDrawable(light);

	gee::Mesh floorMesh{ "floorMesh", std::move(gee::Floor{}.vertices_), std::move(gee::Floor{}.indices_), gee::Material{"../assets/textures/floor/diffuse.jpg", "../assets/textures/floor/normal.png"} };
	gee::Drawable floor{ "floor", floorMesh };
	floor.size = glm::vec3{ 10.0f };
	app.addDrawable(floor);
	
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });
	while (app.isRunning())
	{

	}
	return 0;
}