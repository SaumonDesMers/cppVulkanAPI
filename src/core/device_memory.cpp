#include "device_memory.hpp"

#include <stdexcept>
#include <string.h>
#include <iostream>

namespace LIB_NAMESPACE
{
	namespace core
	{
		DeviceMemory::DeviceMemory(
			VkDevice device,
			const VkMemoryAllocateInfo & alloc_info
		):
			m_device(device)
		{
			VK_CHECK(vkAllocateMemory(device, &alloc_info, nullptr, &m_memory), "failed to allocate device memory.");
		}

		DeviceMemory::DeviceMemory(
			VkDevice device,
			VkPhysicalDevice physical_device,
			VkMemoryPropertyFlags properties,
			VkMemoryRequirements memory_requirements
		):
			m_device(device)
		{
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = memory_requirements.size;
			alloc_info.memoryTypeIndex = findMemoryType(
				physical_device,
				memory_requirements.memoryTypeBits,
				properties
			);

			VK_CHECK(vkAllocateMemory(device, &alloc_info, nullptr, &m_memory), "failed to allocate device memory.");
		}

		DeviceMemory::~DeviceMemory()
		{
			vkFreeMemory(m_device, m_memory, nullptr);
		}

		DeviceMemory::DeviceMemory(DeviceMemory && other):
			m_memory(other.m_memory),
			m_device(other.m_device),
			m_is_mapped(other.m_is_mapped),
			m_mapped_memory(other.m_mapped_memory)
		{
			other.m_memory = VK_NULL_HANDLE;
			other.m_is_mapped = false;
			other.m_mapped_memory = nullptr;
		}

		uint32_t DeviceMemory::findMemoryType(
			VkPhysicalDevice physical_device,
			uint32_t type_filter,
			VkMemoryPropertyFlags properties
		)
		{
			VkPhysicalDeviceMemoryProperties mem_properties;
			vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

			for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
			{
				if (
					(type_filter & (1 << i))
					&& (mem_properties.memoryTypes[i].propertyFlags & properties) == properties
				)
				{
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type for buffer.");
		}

		VkResult DeviceMemory::map(
			VkDeviceSize offset,
			VkDeviceSize size,
			VkMemoryMapFlags flags
		)
		{
			VkResult result = vkMapMemory(m_device, m_memory, offset, size, flags, &m_mapped_memory);

			if (result == VK_SUCCESS)
			{
				m_is_mapped = true;
			}

			return result;
		}

		void DeviceMemory::unmap()
		{
			vkUnmapMemory(m_device, m_memory);
			m_is_mapped = false;
		}

		void DeviceMemory::write(void *data, uint32_t size)
		{
			if (m_is_mapped == false)
			{
				throw std::runtime_error("failed to write data to memory: memory is not mapped.");
			}

			memcpy(m_mapped_memory, data, size);

		}
	}
}