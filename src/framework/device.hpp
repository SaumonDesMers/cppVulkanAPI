#pragma once

#include "defines.hpp"
#include "window/surface.hpp"
#include "core/instance/instance.hpp"
#include "core/debug.hpp"
#include "core/physical_device.hpp"
#include "core/logical_device.hpp"
#include "swapchain.hpp"
#include "queue.hpp"

#include <memory>
#include <optional>

namespace LIB_NAMESPACE
{

	class Device
	{

	public:

		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
		};

		Surface & surface() { return m_surface; }
		const Surface & surface() const { return m_surface; }

		core::PhysicalDevice & physicalDevice() { return m_physical_device; }
		const core::PhysicalDevice & physicalDevice() const { return m_physical_device; }

		core::Device & device() { return m_device; }
		const core::Device & device() const { return m_device; }

		core::Queue & graphicsQueue() { return m_graphicsQueue; }
		const core::Queue & graphicsQueue() const { return m_graphicsQueue; }

		core::Queue & presentQueue() { return m_presentQueue; }
		const core::Queue & presentQueue() const { return m_presentQueue; }

		Device(GLFWwindow *glfwWindow);
		~Device();

		Swapchain::SupportDetails querySwapChainSupport(const VkPhysicalDevice&  physical_device);

		GLFWwindow *glfwWindow;

	private:

		core::Instance m_instance;
#ifndef NDEBUG
		core::DebugMessenger m_debug_messenger;
#endif

		Surface m_surface;

		core::PhysicalDevice m_physical_device;

		core::Device m_device;
		core::Queue m_graphicsQueue;
		core::Queue m_presentQueue;

		std::vector<const char*> getRequiredExtensions();

	};
}