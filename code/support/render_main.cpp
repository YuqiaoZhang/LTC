#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <windows.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR 1
#include "../../vulkansdk/include/vulkan/vulkan.h"

#include "window_main.h"

#include "render_main.h"

#include "frame_throttling.h"

#include "resolution.h"

#include "../demo.h"

static VkBool32 VKAPI_CALL vulkan_debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

unsigned __stdcall render_main(void *pVoid)
{
	HWND hWnd = static_cast<HWND>(pVoid);

	VkAllocationCallbacks *vulkan_allocation_callbacks = NULL;

#ifndef NDEBUG
	{
		WCHAR file_name[4096];
		DWORD res_get_file_name = GetModuleFileNameW(NULL, file_name, sizeof(file_name) / sizeof(file_name[0]));
		assert(0U != res_get_file_name);

		for (int i = (res_get_file_name - 1); i > 0; --i)
		{
			if (L'\\' == file_name[i])
			{
				file_name[i] = L'\0';
				break;
			}
		}

		BOOL res_set_environment_variable = SetEnvironmentVariableW(L"VK_LAYER_PATH", file_name);
		assert(FALSE != res_set_environment_variable);
	}
#endif

	PFN_vkGetInstanceProcAddr pfn_vk_get_instance_proc_addr = vkGetInstanceProcAddr;

	VkInstance vulkan_instance = VK_NULL_HANDLE;
	{
		PFN_vkCreateInstance pfn_vk_create_instance = reinterpret_cast<PFN_vkCreateInstance>(pfn_vk_get_instance_proc_addr(VK_NULL_HANDLE, "vkCreateInstance"));
		assert(NULL != pfn_vk_create_instance);

		VkApplicationInfo application_info = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			NULL,
			"Windows-Vulkan-Demo",
			0,
			"Windows-Vulkan-Demo",
			0,
			VK_API_VERSION_1_0};

#ifndef NDEBUG
		char const *enabled_layer_names[] = {
			"VK_LAYER_KHRONOS_validation"};
		char const *enabled_extension_names[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#else
		char const *enabled_extension_names[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#endif

		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			NULL,
			0U,
			&application_info,
#ifndef NDEBUG
			sizeof(enabled_layer_names) / sizeof(enabled_layer_names[0]),
			enabled_layer_names,
#else
			0U,
			NULL,
#endif
			sizeof(enabled_extension_names) / sizeof(enabled_extension_names[0]),
			enabled_extension_names};

		VkResult res_create_instance = pfn_vk_create_instance(&instance_create_info, vulkan_allocation_callbacks, &vulkan_instance);
		assert(VK_SUCCESS == res_create_instance);
	}

	pfn_vk_get_instance_proc_addr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetInstanceProcAddr"));
	assert(NULL != pfn_vk_get_instance_proc_addr);

#ifndef NDEBUG
	VkDebugUtilsMessengerEXT vulkan_messenge;
	{
		PFN_vkCreateDebugUtilsMessengerEXT pfn_vk_create_debug_utils_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT"));
		assert(NULL != pfn_vk_create_debug_utils_messenger);

		VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info =
			{
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				0U,
				0U,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				vulkan_debug_utils_messenger_callback,
				NULL};

		VkResult res_create_debug_utils_messenger = pfn_vk_create_debug_utils_messenger(vulkan_instance, &debug_utils_messenger_create_info, vulkan_allocation_callbacks, &vulkan_messenge);
		assert(VK_SUCCESS == res_create_debug_utils_messenger);
	}
#endif

	VkPhysicalDevice vulkan_physical_device = VK_NULL_HANDLE;
	{
		PFN_vkEnumeratePhysicalDevices pfn_vk_enumerate_physical_devices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkEnumeratePhysicalDevices"));
		assert(NULL != pfn_vk_enumerate_physical_devices);
		PFN_vkGetPhysicalDeviceProperties pfn_vk_get_physical_device_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceProperties"));
		assert(NULL != pfn_vk_get_physical_device_properties);

		uint32_t physical_device_count_1 = uint32_t(-1);
		VkResult res_enumerate_physical_devices_1 = pfn_vk_enumerate_physical_devices(vulkan_instance, &physical_device_count_1, NULL);
		assert(VK_SUCCESS == res_enumerate_physical_devices_1 && 0U < physical_device_count_1);

		VkPhysicalDevice *physical_devices = static_cast<VkPhysicalDevice *>(malloc(sizeof(VkPhysicalDevice) * physical_device_count_1));
		assert(NULL != physical_devices);

		uint32_t physical_device_count_2 = physical_device_count_1;
		VkResult res_enumerate_physical_devices_2 = pfn_vk_enumerate_physical_devices(vulkan_instance, &physical_device_count_2, physical_devices);
		assert(VK_SUCCESS == res_enumerate_physical_devices_2 && physical_device_count_1 == physical_device_count_2);

		// The lower index may imply the user preference (e.g. VK_LAYER_MESA_device_select)
		uint32_t physical_device_index_first_discrete_gpu = uint32_t(-1);
		uint32_t physical_device_index_first_integrated_gpu = uint32_t(-1);
		for (uint32_t physical_device_index = 0; (uint32_t(-1) == physical_device_index_first_discrete_gpu) && (physical_device_index < physical_device_count_2); ++physical_device_index)
		{
			VkPhysicalDeviceProperties physical_device_properties;
			pfn_vk_get_physical_device_properties(physical_devices[physical_device_index], &physical_device_properties);

			if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == physical_device_properties.deviceType)
			{
				physical_device_index_first_discrete_gpu = physical_device_index;
			}
			else if (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU == physical_device_properties.deviceType)
			{
				physical_device_index_first_integrated_gpu = physical_device_index;
			}
		}

		// The discrete gpu is preferred
		if (uint32_t(-1) != physical_device_index_first_discrete_gpu)
		{
			vulkan_physical_device = physical_devices[physical_device_index_first_discrete_gpu];
		}
		else if (uint32_t(-1) != physical_device_index_first_integrated_gpu)
		{
			vulkan_physical_device = physical_devices[physical_device_index_first_integrated_gpu];
		}
		else
		{
			assert(false);
		}

		free(physical_devices);
	}

	// https://github.com/ValveSoftware/dxvk
	// src/dxvk/dxvk_device.h
	// DxvkDevice::hasDedicatedTransferQueue
	bool vulkan_has_dedicated_transfer_queue = false;
	// nvpro-samples/shared_sources/nvvk/context_vk.cpp
	// Context::initDevice
	// m_queue_GCT
	// m_queue_CT
	// m_queue_transfer
	uint32_t vulkan_queue_graphics_family_index = VK_QUEUE_FAMILY_IGNORED;
	uint32_t vulkan_queue_transfer_family_index = VK_QUEUE_FAMILY_IGNORED;
	uint32_t vulkan_queue_graphics_queue_index = uint32_t(-1);
	uint32_t vulkan_queue_transfer_queue_index = uint32_t(-1);
	{
		PFN_vkGetPhysicalDeviceQueueFamilyProperties pfn_vk_get_physical_device_queue_family_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceQueueFamilyProperties"));
		assert(NULL != pfn_vk_get_physical_device_queue_family_properties);
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR pfn_vk_get_physical_device_win32_presentation_support = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
		assert(NULL != pfn_vk_get_physical_device_win32_presentation_support);

		uint32_t queue_family_property_count_1 = uint32_t(-1);
		pfn_vk_get_physical_device_queue_family_properties(vulkan_physical_device, &queue_family_property_count_1, NULL);

		VkQueueFamilyProperties *queue_family_properties = static_cast<VkQueueFamilyProperties *>(malloc(sizeof(VkQueueFamilyProperties) * queue_family_property_count_1));
		assert(NULL != queue_family_properties);

		uint32_t queue_family_property_count_2 = queue_family_property_count_1;
		pfn_vk_get_physical_device_queue_family_properties(vulkan_physical_device, &queue_family_property_count_2, queue_family_properties);
		assert(queue_family_property_count_1 == queue_family_property_count_2);

		// TODO: support seperated present queue
		// src/dxvk/dxvk_adapter.cpp
		// DxvkAdapter::findQueueFamilies
		// src/d3d11/d3d11_swapchain.cpp
		// D3D11SwapChain::CreatePresenter

		for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_property_count_2; ++queue_family_index)
		{
			VkQueueFamilyProperties queue_family_property = queue_family_properties[queue_family_index];

			if ((queue_family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT) && pfn_vk_get_physical_device_win32_presentation_support(vulkan_physical_device, queue_family_index))
			{
				vulkan_queue_graphics_family_index = queue_family_index;
				vulkan_queue_graphics_queue_index = 0U;
				break;
			}
		}

		if (VK_QUEUE_FAMILY_IGNORED != vulkan_queue_graphics_family_index)
		{
			// Find transfer queues
			for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_property_count_2; ++queue_family_index)
			{
				if ((vulkan_queue_graphics_family_index != queue_family_index) && (VK_QUEUE_TRANSFER_BIT == (queue_family_properties[queue_family_index].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))))
				{
					vulkan_queue_transfer_family_index = queue_family_index;
					vulkan_queue_transfer_queue_index = 0U;
					vulkan_has_dedicated_transfer_queue = true;
					break;
				}
			}

			// Fallback to other graphics/compute queues
			// By vkspec, "either GRAPHICS or COMPUTE implies TRANSFER". This means TRANSFER is optional.
			if (VK_QUEUE_FAMILY_IGNORED == vulkan_queue_transfer_family_index)
			{
				for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_property_count_2; ++queue_family_index)
				{
					if ((vulkan_queue_graphics_family_index != queue_family_index) && (0 != (queue_family_properties[queue_family_index].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))))
					{
						vulkan_queue_transfer_family_index = queue_family_index;
						vulkan_queue_transfer_queue_index = 0U;
						vulkan_has_dedicated_transfer_queue = true;
						break;
					}
				}
			}

			// Try the same queue family
			if (VK_QUEUE_FAMILY_IGNORED == vulkan_queue_transfer_family_index)
			{
				if (2U <= queue_family_properties[vulkan_queue_graphics_family_index].queueCount)
				{
					vulkan_queue_transfer_family_index = vulkan_queue_graphics_family_index;
					vulkan_queue_transfer_queue_index = 1U;
					vulkan_has_dedicated_transfer_queue = true;
				}
				else
				{
					vulkan_queue_transfer_family_index = VK_QUEUE_FAMILY_IGNORED;
					vulkan_queue_transfer_queue_index = uint32_t(-1);
					vulkan_has_dedicated_transfer_queue = false;
				}
			}
		}

		assert(VK_QUEUE_FAMILY_IGNORED != vulkan_queue_graphics_family_index && (!vulkan_has_dedicated_transfer_queue || VK_QUEUE_FAMILY_IGNORED != vulkan_queue_transfer_family_index));

		free(queue_family_properties);
	}

	VkSurfaceKHR vulkan_surface = VK_NULL_HANDLE;
	{
		PFN_vkCreateWin32SurfaceKHR pfn_vk_create_win32_surface = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkCreateWin32SurfaceKHR"));
		assert(NULL != pfn_vk_create_win32_surface);

		VkWin32SurfaceCreateInfoKHR win32_surface_create_info;
		win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32_surface_create_info.pNext = NULL;
		win32_surface_create_info.flags = 0U;
		win32_surface_create_info.hinstance = reinterpret_cast<HINSTANCE>(GetClassLongPtrW(hWnd, GCLP_HMODULE));
		win32_surface_create_info.hwnd = hWnd;
		VkResult res_create_win32_surface = pfn_vk_create_win32_surface(vulkan_instance, &win32_surface_create_info, vulkan_allocation_callbacks, &vulkan_surface);
		assert(VK_SUCCESS == res_create_win32_surface);
	}

	VkFormat vulkan_swapchain_image_format = VK_FORMAT_UNDEFINED;
	{
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfn_get_physical_device_surface_formats = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		assert(NULL != pfn_get_physical_device_surface_formats);

		uint32_t surface_format_count_1 = uint32_t(-1);
		pfn_get_physical_device_surface_formats(vulkan_physical_device, vulkan_surface, &surface_format_count_1, NULL);

		VkSurfaceFormatKHR *surface_formats = static_cast<VkSurfaceFormatKHR *>(malloc(sizeof(VkQueueFamilyProperties) * surface_format_count_1));
		assert(NULL != surface_formats);

		uint32_t surface_format_count_2 = surface_format_count_1;
		pfn_get_physical_device_surface_formats(vulkan_physical_device, vulkan_surface, &surface_format_count_2, surface_formats);
		assert(surface_format_count_1 == surface_format_count_2);

		assert(surface_format_count_2 >= 1U);
		if (VK_FORMAT_UNDEFINED != surface_formats[0].format)
		{
			vulkan_swapchain_image_format = surface_formats[0].format;
		}
		else
		{
			vulkan_swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
		}

		assert(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == surface_formats[0].colorSpace);

		free(surface_formats);
	}

	uint32_t vulkan_framebuffer_width = uint32_t(-1);
	uint32_t vulkan_framebuffer_height = uint32_t(-1);
	{
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfn_get_physical_device_surface_capabilities = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		assert(NULL != pfn_get_physical_device_surface_capabilities);

		VkSurfaceCapabilitiesKHR surface_capabilities;
		VkResult res_get_physical_device_surface_capablilities = pfn_get_physical_device_surface_capabilities(vulkan_physical_device, vulkan_surface, &surface_capabilities);
		assert(VK_SUCCESS == res_get_physical_device_surface_capablilities);

		if (surface_capabilities.currentExtent.width != 0XFFFFFFFFU)
		{
			vulkan_framebuffer_width = surface_capabilities.currentExtent.width;
		}
		else
		{
			vulkan_framebuffer_width = g_resolution_width;
		}
		vulkan_framebuffer_width = (vulkan_framebuffer_width < surface_capabilities.minImageExtent.width) ? surface_capabilities.minImageExtent.width : (vulkan_framebuffer_width > surface_capabilities.maxImageExtent.width) ? surface_capabilities.maxImageExtent.width : vulkan_framebuffer_width;
		
		if (surface_capabilities.currentExtent.height != 0XFFFFFFFFU)
		{
			vulkan_framebuffer_height = surface_capabilities.currentExtent.height;
		}
		else
		{
			vulkan_framebuffer_height = g_resolution_height;
		}
		vulkan_framebuffer_height = (vulkan_framebuffer_height < surface_capabilities.minImageExtent.height) ? surface_capabilities.minImageExtent.height : (vulkan_framebuffer_height > surface_capabilities.maxImageExtent.height) ? surface_capabilities.maxImageExtent.height : vulkan_framebuffer_height;

		assert(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR == (VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR & surface_capabilities.supportedTransforms));
		assert(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR == (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR & surface_capabilities.supportedCompositeAlpha));
	}

	bool physical_device_feature_texture_compression_ASTC_LDR;
	bool physical_device_feature_texture_compression_BC;
	VkDevice vulkan_device = VK_NULL_HANDLE;
	{
		PFN_vkGetPhysicalDeviceFeatures pfn_vk_get_physical_device_features = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetPhysicalDeviceFeatures"));
		assert(NULL != pfn_vk_get_physical_device_features);
		PFN_vkCreateDevice pfn_vk_create_device = reinterpret_cast<PFN_vkCreateDevice>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkCreateDevice"));
		assert(NULL != pfn_vk_create_device);

		VkPhysicalDeviceFeatures physical_device_features;
		pfn_vk_get_physical_device_features(vulkan_physical_device, &physical_device_features);

		physical_device_feature_texture_compression_ASTC_LDR = (physical_device_features.textureCompressionASTC_LDR != VK_FALSE) ? true : false;
		physical_device_feature_texture_compression_BC = (physical_device_features.textureCompressionBC != VK_FALSE) ? true : false;

		struct VkDeviceCreateInfo device_create_info;
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pNext = NULL;
		device_create_info.flags = 0U;
		float queue_graphics_priority = 1.0f;
		float queue_transfer_priority = 1.0f;
		float queue_graphics_transfer_priorities[2] = {queue_graphics_priority, queue_transfer_priority};
		struct VkDeviceQueueCreateInfo device_queue_create_infos[2];
		if (vulkan_has_dedicated_transfer_queue)
		{
			if (vulkan_queue_graphics_family_index != vulkan_queue_transfer_family_index)
			{
				device_queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				device_queue_create_infos[0].pNext = NULL;
				device_queue_create_infos[0].flags = 0U;
				device_queue_create_infos[0].queueFamilyIndex = vulkan_queue_graphics_family_index;
				assert(0U == vulkan_queue_graphics_queue_index);
				device_queue_create_infos[0].queueCount = 1U;
				device_queue_create_infos[0].pQueuePriorities = &queue_graphics_priority;

				device_queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				device_queue_create_infos[1].pNext = NULL;
				device_queue_create_infos[1].flags = 0U;
				device_queue_create_infos[1].queueFamilyIndex = vulkan_queue_transfer_family_index;
				assert(0U == vulkan_queue_transfer_queue_index);
				device_queue_create_infos[1].queueCount = 1U;
				device_queue_create_infos[1].pQueuePriorities = &queue_transfer_priority;

				device_create_info.pQueueCreateInfos = device_queue_create_infos;
				device_create_info.queueCreateInfoCount = 2U;
			}
			else
			{
				device_queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				device_queue_create_infos[0].pNext = NULL;
				device_queue_create_infos[0].flags = 0U;
				device_queue_create_infos[0].queueFamilyIndex = vulkan_queue_graphics_family_index;
				assert(0U == vulkan_queue_graphics_queue_index);
				assert(1U == vulkan_queue_transfer_queue_index);
				device_queue_create_infos[0].queueCount = 2U;
				device_queue_create_infos[0].pQueuePriorities = queue_graphics_transfer_priorities;
				device_create_info.pQueueCreateInfos = device_queue_create_infos;
				device_create_info.queueCreateInfoCount = 1U;
			}
		}
		else
		{
			device_queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_infos[0].pNext = NULL;
			device_queue_create_infos[0].flags = 0U;
			device_queue_create_infos[0].queueFamilyIndex = vulkan_queue_graphics_family_index;
			assert(0U == vulkan_queue_graphics_queue_index);
			device_queue_create_infos[0].queueCount = 1U;
			device_queue_create_infos[0].pQueuePriorities = &queue_graphics_priority;
			device_create_info.pQueueCreateInfos = device_queue_create_infos;
			device_create_info.queueCreateInfoCount = 1U;
		}
		device_create_info.enabledLayerCount = 0U;
		device_create_info.ppEnabledLayerNames = NULL;
		char const *enabled_extension_names[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		device_create_info.enabledExtensionCount = 1U;
		device_create_info.ppEnabledExtensionNames = enabled_extension_names;
		device_create_info.pEnabledFeatures = &physical_device_features;

		VkResult res_create_device = pfn_vk_create_device(vulkan_physical_device, &device_create_info, vulkan_allocation_callbacks, &vulkan_device);
		assert(VK_SUCCESS == res_create_device);
	}

	PFN_vkGetDeviceProcAddr pfn_vk_get_device_proc_addr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(pfn_vk_get_instance_proc_addr(vulkan_instance, "vkGetDeviceProcAddr"));
	assert(NULL != pfn_vk_get_device_proc_addr);

	pfn_vk_get_device_proc_addr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(pfn_vk_get_device_proc_addr(vulkan_device, "vkGetDeviceProcAddr"));
	assert(NULL != pfn_vk_get_device_proc_addr);

	VkQueue vulkan_queue_graphcis = VK_NULL_HANDLE;
	VkQueue vulkan_queue_transfer = VK_NULL_HANDLE;
	{
		PFN_vkGetDeviceQueue vk_get_device_queue = reinterpret_cast<PFN_vkGetDeviceQueue>(pfn_vk_get_device_proc_addr(vulkan_device, "vkGetDeviceQueue"));
		assert(NULL != vk_get_device_queue);

		vk_get_device_queue(vulkan_device, vulkan_queue_graphics_family_index, vulkan_queue_graphics_queue_index, &vulkan_queue_graphcis);

		if (vulkan_has_dedicated_transfer_queue)
		{
			assert(VK_QUEUE_FAMILY_IGNORED != vulkan_queue_transfer_family_index);
			assert(uint32_t(-1) != vulkan_queue_transfer_queue_index);
			vk_get_device_queue(vulkan_device, vulkan_queue_transfer_family_index, vulkan_queue_transfer_queue_index, &vulkan_queue_transfer);
		}
	}
	assert(VK_NULL_HANDLE != vulkan_queue_graphcis && (!vulkan_has_dedicated_transfer_queue || VK_NULL_HANDLE != vulkan_queue_transfer));

	VkSwapchainKHR vulkan_swapchain = VK_NULL_HANDLE;
	{
		PFN_vkCreateSwapchainKHR pfn_create_swapchain = reinterpret_cast<PFN_vkCreateSwapchainKHR>(pfn_vk_get_device_proc_addr(vulkan_device, "vkCreateSwapchainKHR"));
		assert(NULL != pfn_create_swapchain);

		VkSwapchainCreateInfoKHR swapchain_create_info;
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.pNext = NULL;
		swapchain_create_info.flags = 0U;
		swapchain_create_info.surface = vulkan_surface;
		swapchain_create_info.minImageCount = FRAME_THROTTLING_COUNT;
		swapchain_create_info.imageFormat = vulkan_swapchain_image_format;
		swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_create_info.imageExtent.width = vulkan_framebuffer_width;
		swapchain_create_info.imageExtent.height = vulkan_framebuffer_height;
		swapchain_create_info.imageArrayLayers = 1U;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0U;
		swapchain_create_info.pQueueFamilyIndices = NULL;
		swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchain_create_info.clipped = VK_FALSE;
		swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

		VkResult res_create_swapchain = pfn_create_swapchain(vulkan_device, &swapchain_create_info, vulkan_allocation_callbacks, &vulkan_swapchain);
		assert(VK_SUCCESS == res_create_swapchain);
	}

	std::vector<VkImage> vulkan_swapchain_images;
	{
		PFN_vkGetSwapchainImagesKHR pfn_get_swapchain_images = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(pfn_vk_get_device_proc_addr(vulkan_device, "vkGetSwapchainImagesKHR"));
		assert(NULL != pfn_get_swapchain_images);
		
		uint32_t swapchain_image_count_1 = uint32_t(-1);
		VkResult res_get_swapchain_images_1 = pfn_get_swapchain_images(vulkan_device, vulkan_swapchain, &swapchain_image_count_1, NULL);
		assert(VK_SUCCESS == res_get_swapchain_images_1);

		VkImage *swapchain_images = static_cast<VkImage *>(malloc(sizeof(VkImage) * swapchain_image_count_1));
		assert(NULL != swapchain_images);

		uint32_t swapchain_image_count_2 = swapchain_image_count_1;
		VkResult res_get_swapchain_images_2 = pfn_get_swapchain_images(vulkan_device, vulkan_swapchain, &swapchain_image_count_2, swapchain_images);
		assert(VK_SUCCESS == res_get_swapchain_images_2 && swapchain_image_count_2 == swapchain_image_count_1);

		vulkan_swapchain_images.assign(swapchain_images, swapchain_images + swapchain_image_count_2);

		free(swapchain_images);
	}

	VkCommandPool vulkan_command_pools[FRAME_THROTTLING_COUNT];
	VkCommandBuffer vulkan_command_buffers[FRAME_THROTTLING_COUNT];
	VkFence vulkan_fences[FRAME_THROTTLING_COUNT];
	VkSemaphore vulkan_semaphores_acquire_next_image[FRAME_THROTTLING_COUNT];
	VkSemaphore vulkan_semaphores_queue_submit[FRAME_THROTTLING_COUNT];
	{
		PFN_vkCreateCommandPool pfn_create_command_pool = reinterpret_cast<PFN_vkCreateCommandPool>(pfn_vk_get_device_proc_addr(vulkan_device, "vkCreateCommandPool"));
		assert(NULL != pfn_create_command_pool);
		PFN_vkAllocateCommandBuffers pfn_allocate_command_buffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(pfn_vk_get_device_proc_addr(vulkan_device, "vkAllocateCommandBuffers"));
		assert(NULL != pfn_allocate_command_buffers);
		PFN_vkCreateFence pfn_create_fence = reinterpret_cast<PFN_vkCreateFence>(pfn_vk_get_device_proc_addr(vulkan_device, "vkCreateFence"));
		assert(NULL != pfn_create_fence);
		PFN_vkCreateSemaphore pfn_create_semaphore = reinterpret_cast<PFN_vkCreateSemaphore>(pfn_vk_get_device_proc_addr(vulkan_device, "vkCreateSemaphore"));
		assert(NULL != pfn_create_semaphore);

		for (uint32_t frame_throtting_index = 0U; frame_throtting_index < FRAME_THROTTLING_COUNT; ++frame_throtting_index)
		{
			VkCommandPoolCreateInfo command_pool_create_info;
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.pNext = NULL;
			command_pool_create_info.flags = 0U;
			command_pool_create_info.queueFamilyIndex = vulkan_queue_graphics_family_index;
			VkResult res_create_command_pool = pfn_create_command_pool(vulkan_device, &command_pool_create_info, vulkan_allocation_callbacks, &vulkan_command_pools[frame_throtting_index]);
			assert(VK_SUCCESS == res_create_command_pool);

			VkCommandBufferAllocateInfo command_buffer_allocate_info;
			command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.pNext = NULL;
			command_buffer_allocate_info.commandPool = vulkan_command_pools[frame_throtting_index];
			command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount = 1U;
			VkResult res_allocate_command_buffers = pfn_allocate_command_buffers(vulkan_device, &command_buffer_allocate_info, &vulkan_command_buffers[frame_throtting_index]);
			assert(VK_SUCCESS == res_allocate_command_buffers);

			VkFenceCreateInfo fence_create_info;
			fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_create_info.pNext = NULL;
			fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VkResult res_create_fence = pfn_create_fence(vulkan_device, &fence_create_info, vulkan_allocation_callbacks, &vulkan_fences[frame_throtting_index]);
			assert(VK_SUCCESS == res_create_fence);

			VkSemaphoreCreateInfo semaphore_create_info_acquire_next_image;
			semaphore_create_info_acquire_next_image.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info_acquire_next_image.pNext = NULL;
			semaphore_create_info_acquire_next_image.flags = 0U;
			VkResult res_create_semaphore_acquire_next_image = pfn_create_semaphore(vulkan_device, &semaphore_create_info_acquire_next_image, vulkan_allocation_callbacks, &vulkan_semaphores_acquire_next_image[frame_throtting_index]);
			assert(VK_SUCCESS == res_create_semaphore_acquire_next_image);

			VkSemaphoreCreateInfo semaphore_create_info_queue_submit;
			semaphore_create_info_queue_submit.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info_queue_submit.pNext = NULL;
			semaphore_create_info_queue_submit.flags = 0U;
			VkResult res_create_semaphore_queue_submit = pfn_create_semaphore(vulkan_device, &semaphore_create_info_queue_submit, vulkan_allocation_callbacks, &vulkan_semaphores_queue_submit[frame_throtting_index]);
			assert(VK_SUCCESS == res_create_semaphore_queue_submit);
		}
	}

	PFN_vkWaitForFences pfn_wait_for_fences = reinterpret_cast<PFN_vkWaitForFences>(pfn_vk_get_device_proc_addr(vulkan_device, "vkWaitForFences"));
	assert(NULL != pfn_wait_for_fences);
	PFN_vkResetFences pfn_reset_fences = reinterpret_cast<PFN_vkResetFences>(pfn_vk_get_device_proc_addr(vulkan_device, "vkResetFences"));
	assert(NULL != pfn_reset_fences);
	PFN_vkResetCommandPool pfn_reset_command_pool = reinterpret_cast<PFN_vkResetCommandPool>(pfn_vk_get_device_proc_addr(vulkan_device, "vkResetCommandPool"));
	assert(NULL != pfn_reset_command_pool);
	PFN_vkAcquireNextImageKHR pfn_acquire_next_image = reinterpret_cast<PFN_vkAcquireNextImageKHR>(pfn_vk_get_device_proc_addr(vulkan_device, "vkAcquireNextImageKHR"));
	assert(NULL != pfn_acquire_next_image);
	PFN_vkQueueSubmit pfn_queue_submit = reinterpret_cast<PFN_vkQueueSubmit>(pfn_vk_get_device_proc_addr(vulkan_device, "vkQueueSubmit"));
	assert(NULL != pfn_queue_submit);
	PFN_vkQueuePresentKHR pfn_queue_present = reinterpret_cast<PFN_vkQueuePresentKHR>(pfn_vk_get_device_proc_addr(vulkan_device, "vkQueuePresentKHR"));
	assert(NULL != pfn_queue_present);

	class Demo demo;
	demo.Init(vulkan_device, vulkan_swapchain_images.size(), &vulkan_swapchain_images[0], vulkan_swapchain_image_format);

	uint32_t frame_throtting_index = 0U;
	while (!g_window_quit)
	{
		VkCommandPool vulkan_command_pool = vulkan_command_pools[frame_throtting_index];
		VkCommandBuffer vulkan_command_buffer = vulkan_command_buffers[frame_throtting_index];
		VkFence vulkan_fence = vulkan_fences[frame_throtting_index];
		VkSemaphore vulkan_semaphore_acquire_next_image = vulkan_semaphores_acquire_next_image[frame_throtting_index];
		VkSemaphore vulkan_semaphore_queue_submit = vulkan_semaphores_queue_submit[frame_throtting_index];

		uint32_t vulkan_swapchain_image_index = uint32_t(-1); 
		VkResult res_acquire_next_image = pfn_acquire_next_image(vulkan_device, vulkan_swapchain, UINT64_MAX, vulkan_semaphore_acquire_next_image, VK_NULL_HANDLE, &vulkan_swapchain_image_index);
		assert(VK_SUCCESS == res_acquire_next_image);

		VkResult res_wait_for_fences = pfn_wait_for_fences(vulkan_device, 1U, &vulkan_fence, VK_TRUE, UINT64_MAX);
		assert(VK_SUCCESS == res_wait_for_fences);

		VkResult res_reset_command_pool = pfn_reset_command_pool(vulkan_device, vulkan_command_pool, 0U);
		assert(VK_SUCCESS == res_reset_command_pool);

		demo.Tick(vulkan_command_buffer, vulkan_swapchain_image_index);

		VkResult res_reset_fences = pfn_reset_fences(vulkan_device, 1U, &vulkan_fence);
		assert(VK_SUCCESS == res_reset_fences);

		VkSubmitInfo submit_info;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 1U;
		submit_info.pWaitSemaphores = &vulkan_semaphore_acquire_next_image;
		VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
		submit_info.commandBufferCount = 1U;
		submit_info.pCommandBuffers = &vulkan_command_buffer;
		submit_info.signalSemaphoreCount = 1U;
		submit_info.pSignalSemaphores = &vulkan_semaphore_queue_submit;
		VkResult res_queue_submit = pfn_queue_submit(vulkan_queue_graphcis, 1U, &submit_info, vulkan_fence);
		assert(VK_SUCCESS == res_queue_submit);

		VkPresentInfoKHR present_info;
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = NULL;
		present_info.waitSemaphoreCount = 1U;
		present_info.pWaitSemaphores = &vulkan_semaphore_queue_submit;
		present_info.swapchainCount = 1U;
		present_info.pSwapchains = &vulkan_swapchain;
		present_info.pImageIndices = &vulkan_swapchain_image_index;
		present_info.pResults = NULL;
		VkResult res_queue_present = pfn_queue_present(vulkan_queue_graphcis, &present_info);
		assert(VK_SUCCESS == res_queue_present);
	}

	return 0U;
}

static VkBool32 VKAPI_CALL vulkan_debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
	OutputDebugStringA(pCallbackData->pMessage);
	return VK_FALSE;
}