#pragma once

#include "defines.hpp"
#include "physical_device.hpp"

#include <vulkan/vulkan.h>

namespace LIB_NAMESPACE
{
	namespace core
	{
		class Device
		{

		public:

			struct CreateInfo: public VkDeviceCreateInfo
			{
				CreateInfo(): VkDeviceCreateInfo()
				{
					sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				}
			};

			Device(
				const PhysicalDevice & physical_device,
				const std::vector<const char*> & validation_layers,
				const std::vector<const char*> & device_extensions
			);

			Device(VkPhysicalDevice physicalDevice, const CreateInfo & createInfo);
			~Device();

			VkDevice getVk() const { return m_device; }

			VkResult waitIdle();

		private:

			VkDevice m_device;
		};
	}
}