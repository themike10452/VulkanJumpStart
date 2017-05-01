#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>

typedef HINSTANCE	LibHandle;
typedef HINSTANCE	InstanceHandle;
typedef HWND		WindowHandle;

#define _VKFW_WNDCLASSNAME					"VFKW10"
#define _VKFW_PLATFORM_SURFACE_EXTENSION	"VK_KHR_win32_surface"

#ifdef VKFW_ENABLE_VALIDATION
#define _VKFW_STANDARD_VALIDATION_LAYER		"VK_LAYER_LUNARG_standard_validation"
#endif

#define LoadProcAddress			GetProcAddress
#define LoadDynamicLibrary		LoadLibrary
#define FreeDynamicLibrary		FreeLibrary

#include "win32_window.h"

#define _vkfwPlatformCreateWindow		_vkfwCreateWindowWin32
#define _vkfwPlatformDestroyWindow		_vkfwDestroyWindowWin32
#define _vkfwPlatformCreateSurfaceKHR	_vkfwCreateSurfaceKHRWin32
#define _vkfwPlatformPollEvents         _vkfwPollEventsWin32

#endif // VK_USE_PLATFORM_WIN32_KHR

#endif // !PLATFORM_HEADER