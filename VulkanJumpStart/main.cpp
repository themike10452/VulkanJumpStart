#define GLM_FORCE_RADIANS
#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "vkptr.h"
#include "smartptr.h"
#include "tools.h"
#include "helper_macros.h"
#include "geometry/model.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <chrono>
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

struct QueueFamilyIndices
{
	VkfwInt32 graphics = -1;
	VkfwInt32 present = -1;

	VkfwBool iscomplete() const { return graphics > -1 && present > -1; }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct DepthBuffer
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
};

class VulkanApplication
{
public:
    VulkanApplication()
    {
        model = Model();

        //LoadModel( "C:/Users/Mike/Documents/Visual Studio 2017/Projects/VulkanJumpStart/VulkanJumpStart/Data/Models/scene1.mdl", &model );
        LoadModel( "../Data/Models/cube.mdl", &model );
    }

	void Run()
	{
		InitVulkan();
		MainLoop();
	}

private:
    Model model;

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

        DepthBuffer                       depthBuffer;

        VkPtr<VkShaderModule>             vertexShaderModule{ device, &vkDestroyShaderModule };
        VkPtr<VkShaderModule>             fragmentShaderModule{ device, &vkDestroyShaderModule };
        
	    VkPtr<VkBuffer>                   vertexBuffer{ device, &vkDestroyBuffer };
	    VkPtr<VkBuffer>                   vertexStagingBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkBuffer>                   indexBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkBuffer>                   indexStagingBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkBuffer>                   uniformBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkBuffer>                   uniformStagingBuffer{ device, &vkDestroyBuffer };
        VkPtr<VkDeviceMemory>             vertexBufferMemory{ device, &vkFreeMemory };
        VkPtr<VkDeviceMemory>             vertexStagingBufferMemory{ device, &vkFreeMemory };
        VkPtr<VkDeviceMemory>             indexBufferMemory{ device, &vkFreeMemory };
        VkPtr<VkDeviceMemory>             indexStagingBufferMemory{ device, &vkFreeMemory };
        VkPtr<VkDeviceMemory>             uniformBufferMemory{ device, &vkFreeMemory };
        VkPtr<VkDeviceMemory>             uniformStagingBufferMemory{ device, &vkFreeMemory };

        VkPtr<VkDescriptorSetLayout>      descriptorSetLayout{ device, &vkDestroyDescriptorSetLayout };
        VkPtr<VkDescriptorPool>           descriptorPool{ device, &vkDestroyDescriptorPool };

        VkDescriptorSet                   descriptorSet;

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
        this->CreateDescriptorSetLayout();
        this->CreateGraphicsPipeline();
        this->CreateDepthResources();
        this->CreateFrameBuffers();
        this->CreateCommandPool();
        this->CreateVertexBuffer();
        this->CreateIndexBuffer();
        this->CreateUniformBuffer();
        this->CreateDescriptorPool();
        this->CreateDescriptorSets();
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

            UpdateUniformBuffer();

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

