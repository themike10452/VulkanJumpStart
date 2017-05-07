#include "vulkan/vulkan.h"
#include "vulkan_functions.h"

#define VK_EXPORTED_FUNCTION( FUNC ) PFN_##FUNC FUNC;
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC ) PFN_##FUNC FUNC;
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC ) PFN_##FUNC FUNC;
#define VK_DEVICE_LEVEL_FUNCTION( FUNC ) PFN_##FUNC FUNC;

#include "vulkan_functions.inl"