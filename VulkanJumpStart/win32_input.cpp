#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "win32_input.h"
#include "types.h"

#include <windows.h>

short _vkfwGetKeyStateWin32( int key )
{
    short state = GetKeyState( key );

    short result = VKFW_RELEASED;
    
    if (state & 1)
        result = VKFW_TOGGLED;

    if (state & 0x800)
        result = VKFW_PRESSED;

    return result;
}

short _vkfwGetMouseButtonStateWin32( int button )
{
    short state = GetKeyState( button );

    short result = VKFW_RELEASED;

    if (state & 0x100)
        result = VKFW_PRESSED;

    return result;
}

#endif // VK_USE_PLATFORM_WIN32_KHR
