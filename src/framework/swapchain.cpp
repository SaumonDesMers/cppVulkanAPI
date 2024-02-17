#include "swapchain.hpp"

#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace LIB_NAMESPACE
{
	Swapchain::Swapchain(VkDevice device, const CreateInfo& createInfo):
		m_device(device)
	{
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(createInfo.support_details.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(createInfo.support_details.presentModes);
		VkExtent2D extent = chooseSwapExtent(
			createInfo.support_details.capabilities,
			createInfo.extent.width, 
			createInfo.extent.height
		);

		uint32_t imageCount = createInfo.support_details.capabilities.minImageCount + 1;

		if (
			createInfo.support_details.capabilities.maxImageCount > 0
			&& imageCount > createInfo.support_details.capabilities.maxImageCount
		)
		{
			imageCount = createInfo.support_details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = createInfo.surface;
		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.imageFormat = surfaceFormat.format;
		swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage =
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		uint32_t queueFamilyIndices[] = {
			createInfo.queue_family_indices.graphicsFamily.value(),
			createInfo.queue_family_indices.presentFamily.value()
		};

		if (createInfo.queue_family_indices.graphicsFamily != createInfo.queue_family_indices.presentFamily)
		{
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		swapchainInfo.preTransform = createInfo.support_details.capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.presentMode = presentMode;
		swapchainInfo.clipped = VK_TRUE;

		swapchainInfo.oldSwapchain = createInfo.old_swapchain;

		if (vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain.");
		}

		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
		m_images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

		m_imageFormat = swapchainInfo.imageFormat;
		m_extent = swapchainInfo.imageExtent;
	}

	Swapchain::~Swapchain()
	{
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	}

	VkResult Swapchain::acquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* imageIndex)
	{
		return vkAcquireNextImageKHR(m_device, m_swapchain, timeout, semaphore, fence, imageIndex);
	}


	VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(
				actualExtent.width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width
			);

			actualExtent.height = std::clamp(
				actualExtent.height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height
			);

			return actualExtent;
		}
	}

}