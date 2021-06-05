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
 
	gee::Material phongMaterial{ "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv" };
	gee::Texture floorColorTex{ "../assets/textures/floor.jpg" };
	gee::Texture floorNormalTex{ "../assets/textures/Floor/normal.png" };
	
	gee::MaterialInstance matInstance1{ phongMaterial };
	matInstance1.set_property(gee::TEXTURE_SLOT::COLOR, floorColorTex);
	matInstance1.set_property(gee::TEXTURE_SLOT::NORMAL, floorNormalTex);

	auto cubeGeom = gee::getFloorGeometry();
	gee::Drawable floor{ cubeGeom, matInstance1};

	floor.size = glm::vec3{ 0.5f };
	floor.position += glm::vec3{ 0.0f, 2.0f, 0.0f };
	
	gee::ShaderTechnique phongTechnique{ "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv" };
	
	gee::Pass phongPass{ phongTechnique };
	phongPass.use_screen_target();
	phongPass.add_depth_target(gee::RenderTarget{.size = glm::uvec2{800, 800} }
							);

	gee::Renderpass rp{};
	rp.add_pass(phongPass);
	gee::Application app{ "Graphics's Experimental Engine", 800, 800 };
	app.setCameraPosition(glm::vec3{ 0.0f, 1.0f, 5.0f });
	app.draw(floor);

	while (app.isRunning())
	{
		app.start_renderpass(rp);
			app.use_shader_technique(phongTechnique);
				
	}
	return 0;
}