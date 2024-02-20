#pragma once

#include "defines.hpp"
#include "core/buffer.hpp"
#include "core/device_memory.hpp"

#include <memory>

namespace LIB_NAMESPACE
{
	class Buffer
	{

	public:

		Buffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkBufferCreateInfo bufferInfo,
			VkMemoryPropertyFlags properties
		);
		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator=(const Buffer &) = delete;
		Buffer & operator=(Buffer && other) = delete;
		~Buffer();

		VkBuffer buffer() { return m_buffer.getVk(); }
		VkDeviceMemory memory() { return m_memory.getVk(); }

		VkResult map(
			VkDeviceSize offset = 0,
			VkDeviceSize size = VK_WHOLE_SIZE,
			VkMemoryMapFlags flags = 0
		);
		void unmap();

		void write(void *data, uint32_t size);

		static Buffer createStagingBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size
		);

		static Buffer createVertexBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size
		);

		static Buffer createIndexBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size
		);

	private:

		core::Buffer m_buffer;
		core::DeviceMemory m_memory;

	};
}