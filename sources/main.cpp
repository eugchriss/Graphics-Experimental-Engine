#include <iostream>
#include <vector>
#include "../headers/Application.h"
#include "../headers/ResourceHolder.h"
#include "../headers/Drawable.h"
#include "../headers/utility.h"
#include "../headers/ModelLoader.h"

int main()
{

	glfwInit();
	gee::ModelLoader loader{};
	gee::ResourceHolder<gee::ModelLoader, gee::Mesh> meshHolder{ gee::ModelLoader{} };
	auto& cubeMesh = meshHolder.get("box", "../assets/Meshs/box/box.obj");

	auto& backpackMesh = meshHolder.get("backpack", "../assets/Meshs/backpack/backpack.obj");

	std::array<std::string, 6> skyboxPaths{ "../assets/skybox/space/right.png",
											"../assets/skybox/space/left.png",
											"../assets/skybox/space/top.png",
											"../assets/skybox/space/bottom.png",
											"../assets/skybox/space/front.png",
											"../assets/skybox/space/back.png" };
	auto& skyboxMesh = meshHolder.get("skybox", gee::getCubeMesh, gee::Material{ skyboxPaths });
	
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };
	
	gee::Optics pointLight{ gee::Light::PointLight };
	pointLight.diffuse = glm::vec3{ 1.0f, 0.0f, 0.0f };
	pointLight.linear = 0.7f;
	pointLight.quadratic = 1.8f;
	gee::Drawable cube{ "cube 0", cubeMesh, pointLight };
	cube.setPosition(glm::vec3{ 0.0f, 2.0f, 0.0f });
	gee::Drawable cube2{ "cube 1", cubeMesh, glm::vec3{-10.0f, 0.0f, 0.0f} };
	gee::Drawable backpack{ "backpack", backpackMesh, glm::vec3{10.0f, -10.0f, 10.0f} };
	gee::Drawable backpack2{ "backpack2", backpackMesh, glm::vec3{10.0f, 10.0f, 10.0f} };
	gee::Optics l{ gee::Light::PointLight };
	gee::Drawable light{ "light", cubeMesh, l };
	light.setPosition(glm::vec3{ -10.0f });

	cube.setSize(glm::vec3{ 0.1f });
	cube2.setSize(glm::vec3{ 0.1f });
	backpack.setSize(glm::vec3{ 1.0f });
	backpack2.setSize(glm::vec3{ 1.0f });
	app.addDrawable(cube);
	app.addDrawable(cube2);
	app.addDrawable(backpack);
	app.addDrawable(backpack2);
	app.addDrawable(light);

	gee::Drawable skybox{ "skybox", skyboxMesh };
	auto& floorMesh = gee::getFloorMesh(gee::Material{ "../assets/textures/floor/diffuse.jpg", "../assets/textures/floor/normal.png" });
	gee::Drawable floor{ "floor", floorMesh };
	floor.setSize(glm::vec3{ 10.0f });
	app.addDrawable(floor);

	app.setSkybox(skybox);
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });
	while (app.isRunning())
	{

	}
	return 0;
}