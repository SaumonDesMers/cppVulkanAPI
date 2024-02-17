#pragma once

#include "defines.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "descriptor/descriptor.hpp"
#include "descriptor/uniform_buffer.hpp"
#include "descriptor/texture.hpp"
#include "command.hpp"
#include "pipeline.hpp"
#include "memory/image.hpp"
#include "memory/buffer.hpp"
#include "core/image/sampler.hpp"
#include "core/sync_object.hpp"
#include "object/mesh.hpp"
#include "map.hpp"

#include <glm/glm.hpp>

#include <stdexcept>
#include <memory>
#include <optional>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>

struct ViewProj_UBO {
	glm::mat4 view;
	glm::mat4 proj;
};

namespace LIB_NAMESPACE
{
	class RenderAPI
	{

	public:

		RenderAPI(GLFWwindow *glfwWindow);
		~RenderAPI();

		Mesh::ID loadModel(const std::string& filename);
		Pipeline::ID createPipeline(Pipeline::CreateInfo& createInfo);
		Descriptor::ID createDescriptor(VkDescriptorSetLayoutBinding layoutBinding);
		Texture::ID loadTexture(Texture::CreateInfo& createInfo);
		UniformBuffer::ID createUniformBuffer(UniformBuffer::CreateInfo& createInfo);

		// function to start recording a command buffer
		void startDraw();
		// function to start a render pass
		void startRendering();
		// function to do the actual drawing
		void bindPipeline(Pipeline::ID pipelineID);
		void drawMesh(Mesh::ID meshID);
		void bindDescriptor(
			Pipeline::ID pipelineID,
			uint32_t firstSet,
			uint32_t descriptorSetCount,
			const VkDescriptorSet *pDescriptorSets
		);
		void pushConstant(Pipeline::ID pipelineID, VkShaderStageFlags stageFlags, uint32_t size, const void* data);
		void setViewport(VkViewport& viewport);
		void setScissor(VkRect2D& scissor);

		// function to end a render pass
		void endRendering();
		// function to end recording a command buffer
		void endDraw();

		// temporary functions to access private members
		GLFWwindow* getWindow();
		uint32_t currentFrame();
		std::unique_ptr<vk::Mesh>& getMesh(Mesh::ID meshID);
		std::unique_ptr<vk::Descriptor>& getDescriptor(Descriptor::ID descriptorID);
		std::unique_ptr<Texture>& getTexture(Texture::ID textureID);
		std::unique_ptr<vk::UniformBuffer>& getUniformBuffer(UniformBuffer::ID uniformBufferID);

	private:

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		Device m_device;

		std::unique_ptr<vk::Swapchain> m_swapchain;

		std::map<Descriptor::ID, std::unique_ptr<vk::Descriptor>> m_descriptorMap;

		std::unique_ptr<vk::Command> m_command;
		std::vector<VkCommandBuffer> m_vkCommandBuffers;

		std::map<Pipeline::ID, std::unique_ptr<vk::Pipeline>> m_pipelineMap;

		std::unique_ptr<vk::Image> m_colorImage;
		std::unique_ptr<vk::Image> m_depthImage;

		std::vector<std::unique_ptr<vk::core::Semaphore>> m_imageAvailableSemaphores;
		std::vector<std::unique_ptr<vk::core::Semaphore>> m_renderFinishedSemaphores;
		std::vector<std::unique_ptr<vk::core::Semaphore>> m_swapchainUpdatedSemaphores;
		std::vector<std::unique_ptr<vk::core::Fence>> m_inFlightFences;

		Mesh::ID m_maxMeshID = 0;
		std::map<Mesh::ID, std::unique_ptr<vk::Mesh>> m_meshMap;

		uint32_t m_mipLevels;
		Texture::ID m_maxTextureID;
		std::map<Texture::ID, std::unique_ptr<Texture>> m_textureMap;

		std::map<UniformBuffer::ID, std::unique_ptr<vk::UniformBuffer>> m_uniformBufferMap;
		// vk::Map<vk::UniformBuffer> m_uniformBufferMap;

		bool m_framebufferResized = false;

		uint32_t m_currentFrame = 0;

		std::mutex m_global_mutex;


		void createSwapchain();
		void recreateSwapChain();
		void createCommandPool();
		void createColorResources();
		void createDepthResources();
		void createSyncObjects();

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);

		void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void copyRenderedImageToSwapchainImage(uint32_t swapchainImageIndex);
	};
}
