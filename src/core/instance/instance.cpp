#include "instance.hpp"
#include "debug.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>

namespace LIB_NAMESPACE
{
	namespace core
	{

		Instance::Instance(
			const std::vector<const char*> & enabled_layer_names,
			const std::vector<const char*> & enabled_extension_names
		)
		{

			#ifndef NDEBUG
				if (core::Instance::checkValidationLayerSupport(enabled_layer_names) == false)
				{
					throw std::runtime_error("Validation layers requested, but not available");
				}
			#endif

			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "Application";
			app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.pEngineName = "Engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_3;

			VkInstanceCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &app_info;

			create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
			create_info.ppEnabledExtensionNames = enabled_extension_names.data();

			#ifndef NDEBUG
				create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
				create_info.ppEnabledLayerNames = enabled_layer_names.data();

				VkDebugUtilsMessengerCreateInfoEXT debug_create_info = DebugMessenger::defaultCreateInfo();
				create_info.pNext = &debug_create_info;
			#else
				(void)enabled_layer_names;
				create_info.enabledLayerCount = 0;
			#endif

			VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_instance), "Failed to create instance.");
		}

		Instance::Instance(const VkInstanceCreateInfo & create_info)
		{
			VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_instance), "Failed to create instance.");
		}

		Instance::~Instance()
		{
			vkDestroyInstance(m_instance, nullptr);
		}

		std::vector<VkPhysicalDevice> Instance::getPhysicalDevices() const
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

			return devices;
		}

		bool Instance::checkValidationLayerSupport(const std::vector<const char*> & validation_layers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validation_layers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (layerFound == false)
				{
					return false;
				}
			}

			return true;
		}
	}
}