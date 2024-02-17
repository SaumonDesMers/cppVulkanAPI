#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

#include <functional>

namespace LIB_NAMESPACE
{
	namespace core
	{
		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		class DebugMessengerCreateInfo: public VkDebugUtilsMessengerCreateInfoEXT
		{

		public:

			DebugMessengerCreateInfo(): VkDebugUtilsMessengerCreateInfoEXT()
			{
				sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				pfnUserCallback = debugCallback;
			}
		};

		class DebugMessenger
		{

		public:

			DebugMessenger(VkInstance instance);

			DebugMessenger(
				VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT & createInfo
			);

			~DebugMessenger();

			static VkDebugUtilsMessengerCreateInfoEXT defaultCreateInfo();
		
		private:

			VkDebugUtilsMessengerEXT m_debugMessenger;
			VkInstance m_instance;

			static VkResult createDebugUtilsMessengerEXT(
				VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
				const VkAllocationCallbacks * pAllocator,
				VkDebugUtilsMessengerEXT * pDebugMessenger
			);

			static void destroyDebugUtilsMessengerEXT(
				VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks * pAllocator
			);
		};
	}
}