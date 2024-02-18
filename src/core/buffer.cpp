#include "buffer.hpp"

#include <stdexcept>
#include <iostream>

namespace LIB_NAMESPACE
{
	namespace core
	{
		Buffer::Buffer(
			VkDevice device,
			VkBufferCreateInfo& createInfo
		)
			:m_device(device)
		{
			if (vkCreateBuffer(m_device, &createInfo, nullptr, &m_buffer) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create vertex buffer.");
			}
		}

		Buffer::Buffer(
			VkDevice device,
			VkDeviceSize size,
			VkBufferUsageFlags usage
		)
			:m_device(device)
		{
			VkBufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.size = size;
			createInfo.usage = usage;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(m_device, &createInfo, nullptr, &m_buffer) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create vertex buffer.");
			}
		}

		Buffer::~Buffer()
		{
			vkDestroyBuffer(m_device, m_buffer, nullptr);
		}

		Buffer::Buffer(Buffer&& other)
		{
			m_buffer = other.m_buffer;
			other.m_buffer = VK_NULL_HANDLE;
			m_device = other.m_device;
		}

		VkMemoryRequirements Buffer::getMemoryRequirements()
		{
			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);

			return memRequirements;
		}
	}
}