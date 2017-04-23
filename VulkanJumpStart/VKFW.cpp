#include "vkfw.h"
#include "internal.h"
#include "platform.h"

#include <vector>
#include <assert.h>
#include <iostream>

#ifdef _VKFW_ENABLE_VALIDATION_LAYERS

VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

#endif

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

const char** vkfwGetRequiredInstanceExtensions(uint32_t* extensionCount)
{
	assert(extensionCount != nullptr);
	*extensionCount = (uint32_t)_vkfw.vk.requiredInstanceExtensions.size();
	return (const char**)_vkfw.vk.requiredInstanceExtensions.data();
}

const char** vkfwGetRequiredInstanceLayers(uint32_t* layerCount)
{
	assert(layerCount != nullptr);
	*layerCount = (uint32_t)_vkfw.vk.requiredInstanceLayers.size();
	return (const char**)_vkfw.vk.requiredInstanceLayers.data();
}

VKFWwindow* vkfwCreateWindow(VkfwUint32 width, VkfwUint32 height, VkfwString title)
{
	_VKFWwindow* pWindow = (_VKFWwindow*)calloc(1, sizeof(_VKFWwindow));

	pWindow->windowConfig.width		= width;
	pWindow->windowConfig.height	= height;
	pWindow->windowConfig.title		= (VkfwString)malloc(sizeof(char) * ( strlen(title) + 1 ));
	
	strcpy(pWindow->windowConfig.title, title);

	_vkfwPlatformCreateWindow(pWindow);

	return (VKFWwindow*)pWindow;
}

VkResult vkfwCreateWindowSurface(VkInstance instance, const VKFWwindow* pWindow, VkAllocationCallbacks* allocationCallbacks, VkSurfaceKHR* surface)
{
	return _vkfwPlatformCreateSurfaceKHR(instance, (const _VKFWwindow*)pWindow, allocationCallbacks, surface);
}

void vkfwDestroyWindow(VKFWwindow* pWindow)
{
	_vkfwPlatformDestroyWindow((_VKFWwindow*)pWindow);
}

void vkfwCreateDevice(const VkInstance* instance, VkDevice* outDevice)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	if (deviceCount < 1)
		throw std::runtime_error("Could not find a compatible GPU");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, physicalDevices.data());

	uint32_t
		bestScore = 0, 
		bestPhysicalDeviceIndex = 0, 
		bestQueueFamilyIndex = 0;

	for (uint32_t i = 0; i < deviceCount; i++)
	{
		VkPhysicalDeviceProperties deviceProperties = {};
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures = {};
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

		uint32_t score = 0, queueFamilyIndex = 0;

		score += deviceProperties.limits.maxImageDimension2D;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 10000;

		if (!deviceFeatures.geometryShader)
			score = 0;

		if (score > bestScore)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilies.data());

			int32_t queueFamilyIndex = -1;

			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{
				if (queueFamilies[i].queueCount > 0 &&
					queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queueFamilyIndex = i;
					break;
				}
			}

			if (queueFamilyIndex > -1)
			{
				bestScore = score;
				bestPhysicalDeviceIndex = i;
				bestQueueFamilyIndex = queueFamilyIndex;
			}
		}
	}

	if (bestScore == 0)
		throw std::runtime_error("Could not find a suitable GPU");

	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = nullptr;
	queueCreateInfo.queueFamilyIndex = bestQueueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.enabledLayerCount = 0;

	VkResult result = vkCreateDevice(physicalDevices[bestPhysicalDeviceIndex], &deviceCreateInfo, nullptr, outDevice);
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkCreateDevice failed");

	vkGetDeviceQueue(*outDevice, bestQueueFamilyIndex, 0, &deviceGraphicsQueue);

	if (!deviceGraphicsQueue)
		throw std::runtime_error("vkGetDeviceQueue failed");
}

void vkfwLoadInstanceLevelEntryPoints(const VkInstance* instance)
{
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( *instance, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load instance function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}

void vkfwLoadDeviceLevelEntryPoints(const VkDevice* device)
{
#define VK_DEVICE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetDeviceProcAddr( *device, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load device function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}