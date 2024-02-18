#include "buffer.hpp"

#include <stdexcept>
#include <iostream>

namespace LIB_NAMESPACE
{
	Buffer::Buffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkBufferCreateInfo bufferInfo,
		VkMemoryPropertyFlags properties
	):
		m_buffer(device, bufferInfo),
		m_memory(device, memoryAllocateInfo(physicalDevice, properties))
	{
		vkBindBufferMemory(device, m_buffer.getVk(), m_memory.getVk(), 0);
	}

	Buffer::Buffer(Buffer&& other):
		m_buffer(std::move(other.m_buffer)),
		m_memory(std::move(other.m_memory))
	{

	}

	Buffer::~Buffer()
	{
	}

	VkMemoryAllocateInfo Buffer::memoryAllocateInfo(
		VkPhysicalDevice physicalDevice,
		VkMemoryPropertyFlags properties
	)
	{
		VkMemoryRequirements memRequirements = m_buffer.getMemoryRequirements();

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core::DeviceMemory::findMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			properties
		);

		return allocInfo;
	
	}

	VkResult Buffer::map(
		VkDeviceSize offset,
		VkDeviceSize size,
		VkMemoryMapFlags flags
	)
	{
		return m_memory.map(offset, size, flags);
	}

	void Buffer::unmap()
	{
		m_memory.unmap();
	}

	void Buffer::write(void *data, uint32_t size)
	{
		m_memory.write(data, size);
	}



	Buffer Buffer::createStagingBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size
	)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		return Buffer(
			device,
			physicalDevice,
			bufferInfo,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	}

	Buffer Buffer::createVertexBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size
	)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		return Buffer(
			device,
			physicalDevice,
			bufferInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	}

	Buffer Buffer::createIndexBuffer(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkDeviceSize size
	)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		return Buffer(
			device,
			physicalDevice,
			bufferInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	}

}