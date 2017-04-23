#ifndef VKFW_HEADER
#define VKFW_HEADER

#include "vulkan_functions.h"
#include "platform.h"
#include "types.h"

typedef struct VKFWwindow VKFWwindow;

void			vkfwInit();
LibHandle		vkfwGetVkLibHandle();

const char**	vkfwGetRequiredInstanceExtensions(uint32_t*);
const char**	vkfwGetRequiredInstanceLayers(uint32_t*);

VKFWwindow*		vkfwCreateWindow(VkfwUint32 with, VkfwUint32 height, VkfwString title);
VkResult		vkfwCreateWindowSurface(VkInstance instance, const VKFWwindow* window, VkAllocationCallbacks* allocationCallbacks, VkSurfaceKHR* surface);
void			vkfwDestroyWindow(VKFWwindow*);

void			vkfwCreateDevice(const VkInstance*, VkDevice*);

void			vkfwLoadInstanceLevelEntryPoints(const VkInstance*);
void			vkfwLoadDeviceLevelEntryPoints(const VkDevice*);

#endif // !VKFW_HEADER