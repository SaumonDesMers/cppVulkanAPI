#pragma once

#include "defines.hpp"
#include "core/image/image.hpp"
#include "core/device_memory.hpp"
#include "core/image/image_view.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace LIB_NAMESPACE
{
	class Image
	{

	public:

		Image(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkImageCreateInfo imageInfo,
			VkMemoryPropertyFlags properties,
			VkImageViewCreateInfo viewInfo
		);
		Image(const Image&) = delete;
		Image(Image&&);
		~Image();

		VkImage image() const { return m_image->getVk(); }
		VkDeviceMemory memory() const { return m_memory->getVk(); }
		VkImageView view() const { return m_imageView->getVk(); }

		uint32_t width() const { return m_width; }
		uint32_t height() const { return m_height; }
		VkExtent2D extent() const { return { m_width, m_height }; }
		VkFormat format() const { return m_format; }
		uint32_t mipLevels() const { return m_mipLevels; }

		static Image createDepthImage(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkExtent2D extent,
			VkFormat format
		);

		static Image createColorImage(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkExtent2D extent,
			VkFormat format
		);

	private:

		std::unique_ptr<core::Image> m_image;
		std::unique_ptr<core::DeviceMemory> m_memory;
		std::unique_ptr<core::ImageView> m_imageView;

		uint32_t m_width;
		uint32_t m_height;
		VkFormat m_format;
		uint32_t m_mipLevels;

	};
}