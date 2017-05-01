#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "internal.h"

#include <iostream>

VkQueue deviceGraphicsQueue = VK_NULL_HANDLE;

_VkfwLibrary _vkfw;

void vkfwInit()
{
    _vkfw.vk.libHandle = LoadDynamicLibrary( "vulkan-1.dll" );

    _vkfwLoadExportedEntryPoints();
    _vkfwLoadGlobalLevelEntryPoints();

    _vkfwLoadRequiredInstanceExtensions();
    _vkfwLoadRequiredInstanceLayers();
    _vkfwLoadRequiredDeviceExtensions();

    if (!_vkfwCheckRequiredLayersAvailability())
        throw new std::runtime_error( "Not all required layers are available" );

    _vkfw.initialized = VKFW_TRUE;
}

void vkfwDestroy()
{
    if (_vkfw.initialized)
    {
        FreeLibrary( _vkfw.vk.libHandle );
    }
}

LibHandle vkfwGetVkLibHandle()
{
    _VKFW_REQUIRE_INIT();

    return _vkfw.vk.libHandle;
}

const VkfwString* vkfwGetRequiredInstanceExtensions( VkfwUint32* pCount )
{
    _VKFW_REQUIRE_INIT();
    _VKFW_REQUIRE_PTR(pCount);

    *pCount = (VkfwUint32)_vkfw.vk.requiredInstanceExtensions.size();
    return (const VkfwString*)_vkfw.vk.requiredInstanceExtensions.data();
}

const VkfwString* vkfwGetRequiredInstanceLayers( VkfwUint32* pCount )
{
    _VKFW_REQUIRE_INIT();
    _VKFW_REQUIRE_PTR(pCount);

    *pCount = (VkfwUint32)_vkfw.vk.requiredInstanceLayers.size();
    return const_cast<const VkfwString*>(_vkfw.vk.requiredInstanceLayers.data());
}

const VkfwString* vkfwGetRequiredDeviceExtensions( VkfwUint32* pCount )
{
    _VKFW_REQUIRE_INIT();
    _VKFW_REQUIRE_PTR(pCount);

    *pCount = (VkfwUint32)_vkfw.vk.requiredDeviceExtensions.size();
    return const_cast<const VkfwString*>(_vkfw.vk.requiredDeviceExtensions.data());
}

void vkfwEnumeratePhysicalDevices( const VkInstance* pInstance, std::vector<VkPhysicalDevice>* pData )
{
    _VKFW_REQUIRE_PTR(pData);

    pData->clear();

    VkfwUint32 count;
    vkEnumeratePhysicalDevices( *pInstance, &count, nullptr );

    if (count > 0)
    {
        pData->resize( count );
        vkEnumeratePhysicalDevices( *pInstance, &count, pData->data() );
    }
}

VkfwBool vkfwCheckDeviceExtensionSupport( const VkPhysicalDevice* pPhysicalDevice )
{
    VkfwUint32 extensionCount;
    vkEnumerateDeviceExtensionProperties( *pPhysicalDevice, nullptr, &extensionCount, nullptr );
    std::vector<VkExtensionProperties> availableExtensions( extensionCount );
    vkEnumerateDeviceExtensionProperties( *pPhysicalDevice, nullptr, &extensionCount, availableExtensions.data() );

    VkfwUint32 requiredExtCount;
    const VkfwString* requiredExtensions = vkfwGetRequiredDeviceExtensions( &requiredExtCount );

    VkfwBool found = VKFW_TRUE;
    for (VkfwUint32 i = 0; i < requiredExtCount && found; i++)
    {
        found = VKFW_FALSE;
        for (const auto& extension : availableExtensions)
        {
            if (strcmp( extension.extensionName, requiredExtensions[i] ) == 0)
            {
                found = VKFW_TRUE;
                break;
            }
        }
    }

    return found;
}

