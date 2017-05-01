#include "vulkan/vulkan.h"
#include "win32_window.h"
#include "internal.h"
#include "vulkan_functions.h"

#include <comdef.h>
#include <stdexcept>
#include <iostream>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void _vkfwCreateWindowWin32(_VkfwWindow* window)
{
	_vkfwRegisterWindowClass();

	window->handle = CreateWindow(
		_VKFW_WNDCLASSNAME, 
		window->windowConfig.title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window->windowConfig.width,
		window->windowConfig.height,
		NULL,
		NULL,
		GetModuleHandle(0),
		NULL
	);

	if (!window->handle)
	{
		HRESULT error = GetLastError();
		_com_error err(error);
		throw std::runtime_error(err.ErrorMessage());
	}

	if (window->windowConfig.visible)
	{
		ShowWindow(window->handle, SW_SHOW);
	}
}

void _vkfwDestroyWindowWin32(_VkfwWindow* window)
{
	if (window != nullptr)
	{
        delete window->windowConfig.title;
        delete window;

        window->windowConfig.title = nullptr;
		window = nullptr;

		UnregisterClass(_VKFW_WNDCLASSNAME, GetModuleHandle(0));
	}
}

VkResult _vkfwCreateSurfaceKHRWin32(VkInstance instance, const _VkfwWindow * pWindow, VkAllocationCallbacks * allocationCallbacks, VkSurfaceKHR * surface)
{
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.hinstance = GetModuleHandle(nullptr);
	createInfo.hwnd = pWindow->handle;
	return vkCreateWin32SurfaceKHR(instance, &createInfo, allocationCallbacks, surface);
}

void _vkfwRegisterWindowClass()
{
	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_DBLCLKS;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.cbClsExtra = NULL;
	wndClass.cbWndExtra = NULL;
	wndClass.hIcon = NULL;
	wndClass.hIconSm = NULL;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = GetModuleHandle(0);
	wndClass.lpszMenuName = NULL;
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}