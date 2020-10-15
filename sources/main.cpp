#include <iostream>
#include <vector>
#include "../headers/Application.h"
#include "../headers/ResourceHolder.h"
#include "../headers/Drawable.h"

int main()
{
	glfwInit();
	ResourceHolder<gee::Mesh> meshHolder;
	auto& cubeMesh = meshHolder.get("box", "../assets/Meshs/box/box.obj");
	auto& speakerMesh = meshHolder.get("speaker", "../assets/Meshs/speaker/scene.gltf");
	auto& backpackMesh = meshHolder.get("headphone", "../assets/Meshs/backpack/backpack.obj");
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };

	gee::Drawable cube{"cube 0", cubeMesh };
	gee::Drawable cube2{"cube 1", cubeMesh, glm::vec3{-100.0f, 0.0f, 0.0f} };
	gee::Drawable speaker{"speaker", speakerMesh, glm::vec3{000.0f, 20.0f, 50.0f} };
	gee::Drawable backpack{"backpack", backpackMesh, glm::vec3{20.0f, -20.0f, 50.0f} };

	app.addDrawable(cube);
	app.addDrawable(speaker);
	app.addDrawable(backpack);
	app.addDrawable(cube2);
	while (app.isRunning())
	{

	}
	return 0;
}