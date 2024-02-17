#include "device.hpp"

#include <stdexcept>
#include <set>
#include <iostream>

namespace LIB_NAMESPACE
{
	Device::Device(GLFWwindow *glfwWindow):
		glfwWindow(glfwWindow),
		m_instance(validation_layers, getRequiredExtensions()),
		#ifndef NDEBUG
			m_debug_messenger(m_instance.getVk()),
		#endif
		m_surface(m_instance.getVk(), glfwWindow),
		m_physical_device(m_instance.getVk(), device_extensions, m_surface.getVk()),
		m_device(m_physical_device, validation_layers, device_extensions),
		m_graphicsQueue(m_device.getVk(), m_physical_device.queueFamilyIndices().graphicsFamily.value()),
		m_presentQueue(m_device.getVk(), m_physical_device.queueFamilyIndices().presentFamily.value())
	{

	}

	Device::~Device()
	{

	}


	std::vector<const char*> Device::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		#ifndef NDEBUG
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif

		return extensions;
	}

	Swapchain::SupportDetails Device::querySwapChainSupport(const VkPhysicalDevice & physical_device)
	{
		return core::PhysicalDevice::querySwapChainSupport(physical_device, m_surface.getVk());
	}

}