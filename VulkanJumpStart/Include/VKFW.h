#ifndef VKFW_HEADER
#define VKFW_HEADER

#include "vulkan_functions.h"
#include "platform.h"
#include "types.h"

#include <vector>

typedef struct VkfwWindow			VkfwWindow;
typedef struct VkfwSwapchainSupport	VkfwSwapchainSupport;

void					vkfwInit();
void					vkfwDestroy();
LibHandle				vkfwGetVkLibHandle();

const VkfwString*		vkfwGetRequiredInstanceExtensions( VkfwUint32* pCount );
const VkfwString*		vkfwGetRequiredInstanceLayers( VkfwUint32* pCount );
const VkfwString*		vkfwGetRequiredDeviceExtensions( VkfwUint32* pCount );
void					vkfwEnumeratePhysicalDevices( const VkInstance* pInstance, std::vector<VkPhysicalDevice>* pData );
VkfwBool				vkfwCheckDeviceExtensionSupport( const VkPhysicalDevice* pPhysicalDevice );
VkfwSwapchainSupport	vkfwQuerySwapchainSupport( const VkPhysicalDevice* pPhysicalDevice, const VkSurfaceKHR* pSurface );
VkResult                vkfwCreateShaderModule( const VkDevice* pDevice, const VkfwString pBytecode, VkfwSize byteCodeLength, const VkAllocationCallbacks* pCallbacks, VkShaderModule* pShaderModule );

VkfwWindow*				vkfwCreateWindow( VkfwUint32 with, VkfwUint32 height, VkfwString title );
VkResult				vkfwCreateWindowSurface( const VkInstance* pInstance, const VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface );
void					vkfwDestroyWindow( VkfwWindow* pWindow );
void                    vkfwGetWindowResolution( const VkfwWindow* pWindow, VkfwUint32* width, VkfwUint32* height );

void					vkfwLoadInstanceLevelEntryPoints( const VkInstance* pInstance );
void					vkfwLoadDeviceLevelEntryPoints( const VkDevice* pDevice );

struct VkfwSwapchainSupport
{
	VkSurfaceCapabilitiesKHR		capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>	presentModes;
};

#endif // !VKFW_HEADER