#ifndef WIN32_WINDOW_HEADER
#define WIN32_WINDOW_HEADER

#include "vulkan.h"

typedef struct _VKFWwindow _VKFWwindow;

void		_vkfwCreateWindowWin32(_VKFWwindow* pWindow);
void		_vkfwDestroyWindowWin32(_VKFWwindow* pWindow);
VkResult	_vkfwCreateSurfaceKHRWin32(VkInstance instance, const _VKFWwindow* pWindow, VkAllocationCallbacks* allocationCallbacks, VkSurfaceKHR* surface);
void		_vkfwRegisterWindowClass();

#endif // !WIN32_WINDOW_HEADER