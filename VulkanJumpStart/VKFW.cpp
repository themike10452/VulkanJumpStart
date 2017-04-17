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
	Vulkan.LibHandle = LoadLibrary(L"vulkan-1.dll");

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

const VkPhysicalDevice vkfwGetPhysicalDevice(const VkInstance* instance, std::function<bool(VkPhysicalDeviceProperties&, VkPhysicalDeviceFeatures&)> predicate)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(deviceCount);

	vkEnumeratePhysicalDevices(Vulkan.instance, &deviceCount, physicalDevices.data());

	for each (const VkPhysicalDevice& device in physicalDevices)
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(device, &properties);

		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceFeatures(device, &features);

		if (predicate(properties, features))
		{
			return device;
		}
	}

	throw std::runtime_error("Failed to find a suitable GPU");
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