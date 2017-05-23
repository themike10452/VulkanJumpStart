#ifndef X11_WINDOW_HEADER
#define X11_WINDOW_HEADER

#ifdef VK_USE_PLATFORM_XLIB_KHR

typedef struct _VkfwWindow _VkfwWindow;

void		_vkfwCreateWindowX11( _VkfwWindow* pWindow );
void		_vkfwDestroyWindowX11( _VkfwWindow* pWindow );
VkResult	_vkfwCreateSurfaceKHRX11( VkInstance instance, const _VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface );
void        _vkfwPollEventsX11();

#endif // VK_USE_PLATFORM_XLIB_KHR

#endif // !X11_WINDOW_HEADER
