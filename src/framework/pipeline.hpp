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

		typedef uint32_t ID;

		struct CreateInfo
		{
			std::string vertexShaderPath;
			std::string fragmentShaderPath;

			VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			std::vector<VkPushConstantRange> pushConstantRanges;

			void* pNext = nullptr;
		};

		static inline ID maxID = 0;

		std::unique_ptr<core::PipelineLayout> layout;
		std::unique_ptr<core::Pipeline> pipeline;

		Pipeline(VkDevice device, const CreateInfo& createInfo);
		Pipeline(const Pipeline &) = delete;
		Pipeline(Pipeline && other);
		~Pipeline();

	private:

		void createPipeline(VkDevice device, const CreateInfo& createInfo);

	};
}