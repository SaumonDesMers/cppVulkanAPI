#pragma once

#include "defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace LIB_NAMESPACE
{
	class Surface
	{

	public:

		Surface(VkInstance instance, GLFWwindow* window);
		~Surface();

		VkSurfaceKHR getVk() const { return m_surface; }

	private:

		VkSurfaceKHR m_surface;
		VkInstance m_instance;
	};
}