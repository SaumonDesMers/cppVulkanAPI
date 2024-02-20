#include "render_api.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include <set>
#include <chrono>
#include <iostream>

namespace LIB_NAMESPACE
{

	RenderAPI::RenderAPI(GLFWwindow *glfwWindow):
		m_device(glfwWindow)
	{
		createCommandPool();
		createSwapchain();
		createSyncObjects();
	}

	RenderAPI::~RenderAPI()
	{
		m_device.device().waitIdle();

		for (auto& vkCommandBuffer : m_vk_command_buffers)
		{
			m_command->freeCommandBuffer(vkCommandBuffer);
		}
	}


	void RenderAPI::createSwapchain()
	{
		Swapchain::CreateInfo swapchainInfo = {};
		swapchainInfo.surface = m_device.surface().getVk();
		swapchainInfo.support_details = m_device.querySwapChainSupport(m_device.physicalDevice().getVk());

		int width, height;
		glfwGetFramebufferSize(m_device.glfwWindow, &width, &height);
		swapchainInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		swapchainInfo.queue_family_indices = m_device.physicalDevice().queueFamilyIndices();

		swapchainInfo.old_swapchain = VK_NULL_HANDLE;

		m_swapchain = std::make_unique<Swapchain>(m_device.device().getVk(), swapchainInfo);
	}

