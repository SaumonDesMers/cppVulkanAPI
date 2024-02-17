#include "device.hpp"

#include <stdexcept>
#include <iostream>
#include <set>

namespace LIB_NAMESPACE
{
	namespace core
	{
		Device::Device(
			const PhysicalDevice & physical_device,
			const std::vector<const char*> & validation_layers,
			const std::vector<const char*> & device_extensions
		)
		{
			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			VkPhysicalDeviceFeatures deviceFeatures = {};
			deviceFeatures.samplerAnisotropy = VK_TRUE;
			createInfo.pEnabledFeatures = &deviceFeatures;

			VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
			dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
			dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
			createInfo.pNext = &dynamicRenderingFeatures;

			auto queueFamilyIndices = physical_device.queueFamilyIndices();

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = {
				queueFamilyIndices.graphicsFamily.value(),
				queueFamilyIndices.presentFamily.value()
			};

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
			createInfo.ppEnabledExtensionNames = device_extensions.data();

			#ifndef NDEBUG
				createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
				createInfo.ppEnabledLayerNames = validation_layers.data();
			#else
				(void)validation_layers;
				createInfo.enabledLayerCount = 0;
			#endif

			VK_CHECK(
				vkCreateDevice(physical_device.getVk(), &createInfo, nullptr, &m_device),
				"Failed to create logical device."
			);
		}

		Device::Device(VkPhysicalDevice physicalDevice, const CreateInfo & createInfo)
		{
			if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create logical device.");
			}
		}

		Device::~Device()
		{
			vkDestroyDevice(m_device, nullptr);
		}

		VkResult Device::waitIdle()
		{
			return vkDeviceWaitIdle(m_device);
		}
	}
}