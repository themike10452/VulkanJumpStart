#pragma once

namespace OS
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
#define LoadProcAddress GetProcAddress
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
	typedef HMODULE LibraryHandle;
#endif // VK_USE_PLATFORM_WIN32_KHR
	LibraryHandle VulkanLibrary;
}