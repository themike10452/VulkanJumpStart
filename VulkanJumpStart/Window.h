#pragma once

#include <Windows.h>

class Window
{
public:
	Window();
	~Window();

	HRESULT Create();
	void Destroy();

private:
	HRESULT FuncRegisterClass();
	HRESULT FuncCreateWindow();
};