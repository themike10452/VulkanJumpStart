#ifndef VKFW_HEADER
#define VKFW_HEADER

#include "vulkan_functions.h"
#include "platform.h"
#include "types.h"

#include <vector>

typedef struct VKFWwindow VKFWwindow;
typedef struct QueueFamilyIndices QueueFamilyIndices;

void					vkfwInit();
LibHandle				vkfwGetVkLibHandle();

const VKFWstring*		vkfwGetRequiredInstanceExtensions(VKFWuint32* pCount);
const VKFWstring*		vkfwGetRequiredInstanceLayers(VKFWuint32* pCount);
void					vkfwEnumeratePhysicalDevices(const VkInstance* pInstance, std::vector<VkPhysicalDevice>* pData);

VKFWwindow*				vkfwCreateWindow(VKFWuint32 with, VKFWuint32 height, VKFWstring title);
VkResult				vkfwCreateWindowSurface(const VkInstance* pInstance, const VKFWwindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface);
void					vkfwDestroyWindow(VKFWwindow* pWindow);

void					vkfwLoadInstanceLevelEntryPoints(const VkInstance* pInstance);
void					vkfwLoadDeviceLevelEntryPoints(const VkDevice* pDevice);

#endif // !VKFW_HEADER