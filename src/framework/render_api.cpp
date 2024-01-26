#include "render_api.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include <set>
#include <chrono>

namespace LIB_NAMESPACE
{

	RenderAPI::RenderAPI()
	{	
		init();
	}

	RenderAPI::~RenderAPI()
	{
		m_device->device->waitIdle();

		for (auto& vkCommandBuffer : m_vkCommandBuffers)
		{
			m_command->freeCommandBuffer(vkCommandBuffer);
		}
	}

	void RenderAPI::init()
	{
		createDevice();
		createSwapchain();
		createColorResources();
		createDepthResources();
		createCommandPool();
		createSyncObjects();
	}


	void RenderAPI::createDevice()
	{
		m_device = std::make_unique<vk::Device>();
	}

	void RenderAPI::createSwapchain()
	{
		vk::Swapchain::CreateInfo swapchainInfo = {};
		swapchainInfo.surface = m_device->surface->getVk();
		swapchainInfo.supportDetails = m_device->querySwapChainSupport(m_device->physicalDevice->getVk());
		
		int width, height;
		m_device->window->getFramebufferSize(&width, &height);
		swapchainInfo.frameBufferExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		swapchainInfo.queueFamilyIndices = m_device->findQueueFamilies(m_device->physicalDevice->getVk());

		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		m_swapchain = std::make_unique<vk::Swapchain>(m_device->device->getVk(), swapchainInfo);
	}

	void RenderAPI::recreateSwapChain()
	{
		int width = 0, height = 0;
		m_device->window->getFramebufferSize(&width, &height);
		while (width == 0 || height == 0)
		{
			m_device->window->getFramebufferSize(&width, &height);
			m_device->windowManager->waitEvents();
		}

		m_device->device->waitIdle();

		m_colorImage.reset();
		m_depthImage.reset();
		m_swapchain.reset();


		createSwapchain();
		createColorResources();
		createDepthResources();

	}

	void RenderAPI::createCommandPool()
	{
		vk::Queue::FamilyIndices queueFamilyIndices = m_device->findQueueFamilies(m_device->physicalDevice->getVk());

		vk::Command::CreateInfo commandInfo{};
		commandInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		commandInfo.queue = m_device->graphicsQueue->getVk();
		commandInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		m_command = std::make_unique<vk::Command>(m_device->device->getVk(), commandInfo);


		m_vkCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_vkCommandBuffers[i] = m_command->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
	}

	void RenderAPI::createColorResources()
	{
		m_colorImage = std::make_unique<vk::Image>(vk::Image::createColorImage(
			m_device->device->getVk(),
			m_device->physicalDevice->getVk(),
			m_swapchain->swapchain->getExtent(),
			m_swapchain->swapchain->getImageFormat(),
			m_device->msaaSamples
		));

	}

	void RenderAPI::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();