        vkDestroyImageView( Vulkan.device, Vulkan.depthBuffer.imageView, nullptr );
        vkFreeMemory( Vulkan.device, Vulkan.depthBuffer.memory, nullptr );
        vkDestroyImage( Vulkan.device, Vulkan.depthBuffer.image, nullptr );
	}

    void UpdateUniformBuffer() const
	{
        static auto rotation = glm::mat4();
        
        float hAngle = 0.0f;
        float vAngle = 0.0f;

        if (vkfwGetKeyState( VKFW_KEY_A ) == VKFW_PRESSED)
            hAngle -= 0.05f;

        if (vkfwGetKeyState( VKFW_KEY_D ) == VKFW_PRESSED)
            hAngle += 0.05f;

        if (vkfwGetKeyState( VKFW_KEY_W ) == VKFW_PRESSED)
            vAngle += 0.05f;

        if (vkfwGetKeyState( VKFW_KEY_S ) == VKFW_PRESSED)
            vAngle -= 0.05f;

        if (vkfwGetKeyState( VKFW_KEY_CAPSLOCK ) == VKFW_TOGGLED)
        {
            vAngle *= 2;
            hAngle *= 2;
        }

        if (vkfwGetMouseButtonState( VKFW_MOUSE_BUTTON_LEFT ) == VKFW_PRESSED)
        {
            rotation = glm::mat4();
        }

        rotation = glm::rotate( rotation, glm::radians( hAngle ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        rotation = glm::rotate( rotation, glm::radians( vAngle ), glm::vec3( 1.0f, 0.0f, 0.0f ) );

        UniformBufferObject uniform = {};
        uniform.model = rotation;
        uniform.view  = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        uniform.projection = glm::perspective( glm::radians( 45.0f ), (float) Vulkan.swapchainProperties.extent.width / Vulkan.swapchainProperties.extent.height, 0.1f, 10.0f );
        uniform.projection[1][1] *= -1;
        
        void* pData;
        vkMapMemory( Vulkan.device, Vulkan.uniformStagingBufferMemory, 0, sizeof uniform, 0, &pData );

        memcpy( pData, &uniform, sizeof uniform );

        // flush
        VkMappedMemoryRange 
	    range        = {};
        range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext  = VK_NULL_HANDLE;
        range.size   = sizeof uniform;
        range.memory = Vulkan.uniformStagingBufferMemory;
        range.offset = 0;

        if ( vkFlushMappedMemoryRanges( Vulkan.device, 1, &range ) != VK_SUCCESS )
            throw std::runtime_error("Failed to flush vertex buffer data");

        vkUnmapMemory( Vulkan.device, Vulkan.uniformStagingBufferMemory );

        CopyBuffer( &Vulkan.uniformStagingBuffer, &Vulkan.uniformBuffer, sizeof uniform );
	}

	void CreateInstance()
	{
		VkApplicationInfo appInfo  = {};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext              = VK_NULL_HANDLE;
		appInfo.pApplicationName   = "Vulkan Jump Start";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName        = "";
		appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion         = VK_API_VERSION_1_0;

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
		VkDebugReportCallbackCreateInfoEXT 
	    createInfo             = {};
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
            VkSwapchainCreateInfoKHR
            createInfo                          = {};
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
        std::vector<VkAttachmentDescription> attachments = {};

        {
            VkAttachmentDescription 
	        colorAttachment                     = {};
            colorAttachment.format              = Vulkan.swapchainProperties.format.format;
            colorAttachment.samples             = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp              = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp             = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp       = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp      = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout         = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            colorAttachment.flags               = 0;

            attachments.push_back( colorAttachment );
        }

        {
            VkAttachmentDescription
            depthAttachment                = {};
            depthAttachment.format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.push_back( depthAttachment );
        }

        VkAttachmentReference
	    colorAttachmentReference            = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference
        depthAttachmentReference            = {};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription 
	    subpass                             = {};
        subpass.pipelineBindPoint           = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount        = 1;
        subpass.inputAttachmentCount        = 0;
        subpass.preserveAttachmentCount     = 0;
        subpass.pColorAttachments           = &colorAttachmentReference;
        subpass.pDepthStencilAttachment     = &depthAttachmentReference;

        VkRenderPassCreateInfo createInfo   = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                    = VK_NULL_HANDLE;
        createInfo.attachmentCount          = attachments.size();
        createInfo.pAttachments             = attachments.data();
        createInfo.subpassCount             = 1;
        createInfo.pSubpasses               = &subpass;
        createInfo.flags                    = 0;

        if( vkCreateRenderPass( Vulkan.device, &createInfo, nullptr, Vulkan.renderPass.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create render pass");
	}
    
    void CreateDescriptorSetLayout()
	{
	    VkDescriptorSetLayoutBinding
	    binding                    = {};
        binding.binding            = 0;
        binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount    = 1;
        binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        binding.pImmutableSamplers = VK_NULL_HANDLE;

        VkDescriptorSetLayoutCreateInfo
        createInfo                 = {};
        createInfo.sType           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext           = VK_NULL_HANDLE;
        createInfo.bindingCount    = 1;
        createInfo.pBindings       = &binding;

        if ( vkCreateDescriptorSetLayout( Vulkan.device, &createInfo, nullptr, Vulkan.descriptorSetLayout.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create descriptorset layout");
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
        bindingDescription.stride     = sizeof model.Vertices[0];

        VkVertexInputAttributeDescription
        attributeDescription          = {};
        attributeDescription.binding  = 0;
        attributeDescription.location = 0;
        attributeDescription.format   = VK_FORMAT_R32G32B32_SFLOAT;
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
        rasterizationStateInfo.cullMode                = VK_CULL_MODE_NONE;
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

        // Depth and stencil
        VkPipelineDepthStencilStateCreateInfo
        depthStencilStateInfo                       = {};
        depthStencilStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateInfo.depthTestEnable       = VK_TRUE;
        depthStencilStateInfo.depthWriteEnable      = VK_TRUE;
        depthStencilStateInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo.stencilTestEnable     = VK_FALSE;

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
        pipelineLayoutInfo.setLayoutCount         = 1;
        pipelineLayoutInfo.pSetLayouts            = &Vulkan.descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = VK_NULL_HANDLE;

        if ( vkCreatePipelineLayout( Vulkan.device, &pipelineLayoutInfo, nullptr, Vulkan.pipelineLayout.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo 
	    graphicsPipelineInfo                     = {};
        graphicsPipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineInfo.pNext               = VK_NULL_HANDLE;
        graphicsPipelineInfo.stageCount          = 2;
        graphicsPipelineInfo.pStages             = shaderStages;
        graphicsPipelineInfo.pVertexInputState   = &vertexInputStateInfo;
        graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
        graphicsPipelineInfo.pViewportState      = &viewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &rasterizationStateInfo;
        graphicsPipelineInfo.pMultisampleState   = &multisampleStateInfo;
        graphicsPipelineInfo.pColorBlendState    = &colorBlendStateInfo;
        graphicsPipelineInfo.pDepthStencilState  = &depthStencilStateInfo;
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
            std::vector<VkImageView> attachments = { 
                Vulkan.swapchainImageViews[i],
                Vulkan.depthBuffer.imageView
            };

            VkFramebufferCreateInfo
            createInfo                 = {};
            createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.pNext           = VK_NULL_HANDLE;
            createInfo.renderPass      = Vulkan.renderPass;
            createInfo.attachmentCount = attachments.size();
            createInfo.pAttachments    = attachments.data();
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

    void CreateDepthResources()
	{
	    VkImageCreateInfo
	    imageInfo               = {};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = Vulkan.swapchainProperties.extent.width;
        imageInfo.extent.height = Vulkan.swapchainProperties.extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.format        = VK_FORMAT_D32_SFLOAT_S8_UINT;
        imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;

        if (vkCreateImage( Vulkan.device, &imageInfo, nullptr, &Vulkan.depthBuffer.image ) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements( Vulkan.device, Vulkan.depthBuffer.image, &memRequirements );

        VkfwUint32 memoryTypeIndex = FindMemoryTypeIndex( &Vulkan.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT );

        VkMemoryAllocateInfo 
	    allocInfo                 = {};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        allocInfo.allocationSize  = memRequirements.size;

        if (vkAllocateMemory( Vulkan.device, &allocInfo, nullptr, &Vulkan.depthBuffer.memory ) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate image memory");

        vkBindImageMemory( Vulkan.device, Vulkan.depthBuffer.image, Vulkan.depthBuffer.memory, 0 );

        VkImageViewCreateInfo
	    viewInfo                                 = {};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.format                          = imageInfo.format;
        viewInfo.image                           = Vulkan.depthBuffer.image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView( Vulkan.device, &viewInfo, nullptr, &Vulkan.depthBuffer.imageView ) != VK_SUCCESS)
            throw std::runtime_error("Failed to create imageview");
	}

    void CreateVertexBuffer()
	{
        VkDeviceSize bufferSize = model.Vertices.size() * sizeof(model.Vertices.at( 0 ));

        // local buffer
        {
            VkBufferCreateInfo
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            CreateAndAllocateBuffer(
                &bufferInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                Vulkan.vertexBuffer.Replace(),
                Vulkan.vertexBufferMemory.Replace() );
        }

        // staging buffer
        {
            VkBufferCreateInfo
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
            CreateAndAllocateBuffer(
                &bufferInfo,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                Vulkan.vertexStagingBuffer.Replace(),
                Vulkan.vertexStagingBufferMemory.Replace() );

            // copy the vertex data to VRAM
            void* pData;
            vkMapMemory( Vulkan.device, Vulkan.vertexStagingBufferMemory, 0, bufferInfo.size, 0, &pData );
            memcpy( pData, model.Vertices.data(), bufferInfo.size );

            // flush
            VkMappedMemoryRange 
	        range        = {};
            range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.pNext  = VK_NULL_HANDLE;
            range.size   = bufferInfo.size;
            range.memory = Vulkan.vertexStagingBufferMemory;
            range.offset = 0;

            if ( vkFlushMappedMemoryRanges( Vulkan.device, 1, &range ) != VK_SUCCESS )
                throw std::runtime_error("Failed to flush vertex buffer data");

            vkUnmapMemory( Vulkan.device, Vulkan.vertexStagingBufferMemory );

            CopyBuffer( &Vulkan.vertexStagingBuffer, &Vulkan.vertexBuffer, bufferInfo.size );
        }
	}

    void CreateIndexBuffer()
	{
        VkDeviceSize bufferSize = model.Indices.size() * sizeof model.Indices.at( 0 );

        // local buffer
        {
            VkBufferCreateInfo 
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            CreateAndAllocateBuffer( &bufferInfo, properties, Vulkan.indexBuffer.Replace(), Vulkan.indexBufferMemory.Replace() );
        }

        // staging buffer
        {
            VkBufferCreateInfo 
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            CreateAndAllocateBuffer( &bufferInfo, properties, Vulkan.indexStagingBuffer.Replace(), Vulkan.indexStagingBufferMemory.Replace() );

            void* pData;
            vkMapMemory( Vulkan.device, Vulkan.indexStagingBufferMemory, 0, bufferInfo.size, 0, &pData );
            memcpy( pData, model.Indices.data(), bufferInfo.size );

            VkMappedMemoryRange 
	        range        = {};
            range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.pNext  = VK_NULL_HANDLE;
            range.size   = bufferInfo.size;
            range.memory = Vulkan.indexStagingBufferMemory;
            range.offset = 0;

            if ( vkFlushMappedMemoryRanges( Vulkan.device, 1, &range ) != VK_SUCCESS )
                throw std::runtime_error("Failed to flush vertex buffer data");

            vkUnmapMemory( Vulkan.device, Vulkan.indexStagingBufferMemory );

            CopyBuffer( &Vulkan.indexStagingBuffer, &Vulkan.indexBuffer, bufferInfo.size );
        }
	}

    void CreateUniformBuffer()
	{
        VkDeviceSize bufferSize = sizeof (UniformBufferObject);

        // local buffer
        {
            VkBufferCreateInfo 
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.flags       = 0;

            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; 
            CreateAndAllocateBuffer( &bufferInfo, properties, Vulkan.uniformBuffer.Replace(), Vulkan.uniformBufferMemory.Replace() );
        }

        // staging buffer
        {
            VkBufferCreateInfo 
	        bufferInfo             = {};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.pNext       = VK_NULL_HANDLE;
            bufferInfo.size        = bufferSize;
            bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; 
            CreateAndAllocateBuffer( &bufferInfo, properties, Vulkan.uniformStagingBuffer.Replace(), Vulkan.uniformStagingBufferMemory.Replace() );
        }
	}

    void CreateDescriptorPool()
	{
        VkDescriptorPoolSize
	    poolSize                 = {};
        poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo
        poolInfo               = {};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext         = VK_NULL_HANDLE;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = &poolSize;
        poolInfo.maxSets       = 1;

        if ( vkCreateDescriptorPool( Vulkan.device, &poolInfo, nullptr, Vulkan.descriptorPool.Replace() ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create descriptor pool");
	}

    void CreateDescriptorSets()
	{
	    VkDescriptorSetAllocateInfo
	    allocInfo                    = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext              = VK_NULL_HANDLE;
        allocInfo.descriptorPool     = Vulkan.descriptorPool;
        allocInfo.pSetLayouts        = &Vulkan.descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if ( vkAllocateDescriptorSets( Vulkan.device, &allocInfo, &Vulkan.descriptorSet ) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate descriptor set");

        VkDescriptorBufferInfo
	    bufferInfo        = {};
        bufferInfo.buffer = Vulkan.uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof (UniformBufferObject);
        
        VkWriteDescriptorSet
        descriptorWrite                  = {};
        descriptorWrite.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding       = 0;
        descriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount  = 1;
        descriptorWrite.dstArrayElement  = 0;
        descriptorWrite.dstSet           = Vulkan.descriptorSet;
        descriptorWrite.pBufferInfo      = &bufferInfo;
        descriptorWrite.pImageInfo       = VK_NULL_HANDLE;
        descriptorWrite.pTexelBufferView = VK_NULL_HANDLE;

        vkUpdateDescriptorSets( Vulkan.device, 1, &descriptorWrite, 0, nullptr );
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
        std::vector<VkClearValue> clearValues = {
            { 0.0f },      // color
            { 1.0f, 0.0f } // depth
        };

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
            renderPassInfo.clearValueCount     = clearValues.size();
            renderPassInfo.pClearValues        = clearValues.data();
            
            vkBeginCommandBuffer( Vulkan.swapchainCommandBuffers[i], &beginInfo );
            vkCmdBeginRenderPass( Vulkan.swapchainCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

            VkDeviceSize offset = 0;

            vkCmdBindPipeline( Vulkan.swapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan.graphicsPipeline );
            vkCmdBindVertexBuffers( Vulkan.swapchainCommandBuffers[i], 0, 1, &Vulkan.vertexBuffer, &offset );
            vkCmdBindIndexBuffer( Vulkan.swapchainCommandBuffers[i], Vulkan.indexBuffer, 0, VK_INDEX_TYPE_UINT32 );
            vkCmdBindDescriptorSets( Vulkan.swapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan.pipelineLayout, 0, 1, &Vulkan.descriptorSet, 0, nullptr );
            vkCmdDrawIndexed( Vulkan.swapchainCommandBuffers[i], 3 * model.Indices.size(), 1, 0, 0, 0 );

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
    
    void CreateAndAllocateBuffer( const VkBufferCreateInfo* pCreateInfo, VkMemoryPropertyFlags memoryProperties, VkBuffer* pBuffer, VkDeviceMemory* pMemory ) const
	{
	    if ( vkCreateBuffer( Vulkan.device, pCreateInfo, nullptr, pBuffer ) != VK_SUCCESS )
            throw std::runtime_error("Failed to create buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements( Vulkan.device, *pBuffer, &memRequirements );

        VkfwInt32 memoryTypeIndex = FindMemoryTypeIndex( &Vulkan.physicalDevice, memRequirements.memoryTypeBits, memoryProperties );

        if (memoryTypeIndex == -1)
            throw std::runtime_error("Could not find a compatible memory type");

        VkMemoryAllocateInfo 
	    allocateInfo                 = {};
        allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext           = VK_NULL_HANDLE;
        allocateInfo.allocationSize  = memRequirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        if ( vkAllocateMemory( Vulkan.device, &allocateInfo, nullptr, pMemory ) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate buffer memory");

        vkBindBufferMemory( Vulkan.device, *pBuffer, *pMemory, 0 );
	}
    
    void CopyBuffer( const VkBuffer* src, const VkBuffer* dst, VkDeviceSize size ) const
	{
	    VkCommandBufferAllocateInfo
        allocInfo                    = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext              = VK_NULL_HANDLE;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool        = Vulkan.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer commandBuffer = {};
        vkAllocateCommandBuffers(Vulkan.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, *src, *dst, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(Vulkan.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(Vulkan.queues.graphics);

        vkFreeCommandBuffers(Vulkan.device, Vulkan.commandPool, 1, &commandBuffer);
	}

    //TODO void CreateImage(  );

    static VkfwUint32 FindMemoryTypeIndex( const VkPhysicalDevice* pPhysicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperties )
	{
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties( *pPhysicalDevice, &properties );

	    VkfwInt32 memoryTypeIndex = -1;
        for (VkfwUint32 i = 0; i < properties.memoryTypeCount && memoryTypeIndex == -1; i++)
        {
            if (TEST_FLAGS( memoryTypeBits, (1 << i) ) &&
                TEST_FLAGS( properties.memoryTypes[i].propertyFlags, memoryProperties ))
            {
                memoryTypeIndex = i;
            }
        }

        return (VkfwUint32)memoryTypeIndex;
	}

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
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

    vkfwDestroy();

	return EXIT_SUCCESS;
}
