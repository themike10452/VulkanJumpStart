#include "vulkan/vulkan.h"
#include "glm/common.hpp"
#include "vkfw.h"
#include "vkptr.h"
#include "smartptr.h"
#include "tools.h"
#include "helper_macros.h"

#include <iostream>
#include <vector>
#include <set>
#include <assert.h>

#define F_VERTEX_SHADER     "../Data/Shaders/vert.spv"
#define F_FRAGMENT_SHADER   "../Data/Shaders/frag.spv"

#ifdef VKFW_ENABLE_VALIDATION

VKAPI_ATTR VkBool32 VKAPI_CALL fnDebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

    char t = '?';

    if (flags == VK_DEBUG_REPORT_WARNING_BIT_EXT)
        t = 'W';
    else if (flags == VK_DEBUG_REPORT_ERROR_BIT_EXT)
        t = 'E';

    std::cerr << "## Validation layer (" << t << "): " << msg << std::endl << std::endl;

	return VK_FALSE;
}

#endif // VKFW_ENABLE_VALIDATION

float vertices[3][3] = {
	{  0.0f, -0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    { -0.5f,  0.5f, 0.0f }
};

struct QueueFamilyIndices
{
	VkfwInt32 graphics = -1;
	VkfwInt32 present = -1;

	VkfwBool iscomplete() const { return graphics > -1 && present > -1; }
};

class VulkanApplication
{
public:
	void Run()
	{
		InitVulkan();
		MainLoop();
	}

private:
	struct VkContext
	{
		VkPtr<VkInstance>				  instance{ &vkDestroyInstance };
#ifdef VKFW_ENABLE_VALIDATION
        VkPtr<VkDebugReportCallbackEXT>   debugCallback{ instance, &vkDestroyDebugReportCallbackEXT };
#endif // VKFW_ENABLE_VALIDATION
		VkPtr<VkSurfaceKHR>				  surface{ instance, &vkDestroySurfaceKHR };
		VkPtr<VkDevice>					  device{ &vkDestroyDevice };
        VkPtr<VkSwapchainKHR>             swapchain{ device, &vkDestroySwapchainKHR };
                                          
		VkPhysicalDevice				  physicalDevice;
                                          
        std::vector<VkImage>              swapchainImages;
        std::vector<VkPtr<VkImageView>>   swapchainImageViews;
        std::vector<VkPtr<VkFramebuffer>> swapchainFramebuffers;
        std::vector<VkCommandBuffer>      swapchainCommandBuffers;
                                          
        VkPtr<VkShaderModule>             vertexShaderModule{ device, &vkDestroyShaderModule };
        VkPtr<VkShaderModule>             fragmentShaderModule{ device, &vkDestroyShaderModule };
        
	    VkPtr<VkBuffer>                   vertexBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkDeviceMemory>             vertexBufferMemory{ device, &vkFreeMemory };

        VkPtr<VkRenderPass>               renderPass{ device, &vkDestroyRenderPass };
        VkPtr<VkPipelineLayout>           pipelineLayout{ device, &vkDestroyPipelineLayout };
        VkPtr<VkPipeline>                 graphicsPipeline{ device, &vkDestroyPipeline };
        VkPtr<VkCommandPool>              commandPool{ device, &vkDestroyCommandPool };

        VkPtr<VkSemaphore>                semaphoreImageAvailable{ device, &vkDestroySemaphore };
        VkPtr<VkSemaphore>                semaphoreRenderFinished{ device, &vkDestroySemaphore };

        struct
        {
            VkQueue graphics;
            VkQueue present;
        } queues;

        struct
        {
            VkSurfaceCapabilitiesKHR capabilities;
            VkSurfaceFormatKHR       format;
            VkPresentModeKHR         presentMode;
            VkExtent2D               extent;
        } swapchainProperties;

	} Vulkan;

	SmartPtr<VkfwWindow*>				window{ vkfwDestroyWindow };

	void InitVulkan()
	{
		this->CreateInstance();
#ifdef VKFW_ENABLE_VALIDATION
        this->SetupDebugLogging();
#endif // VKFW_ENABLE_VALIDATION
		this->CreateWindowSurface();
		this->SelectPhysicalDevice();
		this->CreateLogicalDevice();
		this->CreateSwapChain();
        this->CreateRenderPass();
        this->CreateGraphicsPipeline();
        this->CreateFrameBuffers();
        this->CreateVertexBuffer();
        this->CreateCommandPool();
        this->AllocateCommandBuffers();
        this->RecordCommandBuffers();
        this->CreateSemaphores();
	}

	void MainLoop()
	{
        VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

        while ( vkfwWindowShouldClose( window ) != VKFW_TRUE )
        {
            vkfwPollEvents();

            VkfwUint32 imageIndex;
            vkAcquireNextImageKHR(
                Vulkan.device,
                Vulkan.swapchain,
                std::numeric_limits<uint64_t>::max(),
                Vulkan.semaphoreImageAvailable,
                VK_NULL_HANDLE,
                &imageIndex );

            VkSubmitInfo
            submitInfo                      = {};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext                = VK_NULL_HANDLE;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &Vulkan.swapchainCommandBuffers[imageIndex];
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &Vulkan.semaphoreImageAvailable;
            submitInfo.pWaitDstStageMask    = waitStages;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &Vulkan.semaphoreRenderFinished;

            if ( vkQueueSubmit( Vulkan.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
                throw std::runtime_error("Failed to submit command buffer");

            VkPresentInfoKHR
            presentInfo                    = {};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext              = VK_NULL_HANDLE;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = &Vulkan.semaphoreRenderFinished;
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = &Vulkan.swapchain;
            presentInfo.pImageIndices      = &imageIndex;
            presentInfo.pResults           = VK_NULL_HANDLE;

            if ( vkQueuePresentKHR( Vulkan.queues.graphics, &presentInfo ) != VK_SUCCESS )
                throw std::runtime_error("Presentation failed");
        }

        vkQueueWaitIdle( Vulkan.queues.graphics );
        vkDeviceWaitIdle( Vulkan.device );
	}

	void CreateInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = VK_NULL_HANDLE;
		appInfo.pApplicationName = "Vulkan Jump Start";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkfwUint32 requiredExtensionCount;
		const VkfwString* requiredExtensionNames = vkfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		VkfwUint32 layerCount;
		const VkfwString* requiredLayerNames = vkfwGetRequiredInstanceLayers(&layerCount);

		VkInstanceCreateInfo createInfo    = {};
		createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext                   = VK_NULL_HANDLE;
		createInfo.pApplicationInfo        = &appInfo;
		createInfo.flags                   = 0;
		createInfo.enabledExtensionCount   = requiredExtensionCount;
		createInfo.ppEnabledExtensionNames = requiredExtensionNames;
		createInfo.enabledLayerCount       = layerCount;
		createInfo.ppEnabledLayerNames     = requiredLayerNames;

		if (vkCreateInstance(&createInfo, nullptr, Vulkan.instance.Replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance");
		}

		vkfwLoadInstanceLevelEntryPoints(&Vulkan.instance);
	}
    
#ifdef VKFW_ENABLE_VALIDATION
	void SetupDebugLogging()
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext       = VK_NULL_HANDLE;
		createInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = fnDebugReportCallback;
		createInfo.pUserData   = VK_NULL_HANDLE;

		if (vkCreateDebugReportCallbackEXT(Vulkan.instance, &createInfo, nullptr, Vulkan.debugCallback.Replace()) != VK_SUCCESS)
			throw std::runtime_error("CreateDebugReportCallbackEXT failed");
	}
#endif // VKFW_ENABLE_VALIDATION

	void CreateWindowSurface()
	{
		window = vkfwCreateWindow( 800, 600, VTEXT( "Vulkan" ) );

		if (vkfwCreateWindowSurface(&Vulkan.instance, window, nullptr, Vulkan.surface.Replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create surface!");
		}
	}

	void SelectPhysicalDevice()
	{
		std::vector<VkPhysicalDevice> physicalDevices;

		vkfwEnumeratePhysicalDevices(&Vulkan.instance, &physicalDevices);
		VkfwUint32 deviceCount = (VkfwUint32)physicalDevices.size();

		if (deviceCount < 1)
			throw std::runtime_error("Could not find a compatible GPU");

		VkfwUint32 bestDeviceIndex = DetectOptimalPhysicalDevice(physicalDevices.data(), deviceCount, &Vulkan.surface);

		Vulkan.physicalDevice = physicalDevices[bestDeviceIndex];
	}

	void CreateLogicalDevice()
	{
        QueueFamilyIndices deviceQueueFamilyIndices = GetQueueFamilyIndices(&Vulkan.physicalDevice, &Vulkan.surface);
        
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;

        {
            std::set<VkfwUint32> queueFamilyIndices = {};
		    queueFamilyIndices.insert(deviceQueueFamilyIndices.graphics);
		    queueFamilyIndices.insert(deviceQueueFamilyIndices.present);

		    float queuePriority = 1.0f;
		    for (const VkfwUint32& queueFamilyIndex : queueFamilyIndices)
		    {
			    VkDeviceQueueCreateInfo createInfo = {};
			    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			    createInfo.pNext                   = VK_NULL_HANDLE;
			    createInfo.queueFamilyIndex        = queueFamilyIndex;
			    createInfo.queueCount              = 1;
			    createInfo.pQueuePriorities        = &queuePriority;

			    queueCreateInfo.push_back(createInfo);
		    }
        }

        {
            VkPhysicalDeviceFeatures deviceFeatures = {};

		    VkfwUint32 deviceExtensionCount;
		    const VkfwString* deviceExtensions = vkfwGetRequiredDeviceExtensions(&deviceExtensionCount);

		    VkDeviceCreateInfo createInfo      = {};
		    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		    createInfo.pNext                   = VK_NULL_HANDLE;
		    createInfo.pQueueCreateInfos       = queueCreateInfo.data();
		    createInfo.queueCreateInfoCount    = (VkfwUint32)queueCreateInfo.size();
		    createInfo.pEnabledFeatures        = &deviceFeatures;
		    createInfo.enabledExtensionCount   = deviceExtensionCount;
		    createInfo.ppEnabledExtensionNames = deviceExtensions;
		    createInfo.enabledLayerCount       = 0;

		    if (vkCreateDevice(Vulkan.physicalDevice, &createInfo, nullptr, Vulkan.device.Replace()) != VK_SUCCESS)
			    throw std::runtime_error("vkCreateDevice failed");
        }

		vkfwLoadDeviceLevelEntryPoints(&Vulkan.device);

		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.graphics, 0, &Vulkan.queues.graphics);
		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.present, 0, &Vulkan.queues.present);

		if (!Vulkan.queues.graphics || !Vulkan.queues.present)
			throw std::runtime_error("vkGetDeviceQueue failed");
	}

	void CreateSwapChain()
	{
        VkfwSwapchainSupport swapchainSupport = vkfwQuerySwapchainSupport(&Vulkan.physicalDevice, &Vulkan.surface);

        if (swapchainSupport.formats.empty() ||
            swapchainSupport.presentModes.empty())
            throw std::runtime_error("Failed to create swapchain");

        Vulkan.swapchainProperties.capabilities = swapchainSupport.capabilities;
        Vulkan.swapchainProperties.format       = this->DetectOptimalSurfaceFormat( swapchainSupport.formats.data(), swapchainSupport.formats.size() );
        Vulkan.swapchainProperties.presentMode  = this->DetectOptimalPresentMode( swapchainSupport.presentModes.data(), swapchainSupport.presentModes.size() );

        VkfwUint32 minImageCount = Vulkan.swapchainProperties.capabilities.maxImageCount == 0
	        ? Vulkan.swapchainProperties.capabilities.minImageCount + 1
	        : glm::min(Vulkan.swapchainProperties.capabilities.maxImageCount, Vulkan.swapchainProperties.capabilities.minImageCount + 1);

        VkfwUint32 width, height;
        vkfwGetWindowResolution(window, &width, &height);

        Vulkan.swapchainProperties.extent = Vulkan.swapchainProperties.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()
	        ? Vulkan.swapchainProperties.capabilities.currentExtent
	        : VkExtent2D { 
                width  = glm::clamp(width, Vulkan.swapchainProperties.capabilities.minImageExtent.width, Vulkan.swapchainProperties.capabilities.maxImageExtent.width), 
                height = glm::clamp(width, Vulkan.swapchainProperties.capabilities.minImageExtent.height, Vulkan.swapchainProperties.capabilities.maxImageExtent.height)
            };

        {
            VkSwapchainCreateInfoKHR createInfo = {};
            createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.pNext                    = VK_NULL_HANDLE;
            createInfo.surface                  = Vulkan.surface;
            createInfo.imageExtent              = Vulkan.swapchainProperties.extent;
            createInfo.imageColorSpace          = Vulkan.swapchainProperties.format.colorSpace;
            createInfo.presentMode              = Vulkan.swapchainProperties.presentMode;
            createInfo.imageFormat              = Vulkan.swapchainProperties.format.format;
            createInfo.oldSwapchain             = VK_NULL_HANDLE;
            createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.imageArrayLayers         = 1;
            createInfo.minImageCount            = minImageCount;
            createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            createInfo.preTransform             = Vulkan.swapchainProperties.capabilities.currentTransform;
            createInfo.clipped                  = VK_TRUE;

            QueueFamilyIndices indices = GetQueueFamilyIndices( &Vulkan.physicalDevice, &Vulkan.surface );
            VkfwUint32 queueFamilyIndices[] = { indices.graphics, indices.present };

            if (Vulkan.queues.graphics != Vulkan.queues.present)
            {
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices   = queueFamilyIndices;
            }
            else
            {
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
            }

            if (vkCreateSwapchainKHR( Vulkan.device, &createInfo, nullptr, Vulkan.swapchain.Replace() ) != VK_SUCCESS)
                throw std::runtime_error("Failed to create swapchain");
        }

        VkfwUint32 imageCount;
        vkGetSwapchainImagesKHR( Vulkan.device, Vulkan.swapchain, &imageCount, nullptr );
        Vulkan.swapchainImages.resize( imageCount );
        vkGetSwapchainImagesKHR( Vulkan.device, Vulkan.swapchain, &imageCount, Vulkan.swapchainImages.data() );
        
	    Vulkan.swapchainImageViews.resize( imageCount, { Vulkan.device, &vkDestroyImageView } );

        for (VkfwUint32 i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo createInfo           = {};
            createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.pNext                           = VK_NULL_HANDLE;
            createInfo.format                          = Vulkan.swapchainProperties.format.format;
            createInfo.image                           = Vulkan.swapchainImages[i];
            createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;

            if (vkCreateImageView( Vulkan.device, &createInfo, nullptr, Vulkan.swapchainImageViews[i].Replace() ) != VK_SUCCESS)
                throw std::runtime_error("Failed to create image views");
        }
	}

    void CreateRenderPass()
	{
	    VkAttachmentDescription 
	    colorAttachment                = {};
        colorAttachment.format         = Vulkan.swapchainProperties.format.format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.flags          = 0;

        VkAttachmentReference
	    colorAttachmentReference            = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription 
	    subpass                         = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentReference;
        subpass.inputAttachmentCount    = 0;
        subpass.preserveAttachmentCount = 0;

        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                  =  VK_NULL_HANDLE;
        createInfo.attachmentCount        = 1;
        createInfo.pAttachments           = &colorAttachment;
        createInfo.subpassCount           = 1;
        createInfo.pSubpasses             = &subpass;
        createInfo.flags                  = 0;

        if( vkCreateRenderPass( Vulkan.device, &createInfo, nullptr, Vulkan.renderPass.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create render pass");
	}

    void CreateGraphicsPipeline()
	{
	    std::vector<VkfwChar> vertexShaderCode,
	                          fragmentShaderCode;

        tools::readBinaryFile( F_VERTEX_SHADER,   &vertexShaderCode );
        tools::readBinaryFile( F_FRAGMENT_SHADER, &fragmentShaderCode);

        if (vkfwCreateShaderModule( &Vulkan.device, vertexShaderCode.data(), vertexShaderCode.size(), nullptr, Vulkan.vertexShaderModule.Replace() ) != VK_SUCCESS)
            throw std::runtime_error("Failed to create vertex shader module");

        if (vkfwCreateShaderModule( &Vulkan.device, fragmentShaderCode.data(), fragmentShaderCode.size(), nullptr, Vulkan.fragmentShaderModule.Replace() ) != VK_SUCCESS)
            throw std::runtime_error("Failed to create fragment shader module");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {};

	    {
            // Vertex shader stage
	        VkPipelineShaderStageCreateInfo 
	        vertexShaderStageInfo                     = {};
            vertexShaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertexShaderStageInfo.pNext               = VK_NULL_HANDLE;
            vertexShaderStageInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
            vertexShaderStageInfo.module              = Vulkan.vertexShaderModule;
            vertexShaderStageInfo.pName               = "main";
            vertexShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;
            vertexShaderStageInfo.flags               = 0;

            shaderStages[0] = vertexShaderStageInfo;
	    }

	    {
	        // Fragment shader stage
            VkPipelineShaderStageCreateInfo
            fragmentShaderStageInfo                     = {};
            fragmentShaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragmentShaderStageInfo.pNext               = VK_NULL_HANDLE;
            fragmentShaderStageInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragmentShaderStageInfo.module              = Vulkan.fragmentShaderModule;
            fragmentShaderStageInfo.pName               = "main";
            fragmentShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;
            fragmentShaderStageInfo.flags               = 0;
            
            shaderStages[1] = fragmentShaderStageInfo;
	    }
        
        VkVertexInputBindingDescription
	    bindingDescription            = {};
        bindingDescription.binding    = 0;
        bindingDescription.inputRate  = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride     = 3 * sizeof(float);

        VkVertexInputAttributeDescription
        attributeDescription          = {};
        attributeDescription.binding  = 0;
        attributeDescription.location = 0;
        attributeDescription.format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription.offset   = 0;

        // Vertex input state
        VkPipelineVertexInputStateCreateInfo
        vertexInputStateInfo                                 = {};
        vertexInputStateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateInfo.pNext                           = VK_NULL_HANDLE;
        vertexInputStateInfo.vertexBindingDescriptionCount   = 1;
        vertexInputStateInfo.pVertexBindingDescriptions      = &bindingDescription;
        vertexInputStateInfo.vertexAttributeDescriptionCount = 1;
        vertexInputStateInfo.pVertexAttributeDescriptions    = &attributeDescription;
        vertexInputStateInfo.flags                           = 0;

        // Input assembly state
        VkPipelineInputAssemblyStateCreateInfo
        inputAssemblyStateInfo                        = {};
        inputAssemblyStateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateInfo.pNext                  = VK_NULL_HANDLE;
        inputAssemblyStateInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;
        inputAssemblyStateInfo.flags                  = 0;

        // Viewport and scissor
        VkViewport viewport = {};
        viewport.x          = 0;
        viewport.y          = 0;
        viewport.width      = float(Vulkan.swapchainProperties.extent.width);
        viewport.height     = float(Vulkan.swapchainProperties.extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent   = Vulkan.swapchainProperties.extent;

        // Viewport state
        VkPipelineViewportStateCreateInfo
        viewportStateInfo               = {};
        viewportStateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateInfo.pNext         = VK_NULL_HANDLE;
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports    = &viewport;
        viewportStateInfo.scissorCount  = 1;
        viewportStateInfo.pScissors     = &scissor;
        viewportStateInfo.flags         = 0;

        // Rasterizer state
        VkPipelineRasterizationStateCreateInfo
        rasterizationStateInfo                         = {};
        rasterizationStateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateInfo.pNext                   = VK_NULL_HANDLE;
        rasterizationStateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizationStateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizationStateInfo.lineWidth               = 1.0f;
        rasterizationStateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateInfo.depthBiasEnable         = VK_FALSE;
        rasterizationStateInfo.depthClampEnable        = VK_FALSE;
        rasterizationStateInfo.depthBiasClamp          = 0.0f;
        rasterizationStateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateInfo.depthBiasSlopeFactor    = 0.0f;
        rasterizationStateInfo.flags                   = 0;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo
        multisampleStateInfo                       = {};
        multisampleStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateInfo.pNext                 = VK_NULL_HANDLE;
        multisampleStateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateInfo.sampleShadingEnable   = VK_FALSE;
        multisampleStateInfo.minSampleShading      = 1.0f;
        multisampleStateInfo.pSampleMask           = VK_NULL_HANDLE;
        multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateInfo.alphaToOneEnable      = VK_FALSE;
        multisampleStateInfo.flags                 = 0;

        // Depth and stencil TODO
        //VkPipelineDepthStencilStateCreateInfo
        //depthStencilStateInfo = {};
        //depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        // Color blend
        VkPipelineColorBlendAttachmentState
        blendAttachmentState                     = {};
        blendAttachmentState.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.blendEnable         = VK_FALSE;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo
        colorBlendStateInfo                 = {};
        colorBlendStateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateInfo.pNext           = VK_NULL_HANDLE;
        colorBlendStateInfo.logicOpEnable   = VK_FALSE;
        colorBlendStateInfo.logicOp         = VK_LOGIC_OP_COPY;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments    = &blendAttachmentState;
        colorBlendStateInfo.flags           = 0;

        // Pipleline layout
        VkPipelineLayoutCreateInfo
        pipelineLayoutInfo                        = {};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext                  = VK_NULL_HANDLE;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pSetLayouts            = VK_NULL_HANDLE;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = VK_NULL_HANDLE;

        if ( vkCreatePipelineLayout( Vulkan.device, &pipelineLayoutInfo, nullptr, Vulkan.pipelineLayout.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo 
	    graphicsPipelineInfo                     = {};
        graphicsPipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineInfo.pNext               =  VK_NULL_HANDLE;
        graphicsPipelineInfo.stageCount          = 2;
        graphicsPipelineInfo.pStages             = shaderStages;
        graphicsPipelineInfo.pVertexInputState   = &vertexInputStateInfo;
        graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
        graphicsPipelineInfo.pViewportState      = &viewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
        graphicsPipelineInfo.pMultisampleState   = &multisampleStateInfo;
        graphicsPipelineInfo.pColorBlendState    = &colorBlendStateInfo;
        graphicsPipelineInfo.pDepthStencilState  = VK_NULL_HANDLE;
        graphicsPipelineInfo.pDynamicState       = VK_NULL_HANDLE;
        graphicsPipelineInfo.layout              = Vulkan.pipelineLayout;
        graphicsPipelineInfo.renderPass          = Vulkan.renderPass;
        graphicsPipelineInfo.subpass             = 0;
        graphicsPipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        graphicsPipelineInfo.basePipelineIndex   = -1;
        graphicsPipelineInfo.flags               = 0;

        if ( vkCreateGraphicsPipelines( Vulkan.device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, Vulkan.graphicsPipeline.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create graphics pipeline");
	}
    
    void CreateFrameBuffers()
	{
        VkfwSize imageCount = Vulkan.swapchainImageViews.size();

	    Vulkan.swapchainFramebuffers.resize( imageCount, { Vulkan.device, &vkDestroyFramebuffer } );

        for (VkfwSize i = 0; i < imageCount; i++)
        {
            VkImageView attachments[] = { Vulkan.swapchainImageViews[i] };

            VkFramebufferCreateInfo
            createInfo                 = {};
            createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext           = VK_NULL_HANDLE;
            createInfo.renderPass      = Vulkan.renderPass;
            createInfo.attachmentCount = 1;
            createInfo.pAttachments    = attachments;
            createInfo.width           = Vulkan.swapchainProperties.extent.width;
            createInfo.height          = Vulkan.swapchainProperties.extent.height;
            createInfo.layers          = 1;
            createInfo.flags           = 0;

            if ( vkCreateFramebuffer( Vulkan.device, &createInfo, nullptr, Vulkan.swapchainFramebuffers[i].Replace() ) != VK_SUCCESS )
                throw std::runtime_error("Failed to create swapchain framebuffers");
        }
	}

    void CreateCommandPool()
	{
        QueueFamilyIndices indices = GetQueueFamilyIndices( &Vulkan.physicalDevice, &Vulkan.surface );

	    VkCommandPoolCreateInfo
        createInfo                  = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = VK_NULL_HANDLE;
        createInfo.queueFamilyIndex = indices.graphics;
        createInfo.flags            =  0;

        if ( vkCreateCommandPool( Vulkan.device, &createInfo, nullptr, Vulkan.commandPool.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create command pool");
	}

    void CreateVertexBuffer()
	{
	    VkBufferCreateInfo
	    bufferInfo             = {};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext       = nullptr;
        bufferInfo.size        = 9 * sizeof(float);
        bufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if ( vkCreateBuffer( Vulkan.device, &bufferInfo, nullptr, Vulkan.vertexBuffer.Replace() ) != VK_SUCCESS )
            throw std::exception("Failed to create vertex buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements( Vulkan.device, Vulkan.vertexBuffer, &memRequirements );

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties( Vulkan.physicalDevice, &properties );

        VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT/* | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/;

        VkfwInt32 memoryTypeIndex = -1;
        for (VkfwUint32 i = 0; i < properties.memoryTypeCount && memoryTypeIndex == -1; i++)
        {
            if (memRequirements.memoryTypeBits & (1 << i) && (properties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
                memoryTypeIndex = i;
        }

        if (memoryTypeIndex == -1)
            throw std::exception("Failed to allocate buffer memory");

        VkMemoryAllocateInfo 
	    allocateInfo                 = {};
        allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext           = nullptr;
        allocateInfo.allocationSize  = memRequirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        if ( vkAllocateMemory( Vulkan.device, &allocateInfo, nullptr, Vulkan.vertexBufferMemory.Replace() ) != VK_SUCCESS )
            throw std::exception("Failed to allocate buffer memory");

        vkBindBufferMemory( Vulkan.device, Vulkan.vertexBuffer, Vulkan.vertexBufferMemory, 0 );

        // copy the vertex data to VRAM
        void* pData;
        vkMapMemory( Vulkan.device, Vulkan.vertexBufferMemory, 0, bufferInfo.size, 0, &pData );
        memcpy( pData, vertices, bufferInfo.size );

        // flush
        VkMappedMemoryRange 
	    range        = {};
        range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext  = nullptr;
        range.size   = bufferInfo.size;
        range.memory = Vulkan.vertexBufferMemory;
        range.offset = 0;

        if ( vkFlushMappedMemoryRanges( Vulkan.device, 1, &range ) != VK_SUCCESS )
            throw std::exception("Failed to flush vertex buffer data");

        vkUnmapMemory( Vulkan.device, Vulkan.vertexBufferMemory );
	}

    void AllocateCommandBuffers()
	{
        if (Vulkan.swapchainCommandBuffers.size() > 0)
        {
            vkFreeCommandBuffers(Vulkan.device, Vulkan.commandPool, Vulkan.swapchainCommandBuffers.size(), Vulkan.swapchainCommandBuffers.data());
        }

	    VkfwSize count = Vulkan.swapchainFramebuffers.size();
        Vulkan.swapchainCommandBuffers.resize( count );

        VkCommandBufferAllocateInfo 
	    allocateInfo                    = {};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool        = Vulkan.commandPool;
        allocateInfo.commandBufferCount = count;
        allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if ( vkAllocateCommandBuffers( Vulkan.device, &allocateInfo, Vulkan.swapchainCommandBuffers.data() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate commandbuffers from commandpool");
	}

    void RecordCommandBuffers()
	{
        VkClearValue clearColor = { 0.0f };

	    for (VkfwSize i = 0; i < Vulkan.swapchainCommandBuffers.size(); i++)
	    {
	        VkCommandBufferBeginInfo
	        beginInfo                          = {};
            beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext                    = VK_NULL_HANDLE;
            beginInfo.pInheritanceInfo         = VK_NULL_HANDLE;
            beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            VkRenderPassBeginInfo
	        renderPassInfo                     = {};
            renderPassInfo.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass          = Vulkan.renderPass;
            renderPassInfo.framebuffer         = Vulkan.swapchainFramebuffers[i];
            renderPassInfo.renderArea.offset.x = 0;
            renderPassInfo.renderArea.offset.y = 0;
            renderPassInfo.renderArea.extent   = Vulkan.swapchainProperties.extent;
            renderPassInfo.clearValueCount     = 1;
            renderPassInfo.pClearValues        = &clearColor;
            
            vkBeginCommandBuffer( Vulkan.swapchainCommandBuffers[i], &beginInfo );
            vkCmdBeginRenderPass( Vulkan.swapchainCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

            VkDeviceSize offset = 0;

            vkCmdBindPipeline( Vulkan.swapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan.graphicsPipeline );
            vkCmdBindVertexBuffers( Vulkan.swapchainCommandBuffers[i], 0, 1, &Vulkan.vertexBuffer, &offset );
            vkCmdDraw( Vulkan.swapchainCommandBuffers[i], 3, 1, 0, 0 );

            vkCmdEndRenderPass( Vulkan.swapchainCommandBuffers[i] );
            
	        if ( vkEndCommandBuffer( Vulkan.swapchainCommandBuffers[i] ) != VK_SUCCESS )
                throw std::runtime_error("Failed to record command buffer");
	    }
	}

    void CreateSemaphores()
	{
	    VkSemaphoreCreateInfo
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = VK_NULL_HANDLE;
        createInfo.flags = 0;

        if ( vkCreateSemaphore( Vulkan.device, &createInfo, nullptr, Vulkan.semaphoreImageAvailable.Replace() ) != VK_SUCCESS ||
             vkCreateSemaphore( Vulkan.device, &createInfo, nullptr, Vulkan.semaphoreRenderFinished.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create semaphores");
	}

#pragma region

	static VkfwUint32 DetectOptimalPhysicalDevice(const VkPhysicalDevice* pPhysicalDevices, VkfwUint32 deviceCount, const VkSurfaceKHR* pSurface)
	{
        assert( pPhysicalDevices != VK_NULL_HANDLE );
        assert( pSurface != VK_NULL_HANDLE );

		VkfwUint32
			bestScore = 0,
			bestDeviceIndex = 0;

		for (VkfwUint32 i = 0; i < deviceCount; i++)
		{
			VkPhysicalDeviceProperties deviceProperties = {};
			vkGetPhysicalDeviceProperties(pPhysicalDevices[i], &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures = {};
			vkGetPhysicalDeviceFeatures(pPhysicalDevices[i], &deviceFeatures);

			QueueFamilyIndices queueFamilyIndices = GetQueueFamilyIndices(&pPhysicalDevices[i], pSurface);

			VkfwUint32 score = 0;

			score += deviceProperties.limits.maxImageDimension2D;

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 10000;

			if (!deviceFeatures.geometryShader || 
				!vkfwCheckDeviceExtensionSupport(&pPhysicalDevices[i]) ||
				!queueFamilyIndices.iscomplete())
				score = 0;

			if (score > bestScore)
			{
				bestScore = score;
				bestDeviceIndex = i;
			}
		}

		if (bestScore == 0)
			throw std::runtime_error("Unable to find a compatible GPU");

		return bestDeviceIndex;
	}

	static QueueFamilyIndices GetQueueFamilyIndices(const VkPhysicalDevice* pPhysicalDevice, const VkSurfaceKHR* pSurface)
	{
        assert( pPhysicalDevice );
        assert( pSurface );

		QueueFamilyIndices queueFamilyIndices;

		VkfwUint32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(*pPhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(*pPhysicalDevice, &queueFamilyCount, queueFamilies.data());

		VkfwUint32 i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			VkBool32 presentSupported = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(*pPhysicalDevice, i, *pSurface, &presentSupported);

			if (presentSupported)
				queueFamilyIndices.present = i;

			if (queueFamily.queueCount > 0 &&
				queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
				queueFamilyIndices.graphics < 0)
			{
				queueFamilyIndices.graphics = i;
			}

			if (queueFamilyIndices.iscomplete())
				break;

			i++;
		}

		return queueFamilyIndices;
	}

    static VkSurfaceFormatKHR DetectOptimalSurfaceFormat(const VkSurfaceFormatKHR* pSupportedFormats, VkfwUint32 count)
	{
        assert( pSupportedFormats );

        if (count == 1 && pSupportedFormats[0].format == VK_FORMAT_UNDEFINED)
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (VkfwUint32 i = 0; i < count; i++)
        {
            if (pSupportedFormats[i].format     == VK_FORMAT_B8G8R8A8_UNORM && 
                pSupportedFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return pSupportedFormats[i];
        }

        return pSupportedFormats[0];
	}

    static VkPresentModeKHR DetectOptimalPresentMode(const VkPresentModeKHR* pSupportedModes, VkfwUint32 count)
	{
        for (VkfwUint32 i = 0; i < count; i++)
        {
            if (pSupportedModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                return pSupportedModes[i];

            if (pSupportedModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                return pSupportedModes[i];
        }

        return VK_PRESENT_MODE_FIFO_KHR;
	}

#pragma endregion Helper Methods
};

int main(int argc, char** argv)
{
    vkfwInit();

	try
	{
        VulkanApplication application;
		application.Run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

    vkfwDestroy();

	return EXIT_SUCCESS;
}