#include "device.hpp"

#include <stdexcept>
#include <set>
#include <iostream>

namespace LIB_NAMESPACE
{
	Device::Device(GLFWwindow *glfwWindow)
		: glfwWindow(glfwWindow)
	{
		// createWindow();
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	Device::~Device()
	{

	}

	
	void Device::createWindow()
	{
		// windowManager = std::make_unique<Window::Manager>();
		// window = windowManager->createWindow("Master window", 800, 600);
	}

	void Device::createInstance()
	{
		if (enableValidationLayers && vk::core::Instance::checkValidationLayerSupport(validationLayers) == false)
		{
			throw std::runtime_error("Validation layers requested, but not available");
		}

		vk::core::ApplicationInfo appInfo = {};
		appInfo.pApplicationName = "Playground";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Playground Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		vk::core::Instance::CreateInfo createInfo = {};
		createInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> extensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		vk::core::DebugMessenger::CreateInfo debugCreateInfo = {};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		instance = std::make_unique<vk::core::Instance>(createInfo);
	}

	void Device::setupDebugMessenger()
	{
		if (enableValidationLayers == false)
		{
			return;
		}

		vk::core::DebugMessenger::CreateInfo createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		debugMessenger = std::make_unique<vk::core::DebugMessenger>(instance->getVk(), createInfo);
	}

	void Device::createSurface()
	{
		surface = std::make_unique<vk::Surface>(instance->getVk(), glfwWindow);
	}

	void Device::pickPhysicalDevice()
	{
		std::vector<VkPhysicalDevice> physicalDevices = instance->getPhysicalDevices();

		if (physicalDevices.empty())
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support");
		}

		VkPhysicalDevice pickedPhysicalDevice = VK_NULL_HANDLE;
		for (const auto& phyDev : physicalDevices)
		{
			if (isDeviceSuitable(phyDev))
			{
				pickedPhysicalDevice = phyDev;
				msaaSamples = VK_SAMPLE_COUNT_1_BIT;
				break;
			}
		}

		if (pickedPhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU");
		}

		physicalDevice = std::make_unique<vk::core::PhysicalDevice>(pickedPhysicalDevice);
	}

	void Device::createLogicalDevice()
	{
		Queue::FamilyIndices indices = findQueueFamilies(physicalDevice->getVk());

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			vk::core::Queue::CreateInfo queueInfo = {};
			queueInfo.queueFamilyIndex = queueFamily;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueInfos.push_back(queueInfo);
		}

		vk::core::PhysicalDevice::Features deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		vk::core::Device::CreateInfo deviceInfo = {};
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.pEnabledFeatures = &deviceFeatures;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

		deviceInfo.pNext = &dynamicRenderingFeatures;

		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers)
		{
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceInfo.enabledLayerCount = 0;
		}

		device = std::make_unique<vk::core::Device>(physicalDevice->getVk(), deviceInfo);

		graphicsQueue = std::make_unique<vk::core::Queue>(device->getVk(), indices.graphicsFamily.value());
		presentQueue = std::make_unique<vk::core::Queue>(device->getVk(), indices.presentFamily.value());
	}


	std::vector<const char*> Device::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Device::populateDebugMessengerCreateInfo(vk::core::DebugMessenger::CreateInfo& createInfo)
	{
		createInfo.messageSeverity =
			// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pUserData = nullptr; // Optional

		/*
			This is used internally by the debug messenger to call the user callback.
			createInfo.pfnUserCallback = customUserCallback;

			Instead, use the following:
			createInfo.userCallback = customUserCallback;

			Which has the default value: vk::core::DebugMessenger::debugCallback
		*/
	}

	bool Device::isDeviceSuitable(const VkPhysicalDevice& physicalDevice)
	{
		Queue::FamilyIndices indices = findQueueFamilies(physicalDevice);

		bool extensionsSupported = vk::core::PhysicalDevice::checkExtensionSupport(physicalDevice, deviceExtensions);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			Swapchain::SupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
			swapChainAdequate = swapChainSupport.formats.empty() == false
							&&	swapChainSupport.presentModes.empty() == false;
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	Queue::FamilyIndices Device::findQueueFamilies(const VkPhysicalDevice& physicalDevice)
	{
		Queue::FamilyIndices indices;

		std::vector<VkQueueFamilyProperties> queueFamilyProperties = vk::core::PhysicalDevice::getQueueFamilyProperties(physicalDevice);

		int i = 0;
		for (const auto& queueFamily : queueFamilyProperties)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = vk::core::PhysicalDevice::getSurfaceSupport(physicalDevice, i, surface->getVk());

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	Swapchain::SupportDetails Device::querySwapChainSupport(const VkPhysicalDevice& device)
	{
		Swapchain::SupportDetails details;

		details.capabilities = vk::core::PhysicalDevice::getSurfaceCapabilities(device, surface->getVk());
		details.formats = vk::core::PhysicalDevice::getSurfaceFormats(device, surface->getVk());
		details.presentModes = vk::core::PhysicalDevice::getSurfacePresentModes(device, surface->getVk());

		return details;
	}

	VkSampleCountFlagBits Device::getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts =
			physicalDeviceProperties.limits.framebufferColorSampleCounts &
			physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}

}