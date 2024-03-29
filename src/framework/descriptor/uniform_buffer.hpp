#pragma once

#include "defines.hpp"
#include "framework/memory/buffer.hpp"
#include "framework/descriptor/descriptor.hpp"
#include "framework/command.hpp"

#include <memory>

namespace LIB_NAMESPACE
{
	class UniformBuffer
	{

	public:

		struct CreateInfo
		{
			VkDeviceSize size;
			VkShaderStageFlags stageFlags;
		};

		UniformBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const CreateInfo & createInfo
		);
		UniformBuffer(const UniformBuffer & other) = delete;
		UniformBuffer(UniformBuffer && other);
		UniformBuffer & operator=(const UniformBuffer & other) = delete;
		UniformBuffer & operator=(UniformBuffer && other) = delete;
		~UniformBuffer();

		Buffer* buffer(uint32_t index) const { return m_buffers[index].get(); }
		Descriptor* descriptor() const { return m_descriptor.get(); }

		int size() { return m_size; }

	private:

		std::vector<std::unique_ptr<Buffer>> m_buffers;
		std::unique_ptr<Descriptor> m_descriptor;

		VkDeviceSize m_size;

		void createBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const CreateInfo & createInfo
		);

		void createDescriptor(
			VkDevice device,
			const CreateInfo & createInfo
		);

	};
}