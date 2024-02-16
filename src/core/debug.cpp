#include "debug.hpp"

#include <iostream>
#include <stdexcept>

namespace LIB_NAMESPACE
{
	namespace core
	{
		DebugMessenger::DebugMessenger(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT & createInfo
		):
			m_instance(instance)
		{
			// createInfo.pfnUserCallback = debugCallback;

			if (createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to set up debug messenger.");
			}
		}

		DebugMessenger::~DebugMessenger()
		{
			destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
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