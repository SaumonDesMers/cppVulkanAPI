#include "image.hpp"

#include <stdexcept>
#include <iostream>

namespace LIB_NAMESPACE
{
	Image::Image(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkImageCreateInfo imageInfo,
		VkMemoryPropertyFlags properties,
		VkImageViewCreateInfo viewInfo
	):
		m_image(device, imageInfo),
		m_memory(device, physicalDevice, properties, m_image.getMemoryRequirements()),
		m_image_view(device, setupImageViewCreateInfo(device, viewInfo)),
		m_width(imageInfo.extent.width),
		m_height(imageInfo.extent.height),
		m_format(imageInfo.format),
		m_mipLevels(imageInfo.mipLevels)
	{
	}

	Image::Image(Image&& other):
		m_image(std::move(other.m_image)),
		m_memory(std::move(other.m_memory)),
		m_image_view(std::move(other.m_image_view)),
		m_width(other.m_width),
		m_height(other.m_height),
		m_format(other.m_format),
		m_mipLevels(other.m_mipLevels)
	{
	}

	Image::~Image()
	{
	}

	VkImageViewCreateInfo & Image::setupImageViewCreateInfo(
		VkDevice device,
		VkImageViewCreateInfo & viewInfo
	)
	{
		vkBindImageMemory(device, m_image.getVk(), m_memory.getVk(), 0);

		viewInfo.image = m_image.getVk();

		return viewInfo;
	}


	Image Image::createDepthImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkExtent2D extent,
		VkFormat format
	)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = extent.width;
		imageInfo.extent.height = extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage =
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = VK_NULL_HANDLE;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		return Image(device, physicalDevice, imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, viewInfo);
	}

	Image Image::createColorImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkExtent2D extent,
		VkFormat format
	)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent.width = extent.width;
		imageInfo.extent.height = extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage =
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = VK_NULL_HANDLE;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		return Image(device, physicalDevice, imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, viewInfo);
	}

}