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
		uint64_t newPipeline(Pipeline::CreateInfo & createInfo);
		uint64_t newDescriptor(VkDescriptorSetLayoutBinding layoutBinding);
		uint64_t loadTexture(Texture::CreateInfo & createInfo);
		uint64_t newUniformBuffer(const UniformBuffer::CreateInfo & create_info);
		uint64_t newColorTarget();
		uint64_t newDepthTarget();

		// function to start recording a command buffer
		void startDraw();
		// function to start a render pass
		void startRendering(
			const std::vector<uint64_t> & color_target_ids,
			uint64_t depth_target_id
		);
		// function to do the actual drawing
		void bindPipeline(uint64_t pipelineID);
		void drawMesh(uint64_t meshID);
		void bindDescriptor(
			uint64_t pipelineID,
			uint32_t firstSet,
			uint32_t descriptorSetCount,
			const VkDescriptorSet *pDescriptorSets
		);
		void pushConstant(uint64_t pipelineID, VkShaderStageFlags stageFlags, uint32_t size, const void* data);
		void setViewport(VkViewport& viewport);
		void setScissor(VkRect2D& scissor);

		// function to end a render pass
		void endRendering();
		// function to end recording a command buffer
		void endDraw(uint64_t color_target_id);

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
		std::vector<VkCommandBuffer> m_vk_command_buffers;


		std::unique_ptr<Swapchain> m_swapchain;

		Map<Image> m_color_target_map;
		Map<Image> m_depth_target_map;

		std::vector<std::unique_ptr<core::Semaphore>> m_image_available_semaphores;
		std::vector<std::unique_ptr<core::Semaphore>> m_render_finished_semaphores;
		std::vector<std::unique_ptr<core::Semaphore>> m_swapchain_updated_semaphores;
		std::vector<std::unique_ptr<core::Fence>> m_in_flight_fences;


		Map<Descriptor> m_descriptor_map;

		Map<Pipeline> m_pipeline_map;

		Map<Mesh> m_mesh_map;

		Map<Texture> m_texture_map;

		Map<UniformBuffer> m_uniform_buffer_map;


		uint32_t m_current_frame = 0;

		std::mutex m_global_mutex;


		void createSwapchain();
		void recreateSwapChain();
		void createCommandPool();
		void createSyncObjects();

		uint64_t createColorTarget(uint64_t id = Map<Image>::no_id);
		uint64_t createDepthTarget(uint64_t id = Map<Image>::no_id);

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();
		bool hasStencilComponent(VkFormat format);

		void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void copyRenderedImageToSwapchainImage(uint64_t color_target_id, uint32_t swapchain_image_index);
	};
}
