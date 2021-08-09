#include <iostream>
#include <vector>

#include "../headers/Application.h"
#include "../headers/Drawable.h"
#include "../headers/material.h"
#include "../headers/Renderpass.h"
#include "../headers/staticMesh.h"
#include "../headers/Timer.h"
#include "../headers/utility.h"

int main()
{
	glfwInit();
	gee::ResourceHolder<gee::Texture> textureHolder{};
	gee::ResourceHolder<gee::StaticMesh> meshHolder{};
	std::string s{ "Graphics's Experimental Engine" };
	gee::Application app{ s, 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });

	auto& floorTex = textureHolder.get("floorTexture", "../assets/textures/floor.jpg");
	auto& floorNormalTex = textureHolder.get("normalTexture", "../assets/textures/Floor/normal.png");
	auto& floorSpecularTex = textureHolder.get("specularTexture", "../assets/textures/Floor/specular.jpg");

	const auto cubeCount = 3u;
	std::vector<gee::Drawable> cubes;
	cubes.reserve(cubeCount);

	gee::Material floorMaterial{};
	floorMaterial.set_property(gee::MaterialProperty::COLOR, floorTex);
	floorMaterial.set_property(gee::MaterialProperty::NORMAL, floorNormalTex);

	gee::StaticMesh cubeMesh{ gee::getCubeGeometry(), std::move(floorMaterial) };
	auto& backpackMesh = meshHolder.get("backpack", textureHolder, "../assets/Meshs/backpack/backpack.obj");
	for (auto i = 0; i < cubeCount; ++i)
	{
		auto& cube = cubes.emplace_back(backpackMesh);
		cube.position.z = -50;
	}

	int relativeAngle = 0;
	bool isRunning{ true };
	gee::Timer timer;

	auto radius = 5;
	while (isRunning)
	{
		timer.set_timestamp();
		for (auto i = 0; i < cubeCount; ++i)
		{
			auto& cube = cubes[i];
			auto angle = (relativeAngle + i * 360 / cubeCount) % 360;
			cube.position.x = radius * cos(glm::radians(static_cast<float>(angle)));
			cube.position.y = radius * sin(glm::radians(static_cast<float>(angle)));
			app.draw(cube);
		}
		isRunning = app.isRunning();
		relativeAngle += 360.0 * timer.ellapsed_time_s();
		relativeAngle %= 360;
	}
	return 0;
}