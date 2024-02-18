#include "image_view.hpp"

#include <stdexcept>

namespace LIB_NAMESPACE
{
	namespace core
	{
		ImageView::ImageView(VkDevice device, const VkImageViewCreateInfo& create_info)
			: m_device(device)
		{
			if (vkCreateImageView(m_device, &create_info, nullptr, &m_image_view) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image view.");
			}
		}

		ImageView::~ImageView()
		{
			vkDestroyImageView(m_device, m_image_view, nullptr);
		}

		ImageView::ImageView(ImageView && other):
			m_image_view(std::move(other.m_image_view)),
			m_device(std::move(other.m_device))
		{
			other.m_image_view = VK_NULL_HANDLE;
		}
	}
}