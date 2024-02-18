#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

namespace LIB_NAMESPACE
{
	namespace core
	{
		class ImageView
		{
		
		public:

			ImageView(
				VkDevice device,
				const VkImageViewCreateInfo & create_info
			);

			~ImageView();

			ImageView(const ImageView &) = delete;

			ImageView(ImageView && other);

			VkImageView getVk() const { return m_image_view; }

		private:

			VkImageView m_image_view;

			VkDevice m_device;

		};
	}
}