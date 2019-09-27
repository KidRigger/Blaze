﻿// Blaze.cpp : Defines the entry point for the application.
//

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Blaze.hpp"
#include "util/Managed.hpp"
#include "util/DeviceSelection.hpp"
#include "util/createFunctions.hpp"
#include "Context.hpp"
#include "Renderer.hpp"
#include "Datatypes.hpp"
#include "VertexBuffer.hpp"
#include "Texture.hpp"
#include "Model.hpp"
#include "Camera.hpp"

#include <optional>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VALIDATION_LAYERS_ENABLED

#ifdef NDEBUG
#ifdef VALIDATION_LAYERS_ENABLED
#undef VALIDATION_LAYERS_ENABLED
#endif
#endif

namespace blaze
{
	// Constants
	const int WIDTH = 800;
	const int HEIGHT = 600;

#ifdef VALIDATION_LAYERS_ENABLED
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

	bool firstMouse = true;
	double lastX = 0.0;
	double lastY = 0.0;

	double yaw = -90.0;
	double pitch = 0.0;

	glm::vec3 cameraFront{ 0.0f, 0.0f, 1.0f };

	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		double sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}

	void run()
	{
		// Usings
		using namespace std;
		using namespace util;

		// Variables
		GLFWwindow* window = nullptr;
		Renderer renderer;
		IndexedVertexBuffer<Vertex> vertexBuffer;
		TextureImage image;

		Camera cam({ 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, -4.0f }, { 0.0f, 1.0f, 0.0f }, glm::radians(45.0f), 4.0f / 3.0f, 1.0f, 1000.0f);

		// GLFW Setup
		assert(glfwInit());

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_CURSOR_HIDDEN, GLFW_TRUE);
		glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Hello, Vulkan", nullptr, nullptr);
		assert(window != nullptr);

		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		renderer = Renderer(window);
		if (!renderer.complete())
		{
			throw std::runtime_error("Renderer could not be created");
		}

		auto model = loadModel(renderer, "assets/spheres/MetalRoughSpheres.gltf");

		// Run
		bool onetime = true;

		double prevTime = glfwGetTime();
		double deltaTime = 0.0;
		int intermittence = 0;
		double elapsed = 0.0;

		while (!glfwWindowShouldClose(window))
		{
			prevTime = glfwGetTime();
			glfwPollEvents();
			elapsed += deltaTime;

			{
				if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				{
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}
				glm::vec3 cameraPos(0.f);
				float cameraSpeed = 0.005f; // adjust accordingly
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					cameraPos += cameraSpeed * cameraFront;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					cameraPos -= cameraSpeed * cameraFront;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					cameraPos -= glm::normalize(glm::cross(cameraFront, cam.getUp())) * cameraSpeed;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					cameraPos += glm::normalize(glm::cross(cameraFront, cam.getUp())) * cameraSpeed;
				cam.moveBy(cameraPos);
			}
			cam.lookTo(cameraFront);

			try
			{
				model.update();
				renderer.set_cameraUBO(cam.getUbo());
				renderer.renderFrame();
				if (onetime) 
				{
					// renderer.submit(renderCommand);
					renderer.submit([&model](VkCommandBuffer cmdBuffer, VkPipelineLayout layout) { model.draw(cmdBuffer, layout); });
					onetime = false;
				}
			}
			catch (std::exception& e)
			{
				cerr << e.what() << endl;
			}

			deltaTime = glfwGetTime() - prevTime;
			printf("\r%.4lf", deltaTime);
		}

		cout << endl;

		// Wait for all commands to finish
		vkDeviceWaitIdle(renderer.get_device());

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}

int main(int argc, char* argv[])
{
	blaze::run();

	return 0;
}
