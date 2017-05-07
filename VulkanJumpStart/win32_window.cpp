#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "vulkan/vulkan.h"
#include "win32_window.h"
#include "internal.h"
#include "vulkan_functions.h"
#include "helper_macros.h"

#include <comdef.h>
#include <stdexcept>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void _vkfwCreateWindowWin32( _VkfwWindow* pWindow )
{
	REQUIRE_PTR( pWindow );

	_vkfwRegisterWindowClass();

	pWindow->handle = CreateWindow(
		_VKFW_WNDCLASSNAME, 
		pWindow->config.title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		pWindow->config.width,
		pWindow->config.height,
		NULL,
		NULL,
		GetModuleHandle(0),
		NULL
	);

	if (!pWindow->handle)
	{
		HRESULT error = GetLastError();
		_com_error err(error);
		throw std::runtime_error(err.ErrorMessage());
	}

	if ( (pWindow->state.visible = pWindow->config.visible) )
	{
		ShowWindow(pWindow->handle, SW_SHOW);
	}

    pWindow->state.shouldClose = VKFW_FALSE;
}

void _vkfwDestroyWindowWin32( _VkfwWindow* pWindow )
{
	if (pWindow)
	{
        DestroyWindow( pWindow->handle );
        UnregisterClass( _VKFW_WNDCLASSNAME, GetModuleHandle(0) );
	}
}

VkResult _vkfwCreateSurfaceKHRWin32( VkInstance instance, const _VkfwWindow* pWindow, VkAllocationCallbacks* pCallbacks, VkSurfaceKHR* pSurface )
{
	VkWin32SurfaceCreateInfoKHR
    createInfo           = {};
	createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext     = nullptr;
	createInfo.flags     = 0;
	createInfo.hinstance = GetModuleHandle(nullptr);
	createInfo.hwnd      = pWindow->handle;
	return vkCreateWin32SurfaceKHR(instance, &createInfo, pCallbacks, pSurface);
}

void _vkfwRegisterWindowClass()
{
	WNDCLASSEX wndClass;
	wndClass.cbSize        = sizeof(WNDCLASSEX);
	wndClass.style         = CS_DBLCLKS;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.cbClsExtra    = NULL;
	wndClass.cbWndExtra    = NULL;
	wndClass.hIcon         = NULL;
	wndClass.hIconSm       = NULL;
	wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wndClass.lpfnWndProc   = WndProc;
	wndClass.hInstance     = GetModuleHandle(0);
	wndClass.lpszMenuName  = NULL;
	wndClass.lpszClassName = _VKFW_WNDCLASSNAME;

	if (!RegisterClassEx(&wndClass))
	{
		HRESULT error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
		{
			_com_error err(error);
			throw std::runtime_error(err.ErrorMessage());
		}
	}
}

void _vkfwPollEventsWin32()
{
    MSG msg;

    if (PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg)
    {
        case WM_CLOSE:
            {
                const auto it = _vkfw.windowMap.find( hWnd );
                if ( it != _vkfw.windowMap.end() )
                    it->second->state.shouldClose = VKFW_TRUE;
                break;
            }
        case WM_DESTROY:
            {
                // do nothing
                break;
            }
        default:
            return DefWindowProc( hWnd, msg, wParam, lParam );
    }

    return 0;
}

#endif // VK_USE_PLATFORM_WIN32_KHR