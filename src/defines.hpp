#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// Define project namespace
#ifndef LIB_NAMESPACE
#	define LIB_NAMESPACE vk
#endif

// #define NDEBUG

const int MAX_FRAMES_IN_FLIGHT = 2;

#define TROW(message, vkResult) throw std::runtime_error(std::string(message) + " (" + std::string(string_VkResult(vkResult)) + ")");

#define VK_CHECK(function, message) \
	{ \
		VkResult result = function; \
		if (result != VK_SUCCESS) \
		{ \
			TROW(message, result); \
		} \
	}
