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

#ifdef VKFW_ENABLE_VALIDATION
VK_INSTANCE_LEVEL_FUNCTION( vkCreateDebugReportCallbackEXT )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyDebugReportCallbackEXT )
#endif // VKFW_ENABLE_VALIDATION

VK_INSTANCE_LEVEL_FUNCTION( vkGetDeviceProcAddr )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyInstance )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumeratePhysicalDevices )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumerateDeviceExtensionProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceFeatures )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceQueueFamilyProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceSupportKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceFormatsKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceCapabilitiesKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfacePresentModesKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceMemoryProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkCreateDevice )
#ifdef VK_USE_PLATFORM_WIN32_KHR
VK_INSTANCE_LEVEL_FUNCTION( vkCreateWin32SurfaceKHR )
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
VK_INSTANCE_LEVEL_FUNCTION( vkCreateXlibSurfaceKHR )
#endif
VK_INSTANCE_LEVEL_FUNCTION( vkDestroySurfaceKHR )

#undef VK_INSTANCE_LEVEL_FUNCTION
#endif

#ifdef VK_DEVICE_LEVEL_FUNCTION

VK_DEVICE_LEVEL_FUNCTION( vkCreateSemaphore )
VK_DEVICE_LEVEL_FUNCTION( vkDestroySemaphore )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDevice )
VK_DEVICE_LEVEL_FUNCTION( vkDeviceWaitIdle )
VK_DEVICE_LEVEL_FUNCTION( vkGetDeviceQueue )
VK_DEVICE_LEVEL_FUNCTION( vkQueueWaitIdle )
VK_DEVICE_LEVEL_FUNCTION( vkCreateSwapchainKHR )
VK_DEVICE_LEVEL_FUNCTION( vkDestroySwapchainKHR )
VK_DEVICE_LEVEL_FUNCTION( vkGetSwapchainImagesKHR )
VK_DEVICE_LEVEL_FUNCTION( vkAcquireNextImageKHR )
VK_DEVICE_LEVEL_FUNCTION( vkCreateImageView )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyImageView )
VK_DEVICE_LEVEL_FUNCTION( vkCreateShaderModule )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyShaderModule )
VK_DEVICE_LEVEL_FUNCTION( vkCreateRenderPass )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyRenderPass )
VK_DEVICE_LEVEL_FUNCTION( vkCreatePipelineLayout )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyPipelineLayout )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyPipeline )
VK_DEVICE_LEVEL_FUNCTION( vkCreateGraphicsPipelines )
VK_DEVICE_LEVEL_FUNCTION( vkCreateFramebuffer )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyFramebuffer )
VK_DEVICE_LEVEL_FUNCTION( vkCreateBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkCreateCommandPool )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyCommandPool )
VK_DEVICE_LEVEL_FUNCTION( vkAllocateCommandBuffers )
VK_DEVICE_LEVEL_FUNCTION( vkFreeCommandBuffers )
VK_DEVICE_LEVEL_FUNCTION( vkBeginCommandBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkEndCommandBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkCmdCopyBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkCmdBeginRenderPass )
VK_DEVICE_LEVEL_FUNCTION( vkCmdEndRenderPass )
VK_DEVICE_LEVEL_FUNCTION( vkCmdBindPipeline )
VK_DEVICE_LEVEL_FUNCTION( vkCmdBindVertexBuffers )
VK_DEVICE_LEVEL_FUNCTION( vkCmdBindIndexBuffer )
VK_DEVICE_LEVEL_FUNCTION( vkCmdBindDescriptorSets )
VK_DEVICE_LEVEL_FUNCTION( vkCmdDraw )
VK_DEVICE_LEVEL_FUNCTION( vkCmdDrawIndexed )
VK_DEVICE_LEVEL_FUNCTION( vkQueueSubmit )
VK_DEVICE_LEVEL_FUNCTION( vkQueuePresentKHR )
VK_DEVICE_LEVEL_FUNCTION( vkGetBufferMemoryRequirements )
VK_DEVICE_LEVEL_FUNCTION( vkAllocateMemory )
VK_DEVICE_LEVEL_FUNCTION( vkFreeMemory )
VK_DEVICE_LEVEL_FUNCTION( vkBindBufferMemory )
VK_DEVICE_LEVEL_FUNCTION( vkMapMemory )
VK_DEVICE_LEVEL_FUNCTION( vkUnmapMemory )
VK_DEVICE_LEVEL_FUNCTION( vkFlushMappedMemoryRanges )
VK_DEVICE_LEVEL_FUNCTION( vkCreateDescriptorSetLayout )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDescriptorSetLayout )
VK_DEVICE_LEVEL_FUNCTION( vkCreateDescriptorPool )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDescriptorPool )
VK_DEVICE_LEVEL_FUNCTION( vkAllocateDescriptorSets )
VK_DEVICE_LEVEL_FUNCTION( vkFreeDescriptorSets )
VK_DEVICE_LEVEL_FUNCTION( vkUpdateDescriptorSets )

#undef VK_DEVICE_LEVEL_FUNCTION
#endif