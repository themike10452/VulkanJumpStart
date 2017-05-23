#ifdef VK_USE_PLATFORM_XLIB_KHR

#include "x11_input.h"
#include "types.h"

short _vkfwGetKeyStateX11( int key )
{
    return 0;
}

short _vkfwGetMouseButtonStateX11( int button )
{
    return 0;
}

#endif // VK_USE_PLATFORM_XLIB_KHR
