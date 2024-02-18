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
				const VkMemoryAllocateInfo & alloc_info
			);

			DeviceMemory(
				VkDevice device,
				VkPhysicalDevice physical_device,
				VkMemoryPropertyFlags properties,
				VkMemoryRequirements memory_requirements
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
				VkPhysicalDevice physical_device,
				uint32_t typeFilter,
				VkMemoryPropertyFlags properties
			);
		
		private:

			VkDeviceMemory m_memory;

			VkDevice m_device;

			bool m_is_mapped;
			void *m_mapped_memory;

		};
	}
}