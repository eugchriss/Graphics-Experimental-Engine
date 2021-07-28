#include <iostream>
#include <vector>

#include "../headers/Application.h"
#include "../headers/Drawable.h"
#include "../headers/material.h"
#include "../headers/Renderpass.h"
#include "../headers/Timer.h"
#include "../headers/utility.h"

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
	const auto cubeCount = 3u;
	std::vector<gee::Drawable> cubes;
	cubes.reserve(cubeCount);
	auto pi = glm::pi<float>();
	auto radius = 10;
	cubes.emplace_back(cubeGeometry, floorMaterial);
	cubes.emplace_back(cubeGeometry, floorNormalMaterial);
	cubes.emplace_back(cubeGeometry, floorSpecularMaterial);
	for (auto i = 0; i < cubeCount; ++i)
	{
		auto& cube = cubes[i];
		cube.position.x = radius * cos(i * 2 * pi / cubeCount);
		cube.position.y = radius * sin(i * 2 * pi / cubeCount);
		cube.position.z = -50;

	}

	int angle = 0;
	bool isRunning{ true };
	gee::Timer timer;
	while (isRunning)
	{
		timer.set_timestamp();
		for (auto i = 0; i < cubeCount; ++i)
		{
			auto& cube = cubes[i];
			auto relativeAngle = (angle + i * 360 / cubeCount) % 360;
			cube.position.x = radius * cos(glm::radians(static_cast<float>(relativeAngle)));
			cube.position.y = radius * sin(glm::radians(static_cast<float>(relativeAngle)));
			app.draw(cube);
		}
		isRunning = app.isRunning();
		angle += 360.0 * timer.ellapsed_time_s();
		angle %= 360;
	}

	return 0;
}