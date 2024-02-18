#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

namespace LIB_NAMESPACE
{
	namespace core
	{
		class DeviceMemory
		{

		public:

			DeviceMemory(
				VkDevice device,
				const VkMemoryAllocateInfo & allocateInfo
			);

			~DeviceMemory();

			DeviceMemory(const DeviceMemory &) = delete;

			DeviceMemory(DeviceMemory && other);

			VkDeviceMemory getVk() { return m_memory; }

			VkResult map(
				VkDeviceSize offset = 0,
				VkDeviceSize size = VK_WHOLE_SIZE,
				VkMemoryMapFlags flags = 0
			);
			void unmap();

			void write(void *data, uint32_t size);

			static uint32_t findMemoryType(
				VkPhysicalDevice physicalDevice,
				uint32_t typeFilter,
				VkMemoryPropertyFlags properties
			);
		
		private:

			VkDeviceMemory m_memory;

			VkDevice m_device;

			bool m_isMapped;
			void *m_mappedMemory;

			void init(
				const VkMemoryAllocateInfo& allocateInfo,
				VkMemoryPropertyFlags properties
			);

		};
	}
}