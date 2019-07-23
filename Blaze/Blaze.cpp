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

#include <optional>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

	void run()
	{
		// Usings
		using namespace std;
		using namespace util;

		// Variables
		GLFWwindow* window = nullptr;
		Context ctx;
		Renderer renderer;
		VertexBuffer vertexBuffer;

		// GLFW Setup
		assert(glfwInit());

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Hello, Vulkan", nullptr, nullptr);
		assert(window != nullptr);

		renderer = Renderer(window);
		if (!renderer.complete())
		{
			throw std::runtime_error("Renderer could not be created");
		}

		vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		vertexBuffer = VertexBuffer(renderer, sizeof(vertices[0]) * vertices.size());
		vertexBuffer.map(renderer.get_device(), vertices);

		auto renderCommand = [buf = vertexBuffer.get_buffer(), size = vertexBuffer.get_size()](VkCommandBuffer cmdBuffer)
		{
			VkBuffer buffers[] = { buf };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
			vkCmdDraw(cmdBuffer, static_cast<uint32_t>(size), 1, 0, 0);
		};

		// Run
		bool ontime = true;

		double prevTime = glfwGetTime();
		int intermittence = 0;

		while (!glfwWindowShouldClose(window))
		{
			prevTime = glfwGetTime();
			glfwPollEvents();

			try
			{
				renderer.renderFrame();
				if (ontime) 
				{
					renderer.submit(renderCommand);
					ontime = false;
				}
			}
			catch (std::exception& e)
			{
				cerr << e.what() << endl;
			}

			printf("\r%.4lf", (glfwGetTime() - prevTime));
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