		m_depthImage = std::make_unique<vk::Image>(vk::Image::createDepthImage(
			m_device->device->getVk(),
			m_device->physicalDevice->getVk(),
			m_swapchain->swapchain->getExtent(),
			depthFormat,
			m_device->msaaSamples
		));
	}

	void RenderAPI::createSyncObjects()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_swapchainUpdatedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		vk::core::Semaphore::CreateInfo semaphoreInfo{};

		vk::core::Fence::CreateInfo fenceInfo{};
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_imageAvailableSemaphores[i] = std::make_unique<vk::core::Semaphore>(m_device->device->getVk(), semaphoreInfo);
			m_renderFinishedSemaphores[i] = std::make_unique<vk::core::Semaphore>(m_device->device->getVk(), semaphoreInfo);
			m_swapchainUpdatedSemaphores[i] = std::make_unique<vk::core::Semaphore>(m_device->device->getVk(), semaphoreInfo);
			m_inFlightFences[i] = std::make_unique<vk::core::Fence>(m_device->device->getVk(), fenceInfo);
		}

	}


	VkFormat RenderAPI::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (const auto& format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_device->physicalDevice->getVk(), format, &props);

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
		vkGetPhysicalDeviceFormatProperties(m_device->physicalDevice->getVk(), format, &formatProperties);

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

	void RenderAPI::copyRenderedImageToSwapchainImage(uint32_t swapchainImageIndex)
	{

		// First, we need to transition the swap chain image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout, so we can copy the offscreen image to it.
		m_command->transitionImageLayout(
			m_swapchain->swapchain->getImage(swapchainImageIndex),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			0,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		// The offscreen image is allready in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL layout, so we can copy it to the swap chain image.
		VkCommandBuffer commandBuffer = m_command->beginSingleTimeCommands();

		// copy with blit
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {
			static_cast<int32_t>(m_colorImage->width()),
			static_cast<int32_t>(m_colorImage->height()),
			1
		};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {
			static_cast<int32_t>(m_swapchain->swapchain->getExtent().width),
			static_cast<int32_t>(m_swapchain->swapchain->getExtent().height),
			1
		};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			m_colorImage->image(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_swapchain->swapchain->getImage(swapchainImageIndex),
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
			m_imageAvailableSemaphores[m_currentFrame]->getVk(),
			m_renderFinishedSemaphores[m_currentFrame]->getVk()
		};
		VkPipelineStageFlags copyImageWaitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		copyImageInfo.waitSemaphoreCount = 2;
		copyImageInfo.pWaitSemaphores = copyImageWaitSemaphores;
		copyImageInfo.pWaitDstStageMask = copyImageWaitStages;

		VkSemaphore copyImageSignalSemaphores[] = {
			m_swapchainUpdatedSemaphores[m_currentFrame]->getVk()
		};
		copyImageInfo.signalSemaphoreCount = 1;
		copyImageInfo.pSignalSemaphores = copyImageSignalSemaphores;

		m_command->submit(1, &copyImageInfo, VK_NULL_HANDLE);

		m_command->queueWaitIdle();
		m_command->freeCommandBuffer(commandBuffer);

		// Now we need to transition the swap chain image to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout, so we can present it.
		m_command->transitionImageLayout(
			m_swapchain->swapchain->getImage(swapchainImageIndex),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			0,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);
	}



	void RenderAPI::startDraw()
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		m_inFlightFences[m_currentFrame]->wait();
		m_inFlightFences[m_currentFrame]->reset();

		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(cmd, &beginInfo);

		m_command->transitionImageLayout(
			m_colorImage->image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		);
	}

	void RenderAPI::startRendering()
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_colorImage->view();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};

		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.imageView = m_depthImage->view();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.clearValue = {1.0f, 0};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {0, 0, m_colorImage->width(), m_colorImage->height()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		vkCmdBeginRendering(cmd, &renderingInfo);
	}

	void RenderAPI::endRendering()
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdEndRendering(cmd);
	}

	void RenderAPI::endDraw()
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		m_command->transitionImageLayout(
			m_colorImage->image(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);

		vkEndCommandBuffer(cmd);

		VkSubmitInfo renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer commandBuffers[] = {m_vkCommandBuffers[m_currentFrame]};
		renderInfo.commandBufferCount = 1;
		renderInfo.pCommandBuffers = commandBuffers;

		VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]->getVk()};
		renderInfo.signalSemaphoreCount = 1;
		renderInfo.pSignalSemaphores = signalSemaphores;

		m_device->graphicsQueue->submit(1, &renderInfo, m_inFlightFences[m_currentFrame]->getVk());



		// Now instead of rendering directly to the swap chain image, we render to the offscreen image, and then copy it to the swap chain image.

		uint32_t imageIndex;
		VkResult result = m_swapchain->swapchain->acquireNextImage(
			UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame]->getVk(), VK_NULL_HANDLE, &imageIndex
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

		copyRenderedImageToSwapchainImage(imageIndex);

		// Finally, we present the swap chain image.
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore copyImageSignalSemaphores[] = {
			m_swapchainUpdatedSemaphores[m_currentFrame]->getVk()
		};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = copyImageSignalSemaphores;

		VkSwapchainKHR swapChains[] = {m_swapchain->swapchain->getVk()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_device->presentQueue->getVk(), &presentInfo);


		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image.");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}



	vk::Mesh::ID RenderAPI::loadModel(const std::string& filename)
	{
		vk::Mesh::CreateInfo meshInfo = {};

		vk::Mesh::readObjFile(filename, meshInfo.vertices, meshInfo.indices);

		m_meshMap[m_maxMeshID] = std::make_unique<vk::Mesh>(
			m_device->device->getVk(),
			m_device->physicalDevice->getVk(),
			*m_command.get(),
			meshInfo
		);

		return m_maxMeshID++;
	}

	Descriptor::ID RenderAPI::createDescriptor(VkDescriptorSetLayoutBinding layoutBinding)
	{
		vk::Descriptor::CreateInfo descriptorInfo{};
		descriptorInfo.bindings = { layoutBinding };
		descriptorInfo.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		m_descriptorMap[Descriptor::maxID] = std::make_unique<Descriptor>(
			m_device->device->getVk(),
			descriptorInfo
		);

		return Descriptor::maxID++;
	}

	Texture::ID RenderAPI::loadTexture(Texture::CreateInfo& createInfo)
	{
		m_textureMap[m_maxTextureID] = std::make_unique<Texture>(
			m_device->device->getVk(),
			m_device->physicalDevice->getVk(),
			*m_command.get(),
			createInfo
		);

		generateMipmaps(
			m_textureMap[m_maxTextureID]->image().image(),
			VK_FORMAT_R8G8B8A8_SRGB,
			m_textureMap[m_maxTextureID]->width(),
			m_textureMap[m_maxTextureID]->height(),
			m_textureMap[m_maxTextureID]->image().mipLevels()
		);

		return m_maxTextureID++;
	}

	Pipeline::ID RenderAPI::createPipeline(Pipeline::CreateInfo& createInfo)
	{
		VkFormat colorAttachementFormat = m_colorImage->format();
		VkPipelineRenderingCreateInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = &colorAttachementFormat;
		renderingInfo.depthAttachmentFormat = findDepthFormat();

		createInfo.pNext = &renderingInfo;

		m_pipelineMap[Pipeline::maxID] = std::make_unique<vk::Pipeline>(m_device->device->getVk(), createInfo);

		return Pipeline::maxID++;
	}

	UniformBuffer::ID RenderAPI::createUniformBuffer(UniformBuffer::CreateInfo& createInfo)
	{
		m_uniformBufferMap[UniformBuffer::maxID] = std::make_unique<UniformBuffer>(
			m_device->device->getVk(),
			m_device->physicalDevice->getVk(),
			createInfo
		);

		return UniformBuffer::maxID++;
	}

	void RenderAPI::bindPipeline(Pipeline::ID pipelineID)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineMap[pipelineID]->pipeline->getVk());
	}

	void RenderAPI::bindDescriptor(
		Pipeline::ID pipelineID,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const VkDescriptorSet *pDescriptorSets
	)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineMap[pipelineID]->layout->getVk(),
			firstSet, descriptorSetCount,
			pDescriptorSets,
			0, nullptr
		);
	}

	void RenderAPI::pushConstant(Pipeline::ID pipelineID, VkShaderStageFlags stageFlags, uint32_t size, const void* data)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdPushConstants(
			cmd,
			m_pipelineMap[pipelineID]->layout->getVk(),
			stageFlags,
			0,
			size,
			data
		);
	}

	void RenderAPI::setViewport(VkViewport& viewport)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdSetViewport(cmd, 0, 1, &viewport);
	}

	void RenderAPI::setScissor(VkRect2D& scissor)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void RenderAPI::drawMesh(vk::Mesh::ID meshID)
	{
		VkCommandBuffer cmd = m_vkCommandBuffers[m_currentFrame];

		VkBuffer vertexBuffers[] = {m_meshMap[meshID]->vertexBuffer().buffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(cmd, m_meshMap[meshID]->indexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, m_meshMap[meshID]->indexCount(), 1, 0, 0, 0);
	}


	GLFWwindow* RenderAPI::getWindow()
	{
		return m_device->window->getGLFWwindow();
	}

	uint32_t RenderAPI::currentFrame()
	{
		return m_currentFrame;
	}

	std::unique_ptr<vk::Mesh>& RenderAPI::getMesh(vk::Mesh::ID meshID)
	{
		return m_meshMap[meshID];
	}

	std::unique_ptr<vk::Descriptor>& RenderAPI::getDescriptor(Descriptor::ID descriptorID)
	{
		return m_descriptorMap[descriptorID];
	}

	std::unique_ptr<Texture>& RenderAPI::getTexture(Texture::ID textureID)
	{
		return m_textureMap[textureID];
	}

	std::unique_ptr<vk::UniformBuffer>& RenderAPI::getUniformBuffer(UniformBuffer::ID uniformBufferID)
	{
		return m_uniformBufferMap[uniformBufferID];
	}
}