#pragma once

#include "defines.hpp"
#include "window/surface.hpp"
#include "core/instance/instance.hpp"
#include "core/instance/instance_create_info.hpp"
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

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
		};

		Surface & surface() { return m_surface; }
		const Surface & surface() const { return m_surface; }

		core::PhysicalDevice & physicalDevice() { return m_physical_device; }
		const core::PhysicalDevice & physicalDevice() const { return m_physical_device; }

		GLFWwindow *glfwWindow;

		core::Instance m_instance;
#ifndef NDEBUG
		core::DebugMessenger m_debug_messenger;
#endif

		Surface m_surface;

		core::PhysicalDevice m_physical_device;

		std::unique_ptr<core::Device> device;
		std::unique_ptr<core::Queue> graphicsQueue;
		std::unique_ptr<core::Queue> presentQueue;


		Device(GLFWwindow *glfwWindow);
		~Device();

		Swapchain::SupportDetails querySwapChainSupport(const VkPhysicalDevice& device);
		Queue::FamilyIndices findQueueFamilies(const VkPhysicalDevice& physicalDevice);

	private:

		core::InstanceCreateInfo instanceCreateInfo();
		core::DebugMessengerCreateInfo debugMessengerCreateInfo();
		VkPhysicalDevice pickPhysicalDevice();
		void createLogicalDevice();

		std::vector<const char*> getRequiredExtensions();
		bool isDeviceSuitable(const VkPhysicalDevice& physicalDevice);
		VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice);

	};
}