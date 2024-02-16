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

		GLFWwindow *glfwWindow;

		core::Instance m_instance;
#ifndef NDEBUG
		core::DebugMessenger m_debug_messenger;
#endif

		std::unique_ptr<Surface> surface;

		std::unique_ptr<core::PhysicalDevice> physicalDevice;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

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
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();

		std::vector<const char*> getRequiredExtensions();
		bool isDeviceSuitable(const VkPhysicalDevice& physicalDevice);
		VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice);

	};
}