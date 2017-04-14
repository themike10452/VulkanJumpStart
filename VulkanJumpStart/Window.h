#ifndef WINDOW_HEADER
#define WINDOW_HEADER

#ifndef HRESULT
#include <Windows.h>
#endif // !HRESULT

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

#endif // !WINDOW_HEADER