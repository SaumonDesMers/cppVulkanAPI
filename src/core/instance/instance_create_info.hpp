#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <vulkan/vulkan.h>

#include <iostream>
#include <memory>
#include <vector>

namespace LIB_NAMESPACE
{
	namespace core
	{
		struct ApplicationInfo: public VkApplicationInfo
		{
			ApplicationInfo(): VkApplicationInfo()
			{
				sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			}
		};

		class InstanceCreateInfo: public VkInstanceCreateInfo
		{
		
		public:

			InstanceCreateInfo(): VkInstanceCreateInfo()
			{
				sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				pApplicationInfo = &applicationInfo;
			}

			InstanceCreateInfo(const InstanceCreateInfo & other) = delete;

			InstanceCreateInfo(InstanceCreateInfo && other):
				VkInstanceCreateInfo(other),
				applicationInfo(other.applicationInfo),
				debugMessengerCreateInfo(other.debugMessengerCreateInfo)
			{
				m_enabledLayerNames = std::move(other.m_enabledLayerNames);
				m_enabledExtensionNames = std::move(other.m_enabledExtensionNames);
			}

			void setEnabledLayerNames(const std::vector<const char*> & enabledLayerNames)
			{
				m_enabledLayerNames = enabledLayerNames;
				enabledLayerCount = static_cast<uint32_t>(m_enabledLayerNames.size());
				ppEnabledLayerNames = m_enabledLayerNames.data();
			}

			void setEnabledExtensionNames(const std::vector<const char*> & enabledExtensionNames)
			{
				m_enabledExtensionNames = enabledExtensionNames;
				enabledExtensionCount = static_cast<uint32_t>(m_enabledExtensionNames.size());
				ppEnabledExtensionNames = m_enabledExtensionNames.data();
			}

			ApplicationInfo applicationInfo;
			DebugMessenger::CreateInfo debugMessengerCreateInfo;
			
		private:

			std::vector<const char*> m_enabledLayerNames;
			std::vector<const char*> m_enabledExtensionNames;

		};
	}
}