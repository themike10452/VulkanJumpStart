#pragma once

#include <vulkan/vulkan.h>

#define VK_EXPORTED_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_GLOBAL_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_INSTANCE_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;
#define VK_DEVICE_LEVEL_FUNCTION( FUNC ) extern PFN_##FUNC FUNC;

#include "VulkanFunctions.inl"