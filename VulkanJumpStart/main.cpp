#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "vkptr.h"
#include "smartptr.h"
#include "glm/common.hpp"

#include <iostream>
#include <vector>
#include <set>
#include <assert.h>

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
		VkQueue							graphicsQueue;
		VkQueue							presentQueue;

        struct
        {
            VkSurfaceCapabilitiesKHR capabilities;
            VkSurfaceFormatKHR format;
            VkPresentModeKHR presentMode;
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

		std::set<VkfwUint32> queueFamilyIndices = {};
		queueFamilyIndices.insert(deviceQueueFamilyIndices.graphics);
		queueFamilyIndices.insert(deviceQueueFamilyIndices.present);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
		
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

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkfwUint32 deviceExtensionCount;
		const VkfwString* deviceExtensions = vkfwGetRequiredDeviceExtensions(&deviceExtensionCount);

		VkDeviceCreateInfo deviceCreateInfo      = {};
		deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext                   = nullptr;
		deviceCreateInfo.pQueueCreateInfos       = queueCreateInfo.data();
		deviceCreateInfo.queueCreateInfoCount    = (VkfwUint32)queueCreateInfo.size();
		deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount   = deviceExtensionCount;
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
		deviceCreateInfo.enabledLayerCount       = 0;

		if (vkCreateDevice(Vulkan.physicalDevice, &deviceCreateInfo, nullptr, Vulkan.device.Replace()) != VK_SUCCESS)
			throw std::runtime_error("vkCreateDevice failed");

		vkfwLoadDeviceLevelEntryPoints(&Vulkan.device);

		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.graphics, 0, &Vulkan.graphicsQueue);
		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.present, 0, &Vulkan.presentQueue);

		if (!Vulkan.graphicsQueue || !Vulkan.presentQueue)
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

        VkExtent2D windowExtent = Vulkan.swapchainProperties.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()
	        ? Vulkan.swapchainProperties.capabilities.currentExtent
	        : VkExtent2D { 
                width  = glm::clamp(width, Vulkan.swapchainProperties.capabilities.minImageExtent.width, Vulkan.swapchainProperties.capabilities.maxImageExtent.width), 
                height = glm::clamp(width, Vulkan.swapchainProperties.capabilities.minImageExtent.height, Vulkan.swapchainProperties.capabilities.maxImageExtent.height)
            };

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                    = nullptr;
        createInfo.surface                  = Vulkan.surface;
        createInfo.imageExtent              = windowExtent;
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

        if (Vulkan.graphicsQueue != Vulkan.presentQueue)
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