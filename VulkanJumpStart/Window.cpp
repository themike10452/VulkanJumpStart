#include "Window.h"
#include "resource.h"

#define MAX_LOADSTRING 100

WCHAR szClassName[MAX_LOADSTRING];
WCHAR szTitle[MAX_LOADSTRING];

HINSTANCE hInstance;
HWND hWnd;

HICON hIcon, hIconSm;
HCURSOR hCursor;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Window::Window()
{
	hInstance = GetModuleHandle(0);
}

Window::~Window()
{
	Destroy();
}

HRESULT Window::Create()
{
	hIcon = NULL;
	hIconSm = NULL;
	hCursor = LoadCursor(NULL, IDC_ARROW);

	LoadString(hInstance, IDS_APP_NAME, szClassName, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

	HRESULT err;

	err = FuncRegisterClass();
	if (err)
		return err;

	err = FuncCreateWindow();
	if (err)
		return err;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return S_OK;
}

HRESULT Window::FuncRegisterClass()
{
	WNDCLASSEXW wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEXW);
	wndClass.style = CS_DBLCLKS;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.cbClsExtra = NULL;
	wndClass.cbWndExtra = NULL;
	wndClass.hIcon = hIcon;
	wndClass.hIconSm = hIconSm;
	wndClass.hCursor = hCursor;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = szClassName;

	if (!RegisterClassExW(&wndClass))
	{
		HRESULT error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return error;
	}

	return S_OK;
}

HRESULT Window::FuncCreateWindow()
{
	hWnd = CreateWindowW(
		szClassName, szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		return GetLastError();
	}

	return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::Destroy()
{
	DestroyIcon(hIcon);
	DestroyIcon(hIconSm);
	DestroyCursor(hCursor);
	DestroyWindow(hWnd);

	UnregisterClass(szClassName, hInstance);
}