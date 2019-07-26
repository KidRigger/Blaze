
#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace blaze::util
{
	VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& shaderCode);

	VkSemaphore createSemaphore(VkDevice device);

	VkFence createFence(VkDevice device);

	std::tuple<VkBuffer, VkDeviceMemory> createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}