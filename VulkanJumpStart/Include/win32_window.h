#ifndef WIN32_WINDOW_HEADER
#define WIN32_WINDOW_HEADER

#ifdef VK_USE_PLATFORM_WIN32_KHR

typedef struct _VkfwWindow _VkfwWindow;

void		_vkfwCreateWindowWin32( _VkfwWindow* pWindow );
void		_vkfwDestroyWindowWin32( _VkfwWindow* pWindow );
VkResult	_vkfwCreateSurfaceKHRWin32( VkInstance instance, const _VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface );
void		_vkfwRegisterWindowClass();
void        _vkfwPollEventsWin32();

#endif // VK_USE_PLATFORM_WIN32_KHR

#endif // !WIN32_WINDOW_HEADER