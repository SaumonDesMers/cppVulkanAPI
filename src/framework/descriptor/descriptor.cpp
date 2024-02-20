#include "descriptor.hpp"

#include <stdexcept>

namespace LIB_NAMESPACE
{
	Descriptor::Descriptor(VkDevice device, const CreateInfo& createInfo):
		m_device(device)
	{
		if (createInfo.bindings.empty())
		{
			throw std::runtime_error("Cannot create descriptor with 0 binding.");
		}

		createLayout(createInfo);
		createPool(createInfo);
		createSets(createInfo);
	}

	Descriptor::Descriptor(Descriptor&& other):
		m_pool(other.m_pool),
		m_layout(other.m_layout),
		m_sets(std::move(other.m_sets)),
		m_device(other.m_device)
	{
		other.m_pool = VK_NULL_HANDLE;
		other.m_layout = VK_NULL_HANDLE;
	}

	Descriptor::~Descriptor()
	{
		vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
		vkDestroyDescriptorPool(m_device, m_pool, nullptr);
	}

	void Descriptor::createLayout(const CreateInfo& createInfo)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(createInfo.bindings.size());
		layoutInfo.pBindings = createInfo.bindings.data();

		if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_layout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout.");
		}
	}

	void Descriptor::createPool(const CreateInfo& createInfo)
	{
		std::vector<VkDescriptorPoolSize> poolSizes(createInfo.bindings.size());

		for (size_t i = 0; i < createInfo.bindings.size(); i++)
		{
			poolSizes[i].type = createInfo.bindings[i].descriptorType;
			poolSizes[i].descriptorCount = createInfo.descriptor_count;
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = createInfo.descriptor_count;

		if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool.");
		}
	}

	void Descriptor::createSets(const CreateInfo& createInfo)
	{
		std::vector<VkDescriptorSetLayout> layouts(createInfo.descriptor_count, m_layout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(createInfo.descriptor_count);
		allocInfo.pSetLayouts = layouts.data();

		m_sets.resize(createInfo.descriptor_count);

		VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, m_sets.data());
		if (result != VK_SUCCESS)
		{
			TROW("failed to allocate descriptor sets", result)
		}
	}
}