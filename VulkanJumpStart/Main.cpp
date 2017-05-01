#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "vkptr.h"
#include "smartptr.h"
#include "tools.h"
#include "glm/common.hpp"

#include <iostream>
#include <vector>
#include <set>
#include <assert.h>

#define F_VERTEX_SHADER     "../Data/Shaders/vert.spv"
#define F_FRAGMENT_SHADER   "../Data/Shaders/frag.spv"

#ifdef _DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL fnDebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "Validation layer: " << msg << std::endl;

	return VK_FALSE;
}

#endif // _DEBUG

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
		vkfwInit();
		InitVulkan();
		MainLoop();
		vkfwDestroy();
	}

private:
	struct VkContext
	{
		VkPtr<VkInstance>				instance{ &vkDestroyInstance };
		VkPtr<VkDebugReportCallbackEXT> debugCallback{ instance, &vkDestroyDebugReportCallbackEXT };
		VkPtr<VkSurfaceKHR>				surface{ instance, &vkDestroySurfaceKHR };
		VkPtr<VkDevice>					device{ &vkDestroyDevice };
        VkPtr<VkSwapchainKHR>           swapchain{ device, &vkDestroySwapchainKHR };

		VkPhysicalDevice				physicalDevice;

        std::vector<VkImage>            swapchainImages;
        std::vector<VkPtr<VkImageView>> swapchainImageViews;
        
        VkPtr<VkShaderModule>           vertexShaderModule{ device, &vkDestroyShaderModule };
        VkPtr<VkShaderModule>           fragmentShaderModule{ device, &vkDestroyShaderModule };

        VkPtr<VkRenderPass>             renderPass{ device, &vkDestroyRenderPass };
        VkPtr<VkPipelineLayout>         pipelineLayout{ device, &vkDestroyPipelineLayout };
        VkPtr<VkPipeline>               graphicsPipeline{ device, &vkDestroyPipeline };

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
		this->SetupDebugLogging();
		this->CreateWindowSurface();
		this->SelectPhysicalDevice();
		this->CreateLogicalDevice();
		this->CreateSwapChain();
        this->CreateRenderPass();
        this->CreateGraphicsPipeline();
	}

	void MainLoop()
	{

	}

	void CreateInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
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
		createInfo.pNext                   = nullptr;
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

	void SetupDebugLogging()
	{
#ifdef _DEBUG
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext       = nullptr;
		createInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = fnDebugReportCallback;
		createInfo.pUserData   = nullptr;

		if (vkCreateDebugReportCallbackEXT(Vulkan.instance, &createInfo, nullptr, Vulkan.debugCallback.Replace()) != VK_SUCCESS)
			throw std::runtime_error("CreateDebugReportCallbackEXT failed");
#endif // _DEBUG
	}

	void CreateWindowSurface()
	{
		window = vkfwCreateWindow(800, 600, "Vulkan");

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
			    createInfo.pNext                   = nullptr;
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
		    createInfo.pNext                   = nullptr;
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
            createInfo.pNext                    = nullptr;
            createInfo.surface                  = Vulkan.surface;
            createInfo.imageExtent              = Vulkan.swapchainProperties.extent;
            createInfo.imageColorSpace          = Vulkan.swapchainProperties.format.colorSpace;
            createInfo.presentMode              = Vulkan.swapchainProperties.presentMode;
            createInfo.imageFormat              = Vulkan.swapchainProperties.format.format;
            createInfo.oldSwapchain             = nullptr;
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
                createInfo.pQueueFamilyIndices   = nullptr;
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
            createInfo.pNext                           = nullptr;
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
        createInfo.pNext                  =  nullptr;
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
            vertexShaderStageInfo.pNext               = nullptr;
            vertexShaderStageInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
            vertexShaderStageInfo.module              = Vulkan.vertexShaderModule;
            vertexShaderStageInfo.pName               = "main";
            vertexShaderStageInfo.pSpecializationInfo = nullptr;
            vertexShaderStageInfo.flags               = 0;

            shaderStages[0] = vertexShaderStageInfo;
	    }

	    {
	        // Fragment shader stage
            VkPipelineShaderStageCreateInfo
            fragmentShaderStageInfo                     = {};
            fragmentShaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragmentShaderStageInfo.pNext               = nullptr;
            fragmentShaderStageInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragmentShaderStageInfo.module              = Vulkan.fragmentShaderModule;
            fragmentShaderStageInfo.pName               = "main";
            fragmentShaderStageInfo.pSpecializationInfo = nullptr;
            fragmentShaderStageInfo.flags               = 0;
            
            shaderStages[1] = fragmentShaderStageInfo;
	    }

        // Vertex input state
        VkPipelineVertexInputStateCreateInfo
        vertexInputStateInfo                                 = {};
        vertexInputStateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateInfo.pNext                           = nullptr;
        vertexInputStateInfo.vertexBindingDescriptionCount   = 0;
        vertexInputStateInfo.pVertexBindingDescriptions      = nullptr;
        vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateInfo.pVertexAttributeDescriptions    = nullptr;
        vertexInputStateInfo.flags                           = 0;

        // Input assembly state
        VkPipelineInputAssemblyStateCreateInfo
        inputAssemblyStateInfo                        = {};
        inputAssemblyStateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateInfo.pNext                  = nullptr;
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
        viewportStateInfo.pNext         = nullptr;
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports    = &viewport;
        viewportStateInfo.scissorCount  = 1;
        viewportStateInfo.pScissors     = &scissor;
        viewportStateInfo.flags         = 0;

        // Rasterizer state
        VkPipelineRasterizationStateCreateInfo
        rasterizationStateInfo                         = {};
        rasterizationStateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateInfo.pNext                   = nullptr;
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
        multisampleStateInfo.pNext                 = nullptr;
        multisampleStateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateInfo.sampleShadingEnable   = VK_FALSE;
        multisampleStateInfo.minSampleShading      = 1.0f;
        multisampleStateInfo.pSampleMask           = nullptr;
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
        colorBlendStateInfo.pNext           = nullptr;
        colorBlendStateInfo.logicOpEnable   = VK_FALSE;
        colorBlendStateInfo.logicOp         = VK_LOGIC_OP_COPY;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments    = &blendAttachmentState;
        colorBlendStateInfo.flags           = 0;

        // Pipleline layout
        VkPipelineLayoutCreateInfo
        pipelineLayoutInfo                        = {};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext                  = nullptr;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pSetLayouts            = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = nullptr;

        if ( vkCreatePipelineLayout( Vulkan.device, &pipelineLayoutInfo, nullptr, Vulkan.pipelineLayout.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo 
	    graphicsPipelineInfo                     = {};
        graphicsPipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineInfo.pNext               =  nullptr;
        graphicsPipelineInfo.stageCount          = 2;
        graphicsPipelineInfo.pStages             = shaderStages;
        graphicsPipelineInfo.pVertexInputState   = &vertexInputStateInfo;
        graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
        graphicsPipelineInfo.pViewportState      = &viewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
        graphicsPipelineInfo.pMultisampleState   = &multisampleStateInfo;
        graphicsPipelineInfo.pColorBlendState    = &colorBlendStateInfo;
        graphicsPipelineInfo.pDepthStencilState  = nullptr;
        graphicsPipelineInfo.pDynamicState       = nullptr;
        graphicsPipelineInfo.layout              = Vulkan.pipelineLayout;
        graphicsPipelineInfo.renderPass          = Vulkan.renderPass;
        graphicsPipelineInfo.subpass             = 0;
        graphicsPipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        graphicsPipelineInfo.basePipelineIndex   = -1;
        graphicsPipelineInfo.flags               = 0;

        if ( vkCreateGraphicsPipelines( Vulkan.device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, Vulkan.graphicsPipeline.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create graphics pipeline");
	}

#pragma region

	static VkfwUint32 DetectOptimalPhysicalDevice(const VkPhysicalDevice* pPhysicalDevices, VkfwUint32 deviceCount, const VkSurfaceKHR* pSurface)
	{
        assert( pPhysicalDevices != nullptr );
        assert( pSurface != nullptr );

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
        assert(pPhysicalDevice != nullptr);
        assert(pSurface != nullptr);

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
        assert( pSupportedFormats != nullptr );

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
	VulkanApplication application;

	try
	{
		application.Run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}