	void RenderAPI::recreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_device.glfwWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_device.glfwWindow, &width, &height);
			glfwWaitEvents();
		}

		m_device.device().waitIdle();

		m_swapchain.reset();
		createSwapchain();

		for (auto& color_target : m_color_target_map)
		{
			createColorTarget(color_target.first);
		}
		for (auto& depth_target : m_depth_target_map)
		{
			createDepthTarget(depth_target.first);
		}
	}

	void RenderAPI::createCommandPool()
	{
		Command::CreateInfo commandInfo{};
		commandInfo.queueFamilyIndex = m_device.physicalDevice().queueFamilyIndices().graphicsFamily.value();
		commandInfo.queue = m_device.graphicsQueue().getVk();
		commandInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		m_command = std::make_unique<Command>(m_device.device().getVk(), commandInfo);

		m_vk_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_vk_command_buffers[i] = m_command->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
	}

	void RenderAPI::createSyncObjects()
	{
		m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_swapchain_updated_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

		core::Semaphore::CreateInfo semaphoreInfo{};

		core::Fence::CreateInfo fenceInfo{};
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_image_available_semaphores[i] = std::make_unique<core::Semaphore>(m_device.device().getVk(), semaphoreInfo);
			m_render_finished_semaphores[i] = std::make_unique<core::Semaphore>(m_device.device().getVk(), semaphoreInfo);
			m_swapchain_updated_semaphores[i] = std::make_unique<core::Semaphore>(m_device.device().getVk(), semaphoreInfo);
			m_in_flight_fences[i] = std::make_unique<core::Fence>(m_device.device().getVk(), fenceInfo);
		}

	}


	uint64_t RenderAPI::createColorTarget(uint64_t id)
	{
		uint64_t color_target_id = id;

		Image color_target = Image::createColorImage(
			m_device.device().getVk(),
			m_device.physicalDevice().getVk(),
			m_swapchain->extent(),
			VK_FORMAT_R16G16B16A16_SFLOAT
		);

		if (color_target_id == Map<Image>::no_id)
		{
			color_target_id = m_color_target_map.insert(std::move(color_target));
		}
		else
		{
			m_color_target_map.replace(color_target_id, std::move(color_target));
		}

		m_command->transitionImageLayout(
			m_color_target_map.get(color_target_id).image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		);

		return color_target_id;
	}

	uint64_t RenderAPI::createDepthTarget(uint64_t id)
	{
		uint64_t depth_target_id = id;

		Image depth_target = Image::createDepthImage(
			m_device.device().getVk(),
			m_device.physicalDevice().getVk(),
			m_swapchain->extent(),
			findDepthFormat()
		);

		if (depth_target_id == Map<Image>::no_id)
		{
			depth_target_id = m_depth_target_map.insert(std::move(depth_target));
		}
		else
		{
			m_depth_target_map.replace(depth_target_id, std::move(depth_target));
		}

		m_command->transitionImageLayout(
			m_depth_target_map.get(depth_target_id).image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			0,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
		);

		return depth_target_id;
	}


	VkFormat RenderAPI::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (const auto& format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice().getVk(), format, &props);

			if (
				tiling == VK_IMAGE_TILING_LINEAR &&
				(props.linearTilingFeatures & features) == features
			)
			{
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL &&
				(props.optimalTilingFeatures & features) == features
			)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format.");
	}

	VkFormat RenderAPI::findDepthFormat() {
		static VkFormat depthFormat = findSupportedFormat(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		return depthFormat;
	}

	bool RenderAPI::hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}


	void RenderAPI::generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice().getVk(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("texture image format does not support linear blitting.");
		}

		VkCommandBuffer commandBuffer = m_command->beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {
				mipWidth > 1 ? mipWidth / 2 : 1,
				mipHeight > 1 ? mipHeight / 2 : 1,
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_command->endSingleTimeCommands(commandBuffer);
	}

	void RenderAPI::copyRenderedImageToSwapchainImage(uint64_t color_target_id, uint32_t swapchain_image_index)
	{
		Image & target_image = m_color_target_map.get(color_target_id);

		// First, we need to transition the swap chain image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout, so we can copy the offscreen image to it.
		m_command->transitionImageLayout(
			m_swapchain->image(swapchain_image_index),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			0,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		// Then we need to transition the offscreen image to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout, so we can copy it to the swap chain image.
		m_command->transitionImageLayout(
			target_image.image(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);

		// The offscreen image is allready in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout, so we can copy it to the swap chain image.
		VkCommandBuffer commandBuffer = m_command->beginSingleTimeCommands();

		// copy with blit
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {
			static_cast<int32_t>(target_image.width()),
			static_cast<int32_t>(target_image.height()),
			1
		};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {
			static_cast<int32_t>(m_swapchain->extent().width),
			static_cast<int32_t>(m_swapchain->extent().height),
			1
		};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			target_image.image(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_swapchain->image(swapchain_image_index),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR
		);

		VkResult result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS)
		{
			TROW("Failed to record command buffer", result);
		}

		VkSubmitInfo copyImageInfo = {};
		copyImageInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		copyImageInfo.commandBufferCount = 1;
		copyImageInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore copyImageWaitSemaphores[] = {
			m_image_available_semaphores[m_current_frame]->getVk(),
			m_render_finished_semaphores[m_current_frame]->getVk()
		};
		VkPipelineStageFlags copyImageWaitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		copyImageInfo.waitSemaphoreCount = 2;
		copyImageInfo.pWaitSemaphores = copyImageWaitSemaphores;
		copyImageInfo.pWaitDstStageMask = copyImageWaitStages;

		VkSemaphore copyImageSignalSemaphores[] = {
			m_swapchain_updated_semaphores[m_current_frame]->getVk()
		};
		copyImageInfo.signalSemaphoreCount = 1;
		copyImageInfo.pSignalSemaphores = copyImageSignalSemaphores;

		m_command->submit(1, &copyImageInfo, VK_NULL_HANDLE);

		m_command->queueWaitIdle();
		m_command->freeCommandBuffer(commandBuffer);

		// Now we need to transition the swap chain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout, so we can present it.
		m_command->transitionImageLayout(
			m_swapchain->image(swapchain_image_index),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			0,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);

		// And we need to transition the offscreen image back to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL layout, so we can render to it again.
		m_command->transitionImageLayout(
			target_image.image(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			0,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		);
	}



	void RenderAPI::startDraw()
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		m_in_flight_fences[m_current_frame]->wait();
		m_in_flight_fences[m_current_frame]->reset();

		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(cmd, &beginInfo);
	}

	void RenderAPI::startRendering(
		const std::vector<uint64_t> & color_target_ids,
		uint64_t depth_target_id
	)
	{
		#ifndef NDEBUG
			if (color_target_ids.size() == 0)
			{
				throw std::runtime_error("Cannot start rendering without color target.");
			}
		#endif

		std::unique_lock<std::mutex> lock(m_global_mutex);

		std::vector<VkRenderingAttachmentInfo> color_attachments(color_target_ids.size());
		for (size_t i = 0; i < color_target_ids.size(); i++)
		{
			color_attachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			color_attachments[i].imageView = m_color_target_map.get(color_target_ids[i]).view();
			color_attachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			color_attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color_attachments[i].clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
		}

		VkRenderingAttachmentInfo depth_attachment = {};
		depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth_attachment.imageView = m_depth_target_map.get(depth_target_id).view();
		depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.clearValue = {1.0f, 0};

		VkRenderingInfo rendering_info = {};
		rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		rendering_info.renderArea = {
			0, 0,
			m_color_target_map.get(color_target_ids[0]).width(),
			m_color_target_map.get(color_target_ids[0]).height()
		};
		rendering_info.layerCount = 1;
		rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
		rendering_info.pColorAttachments = color_attachments.data();
		rendering_info.pDepthAttachment = &depth_attachment;

		vkCmdBeginRendering(m_vk_command_buffers[m_current_frame], &rendering_info);
	}

	void RenderAPI::endRendering()
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdEndRendering(cmd);
	}

	void RenderAPI::endDraw(uint64_t color_target_id)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		vkEndCommandBuffer(m_vk_command_buffers[m_current_frame]);

		VkSubmitInfo renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer commandBuffers[] = {m_vk_command_buffers[m_current_frame]};
		renderInfo.commandBufferCount = 1;
		renderInfo.pCommandBuffers = commandBuffers;

		VkSemaphore signalSemaphores[] = {m_render_finished_semaphores[m_current_frame]->getVk()};
		renderInfo.signalSemaphoreCount = 1;
		renderInfo.pSignalSemaphores = signalSemaphores;

		m_device.graphicsQueue().submit(1, &renderInfo, m_in_flight_fences[m_current_frame]->getVk());


		// Now instead of rendering directly to the swap chain image, we render to the offscreen image, and then copy it to the swap chain image.

		uint32_t imageIndex;
		VkResult result = m_swapchain->acquireNextImage(
			UINT64_MAX, m_image_available_semaphores[m_current_frame]->getVk(), VK_NULL_HANDLE, &imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		copyRenderedImageToSwapchainImage(color_target_id, imageIndex);

		// Finally, we present the swap chain image.
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore copyImageSignalSemaphores[] = {
			m_swapchain_updated_semaphores[m_current_frame]->getVk()
		};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = copyImageSignalSemaphores;

		VkSwapchainKHR swapChains[] = {m_swapchain->getVk()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_device.presentQueue().getVk(), &presentInfo);


		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image.");
		}

		m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}



	uint64_t RenderAPI::loadModel(const std::string & filename)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		Mesh::CreateInfo meshInfo = {};

		Mesh::readObjFile(filename, meshInfo.vertices, meshInfo.indices);

		return m_mesh_map.insert(Mesh(
			m_device.device().getVk(),
			m_device.physicalDevice().getVk(),
			*m_command.get(),
			meshInfo
		));
	}

	uint64_t RenderAPI::newDescriptor(VkDescriptorSetLayoutBinding layoutBinding)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		Descriptor::CreateInfo descriptorInfo{};
		descriptorInfo.bindings = { layoutBinding };
		descriptorInfo.descriptor_count = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		return m_descriptor_map.insert(Descriptor(
			m_device.device().getVk(),
			descriptorInfo
		));
	}

	uint64_t RenderAPI::loadTexture(Texture::CreateInfo & createInfo)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		uint64_t texture_id = m_texture_map.insert(Texture(
			m_device.device().getVk(),
			m_device.physicalDevice().getVk(),
			*m_command.get(),
			createInfo
		));

		generateMipmaps(
			m_texture_map.get(texture_id).image().image(),
			VK_FORMAT_R8G8B8A8_SRGB,
			m_texture_map.get(texture_id).width(),
			m_texture_map.get(texture_id).height(),
			m_texture_map.get(texture_id).image().mipLevels()
		);

		return texture_id;
	}

	uint64_t RenderAPI::newPipeline(Pipeline::CreateInfo & createInfo)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		std::vector<VkFormat> colorAttachementFormats;
		for (auto& color_target_id : createInfo.color_target_ids)
		{
			colorAttachementFormats.push_back(m_color_target_map.get(color_target_id).format());
		}

		VkPipelineRenderingCreateInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachementFormats.size());
		renderingInfo.pColorAttachmentFormats = colorAttachementFormats.data();
		renderingInfo.depthAttachmentFormat = m_depth_target_map.get(createInfo.depth_target_id).format();

		createInfo.pNext = &renderingInfo;

		return m_pipeline_map.insert(Pipeline(
			m_device.device().getVk(),
			createInfo
		));
	}

	uint64_t RenderAPI::newUniformBuffer(const UniformBuffer::CreateInfo & create_info)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return m_uniform_buffer_map.insert(UniformBuffer(
			m_device.device().getVk(),
			m_device.physicalDevice().getVk(),
			create_info
		));
	}

	uint64_t RenderAPI::newColorTarget()
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return createColorTarget();
	}

	uint64_t RenderAPI::newDepthTarget()
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return createDepthTarget();
	}


	void RenderAPI::bindPipeline(uint64_t pipelineID)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_map.get(pipelineID).pipeline->getVk());
	}

	void RenderAPI::bindDescriptor(
		uint64_t pipelineID,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const VkDescriptorSet *pDescriptorSets
	)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipeline_map.get(pipelineID).layout->getVk(),
			firstSet, descriptorSetCount,
			pDescriptorSets,
			0, nullptr
		);
	}

	void RenderAPI::pushConstant(
		uint64_t pipelineID,
		VkShaderStageFlags stageFlags,
		uint32_t size,
		const void* data
	)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdPushConstants(
			cmd,
			m_pipeline_map.get(pipelineID).layout->getVk(),
			stageFlags,
			0,
			size,
			data
		);
	}

	void RenderAPI::setViewport(VkViewport& viewport)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdSetViewport(cmd, 0, 1, &viewport);
	}

	void RenderAPI::setScissor(VkRect2D& scissor)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void RenderAPI::drawMesh(uint64_t meshID)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		VkCommandBuffer cmd = m_vk_command_buffers[m_current_frame];

		VkBuffer vertexBuffers[] = {m_mesh_map.get(meshID).vertexBuffer().buffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(cmd, m_mesh_map.get(meshID).indexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, m_mesh_map.get(meshID).indexCount(), 1, 0, 0, 0);
	}


	GLFWwindow* RenderAPI::getWindow()
	{
		return m_device.glfwWindow;
	}

	uint32_t RenderAPI::currentFrame()
	{
		return m_current_frame;
	}

	Mesh & RenderAPI::getMesh(uint32_t meshID)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return m_mesh_map.get(meshID);
	}

	Descriptor & RenderAPI::getDescriptor(uint64_t descriptor_id)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return m_descriptor_map.get(descriptor_id);
	}

	Texture & RenderAPI::getTexture(uint64_t textureID)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return m_texture_map.get(textureID);
	}

	UniformBuffer & RenderAPI::getUniformBuffer(uint64_t uniform_buffer_id)
	{
		std::unique_lock<std::mutex> lock(m_global_mutex);

		return m_uniform_buffer_map.get(uniform_buffer_id);
	}
}
