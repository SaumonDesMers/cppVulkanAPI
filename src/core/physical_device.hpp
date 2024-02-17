#pragma once

#include "defines.hpp"
#include "instance.hpp"
#include "queue.hpp"
#include "swapchain.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace LIB_NAMESPACE
{
	namespace core
	{
		class PhysicalDevice
		{

		public:

			PhysicalDevice(
				VkInstance instance,
				const std::vector<const char*> & device_extensions,
				const VkSurfaceKHR & surface
			);

			PhysicalDevice(VkPhysicalDevice physicalDevice);

			~PhysicalDevice();

			VkPhysicalDevice getVk() const { return m_physical_device; }

			core::Queue::FamilyIndices queueFamilyIndices() const { return queue_family_indices; }

			bool isDeviceSuitable(
				const VkPhysicalDevice & physical_device,
				const std::vector<const char*> & device_extensions,
				const VkSurfaceKHR & surface
			);
			static Swapchain::SupportDetails querySwapChainSupport(
				const VkPhysicalDevice& physical_device,
				const VkSurfaceKHR& surface
			);
			static core::Queue::FamilyIndices findQueueFamilies(
				const VkPhysicalDevice & physical_device,
				const VkSurfaceKHR & surface
			);

			static std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice physicalDevice);

			static bool getSurfaceSupport(
				VkPhysicalDevice physicalDevice,
				uint32_t queueFamilyIndex,
				VkSurfaceKHR surface
			);

			static bool checkExtensionSupport(
				const VkPhysicalDevice& physicalDevice,
				const std::vector<const char*>& extensions
			);

			void getProperties(VkPhysicalDeviceProperties* properties) const;


		private:

			VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;

			core::Queue::FamilyIndices queue_family_indices;

		};
	}
}