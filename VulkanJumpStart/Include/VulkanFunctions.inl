///
/// Exported Functions
///
#ifdef VK_EXPORTED_FUNCTION

VK_EXPORTED_FUNCTION( vkGetInstanceProcAddr )

#undef VK_EXPORTED_FUNCTION
#endif

///
/// Global Level Functions
///
#ifdef VK_GLOBAL_LEVEL_FUNCTION

VK_GLOBAL_LEVEL_FUNCTION( vkCreateInstance )
VK_GLOBAL_LEVEL_FUNCTION( vkEnumerateInstanceLayerProperties )

#undef VK_GLOBAL_LEVEL_FUNCTION
#endif

#ifdef VK_INSTANCE_LEVEL_FUNCTION

VK_INSTANCE_LEVEL_FUNCTION( vkGetDeviceProcAddr )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyInstance )
VK_INSTANCE_LEVEL_FUNCTION( vkCreateDebugReportCallbackEXT )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyDebugReportCallbackEXT )

#undef VK_INSTANCE_LEVEL_FUNCTION
#endif

#ifdef VK_DEVICE_LEVEL_FUNCTION

VK_DEVICE_LEVEL_FUNCTION( vkDestroyDevice )

#undef VK_DEVICE_LEVEL_FUNCTION
#endif