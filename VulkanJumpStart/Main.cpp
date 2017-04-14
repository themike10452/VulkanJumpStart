#include "Main.h"

class VulkanApplication
{
public:
	void Run()
	{
		InitVulkan();
		MainLoop();
	}

private:
	VDeleter<VkDebugReportCallbackEXT> debugCallback = VK_NULL_HANDLE;

	void InitVulkan()
	{
		vkfwInit();
		this->CreateInstance();
		this->SetupDebugLogging();
	}

	void MainLoop()
	{
		Window window = Window();
		window.Create();
		window.Destroy();
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

		_loadInstanceLevelEntryPoints();
	}

	void SetupDebugLogging()
	{
		if (!Vulkan.enableValidationLayers)
			return;

		debugCallback = VDeleter<VkDebugReportCallbackEXT>{ Vulkan.instance, nullptr };

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = vkfwDebugCallback;
		createInfo.pUserData = nullptr;

		if (vkCreateDebugReportCallbackEXT(Vulkan.instance, &createInfo, nullptr, debugCallback.Replace()) != VK_SUCCESS)
			throw std::runtime_error("CreateDebugReportCallbackEXT failed");
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