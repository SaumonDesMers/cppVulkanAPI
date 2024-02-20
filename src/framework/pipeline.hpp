#pragma once

#include "defines.hpp"

#include "core/pipeline/graphic_pipeline.hpp"
#include "core/pipeline/pipeline_layout.hpp"

#include <memory>
#include <string>
#include <vector>

namespace LIB_NAMESPACE
{
	class Pipeline
	{

	public:

		struct CreateInfo
		{
			std::string vertex_shader_path;
			std::string fragment_shader_path;

			std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
			std::vector<VkPushConstantRange> push_constant_ranges;

			std::vector<uint64_t> color_target_ids;
			uint64_t depth_target_id;

			void* pNext = nullptr;
		};

		std::unique_ptr<core::PipelineLayout> layout;
		std::unique_ptr<core::Pipeline> pipeline;

		Pipeline(VkDevice device, const CreateInfo& create_info);
		Pipeline(const Pipeline &) = delete;
		Pipeline(Pipeline && other);
		~Pipeline();

	private:

		void createPipeline(VkDevice device, const CreateInfo& create_info);

	};
}