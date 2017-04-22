#ifndef VKFW_HEADER
#define VKFW_HEADER

#include <vector>

#include "VulkanFunctions.h"
#include "VkPtr.h"
#include "OS.h"
#include "Window.h"

#ifdef _DEBUG
	#define VKFW_ENABLE_VALIDATION_LAYERS
#endif

struct VulkanContext
{
	LibraryHandle LibHandle;
	WindowHandle WndHandle;

	std::vector<char*> extensions;
	std::vector<char*> validationLayers;

	VkPtr<VkInstance> instance{ &vkDestroyInstance };
	VkPtr<VkDevice> device{ &vkDestroyDevice };
	VkPtr<VkSurfaceKHR> surface{ instance, &vkDestroySurfaceKHR };
	
	VkQueue graphicsQueue = VK_NULL_HANDLE;

#ifdef VKFW_ENABLE_VALIDATION_LAYERS
	bool enableValidationLayers = 1;

	VkPtr<VkDebugReportCallbackEXT> debugCallback{ instance, &vkDestroyDebugReportCallbackEXT };
#else
	bool enableValidationLayers = 0;
#endif

};

extern VulkanContext Vulkan;

#ifdef VKFW_ENABLE_VALIDATION_LAYERS

VKAPI_ATTR VkBool32 VKAPI_CALL vkfwDebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData);

#endif

void vkfwInit();

const char** vkfwGetRequiredInstanceExtensions(uint32_t*);
const char** vkfwGetRequiredInstanceLayers(uint32_t*);

void vkfwCreateDevice(const VkInstance*, VkDevice*);
void vkfwCreateWindowSurface();

void LoadExportedEntryPoints();
void LoadGlobalLevelEntryPoints();
void LoadInstanceLevelEntryPoints();
void LoadDeviceLevelEntryPoints();

#endif // !VKFW_HEADER