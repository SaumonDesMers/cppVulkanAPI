#include "physical_device.hpp"

#include <string>
#include <set>
#include <stdexcept>

namespace LIB_NAMESPACE
{
	namespace core
	{
		PhysicalDevice::PhysicalDevice(
			VkInstance instance,
			const std::vector<const char*> & device_extensions,
			const VkSurfaceKHR & surface
		)
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			std::vector<VkPhysicalDevice> physical_devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, physical_devices.data());

			if (physical_devices.empty())
			{
				throw std::runtime_error("Failed to find GPUs with Vulkan support");
			}

			for (const auto& physical_device : physical_devices)
			{
				if (isDeviceSuitable(physical_device, device_extensions, surface))
				{
					m_physical_device = physical_device;
					break;
				}
			}

			if (m_physical_device == VK_NULL_HANDLE)
			{
				throw std::runtime_error("Failed to find a suitable GPU");
			}

			queue_family_indices = findQueueFamilies(m_physical_device, surface);
		}

		PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice)
			: m_physical_device(physicalDevice)
		{

		}

		PhysicalDevice::~PhysicalDevice()
		{

		}

		bool PhysicalDevice::isDeviceSuitable(
			const VkPhysicalDevice & physical_device,
			const std::vector<const char*> & device_extensions,
			const VkSurfaceKHR & surface
		)
		{
			core::Queue::FamilyIndices indices = findQueueFamilies(physical_device, surface);

			bool extensionsSupported = core::PhysicalDevice::checkExtensionSupport(physical_device, device_extensions);

			bool swapChainAdequate = false;
			if (extensionsSupported)
			{
				core::Swapchain::SupportDetails swapChainSupport = querySwapChainSupport(physical_device, surface);
				swapChainAdequate = swapChainSupport.formats.empty() == false
								&&	swapChainSupport.presentModes.empty() == false;
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

			return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
		}

		core::Queue::FamilyIndices PhysicalDevice::findQueueFamilies(
			const VkPhysicalDevice & physical_device,
			const VkSurfaceKHR & surface
		)
		{
			core::Queue::FamilyIndices indices;

			std::vector<VkQueueFamilyProperties> queueFamilyProperties = core::PhysicalDevice::getQueueFamilyProperties(physical_device);

			int i = 0;
			for (const auto& queueFamily : queueFamilyProperties)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
				}

				VkBool32 presentSupport = core::PhysicalDevice::getSurfaceSupport(physical_device, i, surface);

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

		core::Swapchain::SupportDetails PhysicalDevice::querySwapChainSupport(
			const VkPhysicalDevice & physical_device,
			const VkSurfaceKHR & surface
		)
		{
			core::Swapchain::SupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
			
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr);
			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties(VkPhysicalDevice physicalDevice)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

			return queueFamilyProperties;
		}

		bool PhysicalDevice::getSurfaceSupport(
			VkPhysicalDevice physicalDevice,
			uint32_t queueFamilyIndex,
			VkSurfaceKHR surface
		)
		{
			VkBool32 surfaceSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &surfaceSupport);

			return surfaceSupport;
		}

		bool PhysicalDevice::checkExtensionSupport(
			const VkPhysicalDevice& physicalDevice,
			const std::vector<const char*>& extensions
		)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

			for (const auto& extension : availableExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		void PhysicalDevice::getProperties(VkPhysicalDeviceProperties* properties) const
		{
			vkGetPhysicalDeviceProperties(m_physical_device, properties);
		}
	}
}