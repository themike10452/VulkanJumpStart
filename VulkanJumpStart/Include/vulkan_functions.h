#ifndef VULKAN_FUNCTIONS_HEADER
#define VULKAN_FUNCTIONS_HEADER

#include "vulkan.h"

#define VK_EXPORTED_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_DEVICE_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;

#include "vulkan_functions.inl"

#endif // !VULKAN_FUNCTIONS_HEADER