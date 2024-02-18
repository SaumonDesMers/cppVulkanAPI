#include "image.hpp"

#include <stdexcept>

namespace LIB_NAMESPACE
{
	namespace core
	{
		Image::Image(
			VkDevice device,
			VkImageCreateInfo& createInfo
		):
			m_device(device)
		{
			if (vkCreateImage(device, &createInfo, nullptr, &m_image) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image.");
			}
		}

		Image::~Image()
		{
			vkDestroyImage(m_device, m_image, nullptr);
		}

		Image::Image(Image&& other):
			m_image(other.m_image),
			m_device(other.m_device)
		{
			other.m_image = VK_NULL_HANDLE;
		}

		VkMemoryRequirements Image::getMemoryRequirements() const
		{
			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(m_device, m_image, &memoryRequirements);
			return memoryRequirements;
		}

		VkResult Image::bindMemory(
			VkDeviceMemory memory,
			VkDeviceSize memoryOffset
		) const
		{
			return vkBindImageMemory(m_device, m_image, memory, memoryOffset);
		}
	}
}