cmake_minimum_required(VERSION 3.15)

project(cppVulkanAPI
	VERSION 0.1
	DESCRIPTION "Wrapper for c++ vulkan API"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		-g
		-Wall
		-Wextra
		-Werror
		-Wpedantic
		-Wno-missing-braces
)

target_sources(${PROJECT_NAME}
	PUBLIC
		src/core/instance.cpp
		src/core/debug.cpp
		src/core/physical_device.cpp
		src/core/device.cpp
		src/core/queue.cpp
		src/core/swapchain.cpp
		src/core/image/image.cpp
		src/core/image/image_view.cpp
		src/core/image/sampler.cpp
		src/core/pipeline/graphic_pipeline.cpp
		src/core/pipeline/shader_module.cpp
		src/core/pipeline/pipeline_layout.cpp
		src/core/pipeline/render_pass.cpp
		src/core/pipeline/descriptor_layout.cpp
		src/core/pipeline/descriptor_pool.cpp
		src/core/pipeline/descriptor_set.cpp
		src/core/frame_buffer.cpp
		src/core/command_pool.cpp
		src/core/command_buffer.cpp
		src/core/sync_object.cpp
		src/core/buffer.cpp
		src/core/device_memory.cpp

		src/framework/window/window.cpp
		src/framework/device.cpp
		src/framework/swapchain.cpp
		src/framework/pipeline.cpp
		src/framework/descriptor/descriptor.cpp
		src/framework/descriptor/texture.cpp
		src/framework/descriptor/uniform_buffer.cpp
		src/framework/command.cpp
		src/framework/memory/buffer.cpp
		src/framework/memory/image.cpp
		src/framework/object/mesh.cpp
		src/framework/render_api.cpp
		src/framework/spirv/parser.cpp
)

# download glm
add_custom_target(
	glm
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/download_glm.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_dependencies(${PROJECT_NAME} glm)

target_link_libraries(${PROJECT_NAME} glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)

target_include_directories(${PROJECT_NAME}
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/external/glm
		${CMAKE_CURRENT_SOURCE_DIR}/external/stb
		${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader
)