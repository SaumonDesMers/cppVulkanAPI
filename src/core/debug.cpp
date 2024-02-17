#include "debug.hpp"

#include <iostream>
#include <stdexcept>

namespace LIB_NAMESPACE
{
	namespace core
	{

		DebugMessenger::DebugMessenger(VkInstance instance):
			m_instance(instance)
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info = defaultCreateInfo();
			VK_CHECK(
				createDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debugMessenger),
				"Failed to set up debug messenger."
			);
		}

		DebugMessenger::DebugMessenger(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT & createInfo
		):
			m_instance(instance)
		{
			VK_CHECK(
				createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger),
				"Failed to set up debug messenger."
			);
		}

		DebugMessenger::~DebugMessenger()
		{
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		VkDebugUtilsMessengerCreateInfoEXT DebugMessenger::defaultCreateInfo()
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity =
				// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			create_info.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			create_info.pfnUserCallback = debugCallback;

			return create_info;
		}

		VkResult DebugMessenger::createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT * createInfo,
			const VkAllocationCallbacks * pAllocator,
			VkDebugUtilsMessengerEXT * pDebugMessenger
		)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				return func(instance, createInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DebugMessenger::destroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks * pAllocator
		)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func(instance, debugMessenger, pAllocator);
			}
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData,
			void* pUserData
		)
		{
			(void)messageSeverity;
			(void)messageType;
			(void)pUserData;

			std::cerr << pCallbackData->pMessage << "\n" << std::endl;

			return VK_FALSE;
		}

	}
}