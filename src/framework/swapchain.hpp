#pragma once

#include "defines.hpp"
#include "queue.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace LIB_NAMESPACE
{
	class Swapchain
	{

	public:

		struct SupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct CreateInfo
		{
			VkSurfaceKHR surface;
			Swapchain::SupportDetails support_details;
			VkExtent2D extent;
			core::Queue::FamilyIndices queue_family_indices;
			VkSwapchainKHR old_swapchain;
		};

		Swapchain(VkDevice device, const CreateInfo& createInfo);
		~Swapchain();

		VkSwapchainKHR getVk() const { return m_swapchain; }

		VkImage image(uint32_t index) const { return m_images[index]; }
		uint32_t imageCount() const { return m_images.size(); }
		VkFormat imageFormat() const { return m_imageFormat; }
		VkExtent2D extent() const { return m_extent; }

		VkResult acquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* imageIndex);


	private:

		VkSwapchainKHR m_swapchain;

		VkDevice m_device;

		std::vector<VkImage> m_images;
		VkFormat m_imageFormat;
		VkExtent2D m_extent;

		void createSwapchain(const CreateInfo& createInfo);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);

	};
}