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

		uint64_t loadModel(const std::string & filename);
		uint64_t createPipeline(Pipeline::CreateInfo & createInfo);
		uint64_t createDescriptor(VkDescriptorSetLayoutBinding layoutBinding);
		uint64_t loadTexture(Texture::CreateInfo & createInfo);
		uint64_t createUniformBuffer(const UniformBuffer::CreateInfo & create_info);

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
		Mesh & getMesh(uint32_t meshID);
		Descriptor & getDescriptor(uint64_t descriptorID);
		Texture & getTexture(uint64_t textureID);
		UniformBuffer & getUniformBuffer(uint64_t uniform_buffer_id);

	private:

		Device m_device;

		std::unique_ptr<Command> m_command;
		std::vector<VkCommandBuffer> m_vkCommandBuffers;


		std::unique_ptr<Swapchain> m_swapchain;

		std::unique_ptr<Image> m_colorImage;
		std::unique_ptr<Image> m_depthImage;

		std::vector<std::unique_ptr<core::Semaphore>> m_imageAvailableSemaphores;
		std::vector<std::unique_ptr<core::Semaphore>> m_renderFinishedSemaphores;
		std::vector<std::unique_ptr<core::Semaphore>> m_swapchainUpdatedSemaphores;
		std::vector<std::unique_ptr<core::Fence>> m_inFlightFences;


		Map<Descriptor> m_descriptor_map;

		Map<Pipeline> m_pipeline_map;

		Map<Mesh> m_mesh_map;

		Map<Texture> m_texture_map;

		Map<UniformBuffer> m_uniform_buffer_map;


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
