#include "surface.hpp"

#include <stdexcept>

namespace LIB_NAMESPACE
{
	Surface::Surface(VkInstance instance, GLFWwindow* window)
		: m_instance(instance)
	{
		if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface.");
		}
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
}