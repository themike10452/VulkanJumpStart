#include "VKFW.h"

#include <assert.h>

VulkanContext Vulkan;

#ifdef VKFW_ENABLE_VALIDATION_LAYERS

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

void vkfwInit()
{
	Vulkan.LibHandle = LoadLibrary("vulkan-1.dll");

	_loadExportedEntryPoints();
	_loadGlobalLevelEntryPoints();

	_loadRequiredInstanceExtensions();
	_loadRequiredInstanceLayers();

#ifdef VKFW_ENABLE_VALIDATION_LAYERS
	if (!_checkValidationLayersAvailable())
		throw new std::runtime_error("Requested validation layers are not available");
#endif
}

const char** vkfwGetRequiredInstanceExtensions(uint32_t* extensionCount)
{
	assert(extensionCount != nullptr);
	*extensionCount = (uint32_t)Vulkan.extensions.size();
	return (const char**)Vulkan.extensions.data();
}

const char** vkfwGetRequiredInstanceLayers(uint32_t* layerCount)
{
	assert(layerCount != nullptr);
	*layerCount = (uint32_t)Vulkan.validationLayers.size();
	return (const char**)Vulkan.validationLayers.data();
}

VkResult vkfwCreateDevice(const VkInstance* instance, VkDevice* outDevice)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	if (deviceCount < 1)
		throw std::runtime_error("Could not find a compatible GPU");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(Vulkan.instance, &deviceCount, physicalDevices.data());

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

#ifdef VKFW_ENABLE_VALIDATION_LAYERS
	deviceCreateInfo.enabledLayerCount = Vulkan.validationLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = Vulkan.validationLayers.data();
#endif // VKFW_ENABLE_VALIDATION_LAYERS

	VkResult result = vkCreateDevice(physicalDevices[bestPhysicalDeviceIndex], &deviceCreateInfo, nullptr, outDevice);
	if (result == VK_SUCCESS)
		_loadDeviceLevelEntryPoints();
	
	return result;
}

void _loadExportedEntryPoints()
{
#define VK_EXPORTED_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)LoadProcAddress( Vulkan.LibHandle, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load exported function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

void _loadGlobalLevelEntryPoints()
{
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC )													\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( nullptr, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load global function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

void _loadInstanceLevelEntryPoints()
{
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( Vulkan.instance, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load instance function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

void _loadDeviceLevelEntryPoints()
{
#define VK_DEVICE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetDeviceProcAddr( Vulkan.device, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load device function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

bool _checkValidationLayersAvailable()
{
	bool available = true;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> properties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, properties.data());

	for (const char* validationLayerName : Vulkan.validationLayers)
	{
		bool found = false;

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

void _loadRequiredInstanceLayers()
{
#ifdef VKFW_ENABLE_VALIDATION_LAYERS
	// enable standard validation layer
	Vulkan.validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
}

void _loadRequiredInstanceExtensions()
{
	Vulkan.extensions.push_back("VK_KHR_surface");
#ifdef VKFW_ENABLE_VALIDATION_LAYERS
	Vulkan.extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
#ifdef WIN32
	Vulkan.extensions.push_back("VK_KHR_win32_surface");
#endif
}