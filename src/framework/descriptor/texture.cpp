#include "texture.hpp"
#include "framework/memory/buffer.hpp"

#include <stdexcept>

namespace LIB_NAMESPACE
{
	Texture::Texture(
		VkDevice device,
		VkPhysicalDevice physical_device,
		Command & command,
		CreateInfo & create_info
	):
		m_width(create_info.width),
		m_height(create_info.height)
	{
		VkDeviceSize imageSize = m_width * m_height * 4;

		vk::Buffer stagingBuffer = vk::Buffer::createStagingBuffer(
			device,
			physical_device,
			imageSize
		);

		stagingBuffer.map();
		stagingBuffer.write((void*)create_info.data, imageSize);
		stagingBuffer.unmap();

		createImage(device, physical_device, create_info);

		command.transitionImageLayout(
			m_image->image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_image->mipLevels(),
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			static_cast<uint32_t>(m_width),
			static_cast<uint32_t>(m_height),
			1
		};

		command.copyBufferToImage(
			stagingBuffer.buffer(),
			m_image->image(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		createSampler(device, physical_device, create_info);
		createDescriptor(device, create_info);
	}

	Texture::Texture(Texture&& other):
		m_image(std::move(other.m_image)),
		m_sampler(std::move(other.m_sampler)),
		m_descriptor(std::move(other.m_descriptor)),
		m_width(other.m_width),
		m_height(other.m_height)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::createImage(
		VkDevice device,
		VkPhysicalDevice physical_device,
		CreateInfo& create_info
	)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_width);
		imageInfo.extent.height = static_cast<uint32_t>(m_height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = create_info.mipLevel;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = VK_NULL_HANDLE;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = create_info.mipLevel;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		m_image = std::make_unique<Image>(
			device,
			physical_device,
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			viewInfo
		);
	}

	void Texture::createSampler(
		VkDevice device,
		VkPhysicalDevice physical_device,
		CreateInfo& create_info
	)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physical_device, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(create_info.mipLevel);

		m_sampler = std::make_unique<vk::core::Sampler>(device, samplerInfo);
	}

	void Texture::createDescriptor(
		VkDevice device,
		CreateInfo& create_info
	)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = create_info.stageFlags;

		vk::Descriptor::CreateInfo descriptorInfo{};
		descriptorInfo.bindings = { layoutBinding };
		descriptorInfo.descriptor_count = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		m_descriptor = std::make_unique<Descriptor>(device, descriptorInfo);


		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_image->view();
			imageInfo.sampler = m_sampler->getVk();

			VkWriteDescriptorSet samplerDescriptorWrites{};
			samplerDescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			samplerDescriptorWrites.dstSet = m_descriptor->set(i);
			samplerDescriptorWrites.dstBinding = 0;
			samplerDescriptorWrites.dstArrayElement = 0;
			samplerDescriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerDescriptorWrites.descriptorCount = 1;
			samplerDescriptorWrites.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(
				device,
				1,
				&samplerDescriptorWrites,
				0, nullptr
			);
		}
	}

}