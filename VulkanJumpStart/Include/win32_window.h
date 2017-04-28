#ifndef WIN32_WINDOW_HEADER
#define WIN32_WINDOW_HEADER

typedef struct _VkfwWindow _VkfwWindow;

void		_vkfwCreateWindowWin32(_VkfwWindow* pWindow);
void		_vkfwDestroyWindowWin32(_VkfwWindow* pWindow);
VkResult	_vkfwCreateSurfaceKHRWin32(VkInstance instance, const _VkfwWindow* pWindow, VkAllocationCallbacks* allocationCallbacks, VkSurfaceKHR* surface);
void		_vkfwRegisterWindowClass();

#endif // !WIN32_WINDOW_HEADER