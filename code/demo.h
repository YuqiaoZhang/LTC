#ifndef _DEMO_H_
#define _DEMO_H_ 1

#include <sdkddkver.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR 1
#include "../vulkansdk/include/vulkan/vulkan.h"
class Demo
{
	VkFramebuffer m_framebuffer;

public:
	void Init(VkDevice vulkan_device, uint32_t vulkan_swapchain_image_count, VkImage *vulkan_swapchain_images, VkFormat vulkan_swapchain_image_format);
	void Tick(VkCommandBuffer vulkan_command_buffer, uint32_t vulkan_swapchain_image_index);
};

#endif