#pragma once

#include <vector>

#include "VulkanFunctions.h"
#include "VDeleter.h"
#include "OS.h"

#ifdef _DEBUG
	#define _VKFW_ENABLE_VALIDATION_LAYERS
#endif

#ifdef _VKFW_ENABLE_VALIDATION_LAYERS

static VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugCallback(
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

struct
{
#ifdef _VKFW_ENABLE_VALIDATION_LAYERS
	bool enableValidationLayers = true;
#else
	bool enableValidationLayers = false;
#endif

	std::vector<char*> extensions;
	std::vector<char*> validationLayers;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDevice> device{ vkDestroyDevice };

	OS::LibraryHandle LibHandle;

} Vulkan;

#pragma region

void vkfwInit();

const char** vkfwGetRequiredInstanceExtensions(uint32_t*);
const char** vkfwGetRequiredInstanceLayers(uint32_t*);

void _loadExportedEntryPoints();
void _loadGlobalLevelEntryPoints();
void _loadInstanceLevelEntryPoints();
void _loadDeviceLevelEntryPoints();

void _loadRequiredInstanceExtensions();
void _loadRequiredInstanceLayers();
bool _checkValidationLayersAvailable();

#pragma endregion forward declarations

#pragma region

inline void vkfwInit()
{
	Vulkan.LibHandle = LoadLibrary(L"vulkan-1.dll");

	_loadExportedEntryPoints();
	_loadGlobalLevelEntryPoints();

	_loadRequiredInstanceExtensions();
	_loadRequiredInstanceLayers();

#ifdef _VKFW_ENABLE_VALIDATION_LAYERS
	if (!_checkValidationLayersAvailable())
		throw new std::runtime_error("Requested validation layers are not available");
#endif
}

inline const char** vkfwGetRequiredInstanceExtensions(uint32_t* extensionCount)
{
	assert(extensionCount != nullptr);
	*extensionCount = (uint32_t)Vulkan.extensions.size();
	return (const char**)Vulkan.extensions.data();
}

inline const char** vkfwGetRequiredInstanceLayers(uint32_t* layerCount)
{
	assert(layerCount != nullptr);
	*layerCount = (uint32_t)Vulkan.validationLayers.size();
	return (const char**)Vulkan.validationLayers.data();
}

inline void _loadExportedEntryPoints()
{
#define VK_EXPORTED_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)LoadProcAddress( Vulkan.LibHandle, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load exported function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

inline void _loadGlobalLevelEntryPoints()
{
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC )													\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( nullptr, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load global function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

inline void _loadInstanceLevelEntryPoints()
{
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( Vulkan.instance, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load instance function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

inline void _loadDeviceLevelEntryPoints()
{
#define VK_DEVICE_LEVEL_FUNCTION( FUNC )														\
	if ((FUNC = (PFN_##FUNC)vkGetDeviceProcAddr( Vulkan.device, #FUNC )) == VK_NULL_HANDLE)		\
		std::cout << "Failed to load device function: " << #FUNC << std::endl;

#include "VulkanFunctions.inl"
}

inline bool _checkValidationLayersAvailable()
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

inline void _loadRequiredInstanceLayers()
{
#ifdef _VKFW_ENABLE_VALIDATION_LAYERS
	// enable standard validation layer
	Vulkan.validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
}

inline void _loadRequiredInstanceExtensions()
{
	Vulkan.extensions.push_back("VK_KHR_surface");
#ifdef _VKFW_ENABLE_VALIDATION_LAYERS
	Vulkan.extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
#if WIN32
	Vulkan.extensions.push_back("VK_KHR_win32_surface");
#endif
}

#pragma endregion implementations