#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER

#ifdef VKFW_ENABLE_VALIDATION
#define _VKFW_STANDARD_VALIDATION_LAYER		"VK_LAYER_LUNARG_standard_validation"
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>

typedef HMODULE		LibHandle;
typedef HINSTANCE	InstanceHandle;
typedef HWND		WindowHandle;

#define _VKFW_VULKAN_LIBRARY				"vulkan-1.dll"
#define _VKFW_WNDCLASSNAME					"VFKW10"
#define _VKFW_PLATFORM_SURFACE_EXTENSION	"VK_KHR_win32_surface"

#define LoadProcAddress( handle, symbol )	GetProcAddress( handle, symbol )
#define LoadDynamicLibrary( path )			LoadLibrary( path )
#define FreeDynamicLibrary( handle )		FreeLibrary( handle )

#include "win32_window.h"
#include "win32_input.h"

#define _vkfwPlatformCreateWindow		 _vkfwCreateWindowWin32
#define _vkfwPlatformDestroyWindow		 _vkfwDestroyWindowWin32
#define _vkfwPlatformCreateSurfaceKHR	 _vkfwCreateSurfaceKHRWin32
#define _vkfwPlatformPollEvents          _vkfwPollEventsWin32
#define _vkfwPlatformGetKeyState         _vkfwGetKeyStateWin32
#define _vkfwPlatformGetMouseButtonState _vkfwGetMouseButtonStateWin32

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

#include <X11/Xlib.h>
#include <dlfcn.h>

typedef void*	LibHandle;
typedef void*	InstanceHandle;
typedef Window	WindowHandle;

#define _VKFW_VULKAN_LIBRARY				"libvulkan.so"
#define _VKFW_PLATFORM_SURFACE_EXTENSION	"VK_KHR_xlib_surface"

#define LoadProcAddress( handle, symbol )	dlsym( handle, symbol )
#define LoadDynamicLibrary( path )			dlopen( path, RTLD_NOW )
#define FreeDynamicLibrary( handle )		dlclose( handle )

#include "x11_window.h"
#include "x11_input.h"

#define _vkfwPlatformCreateWindow		 _vkfwCreateWindowX11
#define _vkfwPlatformDestroyWindow		 _vkfwDestroyWindowX11
#define _vkfwPlatformCreateSurfaceKHR	 _vkfwCreateSurfaceKHRX11
#define _vkfwPlatformPollEvents          _vkfwPollEventsX11
#define _vkfwPlatformGetKeyState         _vkfwGetKeyStateX11
#define _vkfwPlatformGetMouseButtonState _vkfwGetMouseButtonStateX11

#endif

#endif // !PLATFORM_HEADER