VkfwSwapchainSupport vkfwQuerySwapchainSupport( const VkPhysicalDevice* pPhysicalDevice, const VkSurfaceKHR* pSurface )
{
    _VKFW_REQUIRE_PTR(pPhysicalDevice);
    _VKFW_REQUIRE_PTR(pSurface);

    VkfwSwapchainSupport swapchainSupport = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( *pPhysicalDevice, *pSurface, &swapchainSupport.capabilities );

    VkfwUint32 count;
    vkGetPhysicalDeviceSurfaceFormatsKHR( *pPhysicalDevice, *pSurface, &count, nullptr );
    swapchainSupport.formats.resize( count );
    vkGetPhysicalDeviceSurfaceFormatsKHR( *pPhysicalDevice, *pSurface, &count, swapchainSupport.formats.data() );

    vkGetPhysicalDeviceSurfacePresentModesKHR( *pPhysicalDevice, *pSurface, &count, nullptr );
    swapchainSupport.presentModes.resize( count );
    vkGetPhysicalDeviceSurfacePresentModesKHR( *pPhysicalDevice, *pSurface, &count, swapchainSupport.presentModes.data() );

    return swapchainSupport;
}

VkResult vkfwCreateShaderModule( const VkDevice* pDevice, const VkfwString pBytecode, VkfwSize byteCodeLength, const VkAllocationCallbacks* pCallbacks, VkShaderModule* pShaderModule )
{
    _VKFW_REQUIRE_PTR( pDevice );
    _VKFW_REQUIRE_PTR( pBytecode );
    _VKFW_REQUIRE_PTR( pShaderModule );

    std::vector<uint32_t> code( (byteCodeLength / sizeof(uint32_t)) + 1 );
    memcpy(code.data(), pBytecode, byteCodeLength);

    VkShaderModuleCreateInfo 
    createInfo          = {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext    = VK_NULL_HANDLE;
    createInfo.pCode    = code.data();
    createInfo.codeSize = byteCodeLength;
    createInfo.flags    = 0;

    return vkCreateShaderModule( *pDevice, &createInfo, pCallbacks, pShaderModule );
}

VkfwWindow* vkfwCreateWindow( VkfwUint32 width, VkfwUint32 height, VkfwString title )
{
    _VkfwWindow* pWindow = new _VkfwWindow();

    pWindow->config.width = width;
    pWindow->config.height = height;
    pWindow->config.visible = VKFW_TRUE;
    pWindow->config.title = new VkfwChar[strlen( title ) + 1];

    strcpy( pWindow->config.title, title );

    _vkfwPlatformCreateWindow( pWindow );

    if ( pWindow->handle )
        _vkfw.windowMap.insert( { pWindow->handle, pWindow } );

    return (VkfwWindow*)pWindow;
}

VkResult vkfwCreateWindowSurface( const VkInstance* pInstance, const VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface )
{
    return _vkfwPlatformCreateSurfaceKHR( *pInstance, (const _VkfwWindow*)pWindow, pCallbacks, pSurface );
}

void vkfwDestroyWindow( VkfwWindow* pWindow )
{
    _VKFW_REQUIRE_PTR_OR_LEAVE( pWindow );

    _VkfwWindow* window = (_VkfwWindow*)pWindow;

    _vkfw.windowMap.erase( window->handle );

    _vkfwPlatformDestroyWindow( window );
}

void vkfwGetWindowResolution(const VkfwWindow* pWindow, VkfwUint32 * width, VkfwUint32 * height)
{
    _VKFW_REQUIRE_PTR( pWindow );

    _VkfwWindow* window = (_VkfwWindow*)pWindow;
    
    if (width)
        *width = window->config.width;

    if (height)
        *height = window->config.height;
}

VkfwBool vkfwWindowShouldClose( const VkfwWindow* pWindow )
{
    return ((const _VkfwWindow*)pWindow)->state.shouldClose;
}

void vkfwPollEvents()
{
    _vkfwPlatformPollEvents();
}

void vkfwLoadInstanceLevelEntryPoints( const VkInstance* pInstance )
{
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC )													\
	if ((FUNC = (PFN_##FUNC)vkGetInstanceProcAddr( *pInstance, #FUNC )) == VK_NULL_HANDLE)	\
		std::cout << "Failed to load instance function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}

void vkfwLoadDeviceLevelEntryPoints( const VkDevice* pDevice )
{
#define VK_DEVICE_LEVEL_FUNCTION( FUNC )												    \
	if ((FUNC = (PFN_##FUNC)vkGetDeviceProcAddr( *pDevice, #FUNC )) == VK_NULL_HANDLE)	    \
		std::cout << "Failed to load device function: " << #FUNC << std::endl;

#include "vulkan_functions.inl"
}
