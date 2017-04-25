#include "vulkan/vulkan.h"
#include "internal.h"
#include "vulkan_functions.h"
#include "platform.h"

#include <iostream>

void _vkfwLoadExportedEntryPoints()
{
#define VK_EXPORTED_FUNCTION( FUNC )															\
	if ((FUNC = (PFN_##FUNC)LoadProcAddress( _vkfw.vk.libHandle, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load exported function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}

void _vkfwLoadGlobalLevelEntryPoints()
{
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( nullptr, #FUNC )) == VK_NULL_HANDLE)			\
		std::cout << "Failed to load global function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}

void _vkfwLoadRequiredInstanceLayers()
{
#ifdef _VKFW_ENABLE_VALIDATION_LAYERS
	_vkfw.vk.requiredInstanceLayers.push_back(_VKFW_STANDARD_VALIDATION_LAYER);
#endif
}

void _vkfwLoadRequiredInstanceExtensions()
{
	_vkfw.vk.requiredInstanceExtensions.push_back("VK_KHR_surface");
	_vkfw.vk.requiredInstanceExtensions.push_back(_VKFW_PLATFORM_SURFACE_EXTENSION);
#if _VKFW_ENABLE_VALIDATION_LAYERS
	_vkfw.vk.requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
}

VKFWbool _vkfwCheckRequiredLayersAvailability()
{
	VKFWbool available = VKFW_TRUE;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> properties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, properties.data());

	for (const char* validationLayerName : _vkfw.vk.requiredInstanceLayers)
	{
		VKFWbool found = VKFW_FALSE;

		for (const VkLayerProperties &prop : properties)
		{
			found = found || !strcmp(prop.layerName, validationLayerName);
			if (found)
				break;
		}

		available = available && found;
		if (!available)
			break;
	}

	return available;
}