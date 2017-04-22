#ifndef OS_HEADER
#define OS_HEADER

#ifdef VK_USE_PLATFORM_WIN32_KHR
#define LoadProcAddress GetProcAddress
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
typedef HMODULE LibraryHandle;
typedef HWND WindowHandle;
typedef HINSTANCE InstanceHandle;
#endif // VK_USE_PLATFORM_WIN32_KHR

namespace OS
{
	void CreateWindowX(WindowHandle* wndHandle)
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // WIN32
	}

	void DestroyWindowX()
	{

	}
}

#endif // !OS_HEADER