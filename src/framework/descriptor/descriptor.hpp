#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace LIB_NAMESPACE
{
	class Descriptor
	{

	public:

		struct CreateInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			uint32_t descriptor_count;
		};

		Descriptor(VkDevice device, const CreateInfo & create_info);
		Descriptor(const Descriptor & other) = delete;
		Descriptor(Descriptor && other);
		Descriptor & operator=(const Descriptor & other) = delete;
		Descriptor & operator=(Descriptor && other) = delete;
		~Descriptor();

		VkDescriptorPool pool() { return m_pool; }
		VkDescriptorSetLayout layout() { return m_layout; }
		VkDescriptorSet set(uint32_t index) { return m_sets.at(index); }
		VkDescriptorSet* pSet(uint32_t index) { return &m_sets.at(index); }
	
	private:

		VkDescriptorPool m_pool;
		VkDescriptorSetLayout m_layout;
		std::vector<VkDescriptorSet> m_sets;

		VkDevice m_device;

		void createPool(const CreateInfo & create_info);
		void createLayout(const CreateInfo & create_info);
		void createSets(const CreateInfo & create_info);

	};
}