#ifdef VK_USE_PLATFORM_XLIB_KHR

#include "vulkan/vulkan.h"
#include "x11_window.h"
#include "internal.h"
#include "vulkan_functions.h"
#include "helper_macros.h"

#include <stdexcept>

//TODO
Display* display;

void _vkfwCreateWindowX11( _VkfwWindow* window )
{
	REQUIRE_PTR( window );

	//Display* display;
	Visual* visual;
	int depth;
	XSetWindowAttributes windowAttributes;
	Window root;

	display = XOpenDisplay( nullptr );
	visual = DefaultVisual( display, 0 );
	depth = DefaultDepth( display, 0 );
	root = XRootWindow( display, 0 );

	windowAttributes.background_pixel = XBlackPixel( display, 0 );
	windowAttributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                                  ExposureMask | FocusChangeMask | VisibilityChangeMask |
                                  EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

	window->handle = XCreateWindow( display, root, 
									-1, -1, 
									window->config.width, window->config.height,
									5, depth, InputOutput,
									visual, CWBackPixel | CWEventMask, &windowAttributes );

	XStoreName( display, window->handle, window->config.title );
	XMapWindow( display, window->handle );

	XEvent event;

	while (1)
	{
		XNextEvent( display, &event );
		if (event.type == Expose)
			break;
	}

	//TODO
}

void _vkfwDestroyWindowX11( _VkfwWindow* window )
{
    if (window)
    {
        XDestroyWindow( display, window->handle );
    }
}

VkResult _vkfwCreateSurfaceKHRX11( VkInstance instance, const _VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface )
{
	VkXlibSurfaceCreateInfoKHR
    createInfo        = {};
	createInfo.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext  = nullptr;
	createInfo.flags  = 0;
	createInfo.window = pWindow->handle;
	return vkCreateXlibSurfaceKHR( instance, &createInfo, pCallbacks, pSurface );
}

void _vkfwPollEventsX11()
{
	// if (XPending( display ))
	// {
	// 	XEvent event;
	// 	XNextEvent( display, &event );
	// }
}

#endif // VK_USE_PLATFORM_XLIB_KHR
