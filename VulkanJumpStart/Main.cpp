#include "vulkan/vulkan.h"
#include "vkfw.h"
#include "main.h"
#include "vkptr.h"
#include "smartptr.h"

#include <iostream>
#include <vector>
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
		VkPtr<VkInstance>				instance{ &vkDestroyInstance };
		VkPtr<VkDebugReportCallbackEXT> debugCallback{ instance, &vkDestroyDebugReportCallbackEXT };
		VkPtr<VkDevice>					device{ &vkDestroyDevice };
		VkPtr<VkSurfaceKHR>				surface{ instance, &vkDestroySurfaceKHR };

		VkPhysicalDevice				physicalDevice;
		VkQueue							graphicsQueue;
		VkQueue							presentQueue;

		struct
		{
			VKFWint32					graphicsFamily	= -1;
			VKFWint32					presentFamily	= -1;
		} queueFamilyIndices;

	} Vulkan;

	SmartPtr<VKFWwindow*>				window{ vkfwDestroyWindow };

	void InitVulkan()
	{
		vkfwInit();
		this->CreateInstance();
		this->SetupDebugLogging();
		this->CreateWindowSurface();
		this->SelectPhysicalDevice();
		this->CreateLogicalDevice();
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

		VKFWuint32 requiredExtensionCount;
		const VKFWstring* requiredExtensionNames = vkfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		VKFWuint32 layerCount;
		const VKFWstring* requiredLayerNames = vkfwGetRequiredInstanceLayers(&layerCount);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.flags = 0;
		createInfo.enabledExtensionCount = requiredExtensionCount;
		createInfo.ppEnabledExtensionNames = requiredExtensionNames;
		createInfo.enabledLayerCount = layerCount;
		createInfo.ppEnabledLayerNames = requiredLayerNames;

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
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = fnDebugReportCallback;
		createInfo.pUserData = nullptr;

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
		VKFWuint32 deviceCount;
		std::vector<VkPhysicalDevice> physicalDevices;

		vkfwEnumeratePhysicalDevices(&Vulkan.instance, &physicalDevices);
		deviceCount = (VKFWuint32)physicalDevices.size();

		if (deviceCount < 1)
			throw std::runtime_error("Could not find a compatible GPU");

		VKFWuint32 bestDeviceIndex = DetectPrimaryPhysicalDevice(physicalDevices.data(), deviceCount);

		Vulkan.physicalDevice = physicalDevices[bestDeviceIndex];
	}

	void CreateLogicalDevice()
	{
		QueueFamilyIndices deviceQueueFamilyIndices = GetQueueFamilyIndices(Vulkan.physicalDevice);

		if (!deviceQueueFamilyIndices.complete())
			throw std::runtime_error("Could not find a compatible GPU");

		std::vector<VKFWuint32> queueFamilyIndices;
		queueFamilyIndices.push_back(deviceQueueFamilyIndices.graphics);
		queueFamilyIndices.push_back(deviceQueueFamilyIndices.present);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
		
		float queuePriority = 1.0f;
		for (const VKFWuint32& queueFamilyIndex : queueFamilyIndices)
		{
			VkDeviceQueueCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.queueFamilyIndex = queueFamilyIndex;
			createInfo.queueCount = 1;
			createInfo.pQueuePriorities = &queuePriority;

			queueCreateInfo.push_back(createInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceCreateInfo.queueCreateInfoCount = (VKFWuint32)queueCreateInfo.size();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.enabledLayerCount = 0;

		if (vkCreateDevice(Vulkan.physicalDevice, &deviceCreateInfo, nullptr, Vulkan.device.Replace()) != VK_SUCCESS)
			throw std::runtime_error("vkCreateDevice failed");

		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.graphics, 0, &Vulkan.graphicsQueue);
		vkGetDeviceQueue(Vulkan.device, deviceQueueFamilyIndices.present, 0, &Vulkan.presentQueue);

		if (!Vulkan.graphicsQueue || !Vulkan.presentQueue)
			throw std::runtime_error("vkGetDeviceQueue failed");

		vkfwLoadDeviceLevelEntryPoints(&Vulkan.device);
	}

#pragma region

	VKFWuint32 DetectPrimaryPhysicalDevice(const VkPhysicalDevice* physicalDevices, VKFWuint32 deviceCount)
	{
		VKFWuint32
			bestScore = 0,
			bestDeviceIndex = 0;

		for (VKFWuint32 i = 0; i < deviceCount; i++)
		{
			VkPhysicalDeviceProperties deviceProperties = {};
			vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures = {};
			vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

			QueueFamilyIndices queueFamilyIndices = GetQueueFamilyIndices((VkPhysicalDevice)physicalDevices[i]);

			VKFWuint32 score = 0;

			score += deviceProperties.limits.maxImageDimension2D;

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 10000;

			if (!deviceFeatures.geometryShader)
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

	QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice& physicalDevice)
	{
		QueueFamilyIndices queueFamilyIndices;

		VKFWuint32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		VKFWuint32 i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			VkBool32 presentSupported = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, Vulkan.surface, &presentSupported);

			if (presentSupported)
				queueFamilyIndices.present = i;

			if (queueFamily.queueCount > 0 &&
				queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
				queueFamilyIndices.graphics < 0)
			{
				queueFamilyIndices.graphics = i;
			}

			if (queueFamilyIndices.complete())
				break;

			i++;
		}

		return queueFamilyIndices;
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