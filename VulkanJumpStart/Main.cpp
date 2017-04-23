#include "vkfw.h"
#include "vkptr.h"
#include "smartptr.h"

#include <iostream>
#include <assert.h>

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
		VkPtr<VkInstance> instance{ &vkDestroyInstance };
		VkPtr<VkDevice> device{ &vkDestroyDevice };
		VkPtr<VkSurfaceKHR> surface{ instance, &vkDestroySurfaceKHR };
		VkPtr<VkDebugReportCallbackEXT> debugCallback{ instance, &vkDestroyDebugReportCallbackEXT };

	} Vulkan;

	SmartPtr<VKFWwindow*> window{ vkfwDestroyWindow };

	void InitVulkan()
	{
		vkfwInit();
		this->CreateInstance();
		this->SetupDebugLogging();
		this->SelectDevice();
		this->CreateWindowSurface();
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

		uint32_t requiredExtensionCount;
		const char** requiredExtensionNames = vkfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		uint32_t layerCount;
		const char** requiredLayerNames = vkfwGetRequiredInstanceLayers(&layerCount);

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
#if _VKFW_ENABLE_VALIDATION_LAYERS
		/*VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = vkfwDebugCallback;
		createInfo.pUserData = nullptr;

		if (vkCreateDebugReportCallbackEXT(Vulkan.instance, &createInfo, nullptr, Vulkan.debugCallback.Replace()) != VK_SUCCESS)
			throw std::runtime_error("CreateDebugReportCallbackEXT failed");*/
#endif // VKFW_ENABLE_VALIDATION_LAYERS
	}

	void SelectDevice()
	{
		vkfwCreateDevice(&Vulkan.instance, Vulkan.device.Replace());
		vkfwLoadDeviceLevelEntryPoints(&Vulkan.device);
	}

	void CreateWindowSurface()
	{
		window = vkfwCreateWindow(800, 600, "Vulkan");

		if (vkfwCreateWindowSurface(Vulkan.instance, window, nullptr, Vulkan.surface.Replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create surface!");
		}
	}
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