#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace LIB_NAMESPACE
{
	namespace core
	{
		class Instance
		{

		public:

			struct CreateInfo: public VkInstanceCreateInfo
			{
				CreateInfo(): VkInstanceCreateInfo()
				{
					sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				}
			};

			Instance(
				const std::vector<const char*> & enabled_layer_names,
				const std::vector<const char*> & enabled_extension_names
			);
			Instance(const VkInstanceCreateInfo & create_info);
			~Instance();

			VkInstance getVk() const { return m_instance; }

			std::vector<VkPhysicalDevice> getPhysicalDevices() const;

			static bool checkValidationLayerSupport(const std::vector<const char*> & validation_layers);

		private:

			VkInstance m_instance;

		};
	}
}