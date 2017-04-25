#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "internal.h"

#include <assert.h>
#include <iostream>

VkQueue deviceGraphicsQueue = VK_NULL_HANDLE;

_VKFWlibrary _vkfw;

void vkfwInit()
{
	_vkfw.vk.libHandle = LoadDynamicLibrary("vulkan-1.dll");

	_vkfwLoadExportedEntryPoints();
	_vkfwLoadGlobalLevelEntryPoints();

	_vkfwLoadRequiredInstanceExtensions();
	_vkfwLoadRequiredInstanceLayers();

	if (!_vkfwCheckRequiredLayersAvailability())
		throw new std::runtime_error("Not all required layers are available");

	_vkfw.initialized = VKFW_TRUE;
}

LibHandle vkfwGetVkLibHandle()
{
	return _vkfw.vk.libHandle;
}

const VKFWstring* vkfwGetRequiredInstanceExtensions(VKFWuint32* pCount)
{
	assert(pCount != nullptr);
	*pCount = (VKFWuint32)_vkfw.vk.requiredInstanceExtensions.size();
	return (const VKFWstring*)_vkfw.vk.requiredInstanceExtensions.data();
}

const VKFWstring* vkfwGetRequiredInstanceLayers(VKFWuint32* pCount)
{
	assert(pCount != nullptr);
	*pCount = (VKFWuint32)_vkfw.vk.requiredInstanceLayers.size();
	return (const VKFWstring*)_vkfw.vk.requiredInstanceLayers.data();
}

void vkfwEnumeratePhysicalDevices(const VkInstance* pInstance, std::vector<VkPhysicalDevice>* pData)
{
	assert(pData != nullptr);

	pData->clear();

	VKFWuint32 count;
	vkEnumeratePhysicalDevices(*pInstance, &count, nullptr);

	if (count > 0)
	{
		pData->resize(count);
		vkEnumeratePhysicalDevices(*pInstance, &count, pData->data());
	}
}

VKFWwindow* vkfwCreateWindow(VKFWuint32 width, VKFWuint32 height, VKFWstring title)
{
	_VKFWwindow* pWindow = (_VKFWwindow*)calloc(1, sizeof(_VKFWwindow));

	pWindow->windowConfig.width		= width;
	pWindow->windowConfig.height	= height;
	pWindow->windowConfig.visible	= VKFW_TRUE;
	pWindow->windowConfig.title		= (VKFWstring)malloc(sizeof(char) * ( strlen(title) + 1 ));
	
	strcpy(pWindow->windowConfig.title, title);

	_vkfwPlatformCreateWindow(pWindow);

	return (VKFWwindow*)pWindow;
}

VkResult vkfwCreateWindowSurface(const VkInstance* pInstance, const VKFWwindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface)
{
	return _vkfwPlatformCreateSurfaceKHR(*pInstance, (const _VKFWwindow*)pWindow, pCallbacks, pSurface);
}

void vkfwDestroyWindow(VKFWwindow* pWindow)
{
	_vkfwPlatformDestroyWindow((_VKFWwindow*)pWindow);
}

void vkfwLoadInstanceLevelEntryPoints(const VkInstance* pInstance)
{
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( *pInstance, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load instance function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}

void vkfwLoadDeviceLevelEntryPoints(const VkDevice* pDevice)
{
#define VK_DEVICE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetDeviceProcAddr( *pDevice, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load device function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}