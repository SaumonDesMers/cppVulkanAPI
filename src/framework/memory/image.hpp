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
		Image(const Image &) = delete;
		Image(Image && other);
		~Image();

		VkImage image() { return m_image.getVk(); }
		VkDeviceMemory memory() { return m_memory.getVk(); }
		VkImageView view() { return m_image_view.getVk(); }

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

		core::Image m_image;
		core::DeviceMemory m_memory;
		core::ImageView m_image_view;

		uint32_t m_width;
		uint32_t m_height;
		VkFormat m_format;
		uint32_t m_mipLevels;

		VkImageViewCreateInfo & setupImageViewCreateInfo(
			VkDevice device,
			VkImageViewCreateInfo & viewInfo
		);

	};
}