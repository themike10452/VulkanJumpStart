#ifndef WIN32_INPUT_HEADER
#define WIN32_INPUT_HEADER

short _vkfwGetKeyStateWin32( int key );
short _vkfwGetMouseButtonStateWin32( int key );

#define VKFW_KEY_0               0x30
#define VKFW_KEY_1               0x31
#define VKFW_KEY_2               0x32
#define VKFW_KEY_3               0x33
#define VKFW_KEY_4               0x34
#define VKFW_KEY_5               0x35
#define VKFW_KEY_6               0x36
#define VKFW_KEY_7               0x37
#define VKFW_KEY_8               0x38
#define VKFW_KEY_9               0x39

#define VKFW_KEY_A               0x41
#define VKFW_KEY_B               0x42
#define VKFW_KEY_C               0x43
#define VKFW_KEY_D               0x44
#define VKFW_KEY_E               0x45
#define VKFW_KEY_F               0x46
#define VKFW_KEY_G               0x47
#define VKFW_KEY_H               0x48
#define VKFW_KEY_I               0x49
#define VKFW_KEY_J               0x4A
#define VKFW_KEY_K               0x4B
#define VKFW_KEY_L               0x4C
#define VKFW_KEY_M               0x4D
#define VKFW_KEY_N               0x4E
#define VKFW_KEY_O               0x4F
#define VKFW_KEY_P               0x50
#define VKFW_KEY_Q               0x51
#define VKFW_KEY_R               0x52
#define VKFW_KEY_S               0x53
#define VKFW_KEY_T               0x54
#define VKFW_KEY_U               0x55
#define VKFW_KEY_V               0x56
#define VKFW_KEY_W               0x57
#define VKFW_KEY_X               0x58
#define VKFW_KEY_Y               0x59
#define VKFW_KEY_Z               0x5A

#define VKFW_KEY_EQUAL           VK_OEM_PLUS
#define VKFW_KEY_COMMA           VK_OEM_COMMA
#define VKFW_KEY_DASH            VK_OEM_MINUS
#define VKFW_KEY_PERIOD          VK_OEM_PERIOD
#define VKFW_KEY_SEMICOLON       VK_OEM_1
#define VKFW_KEY_SLASH           VK_OEM_2
#define VKFW_KEY_GRAVE_ACCENT    VK_OEM_3
#define VKFW_KEY_LEFT_BRACKET    VK_OEM_4
#define VKFW_KEY_BACKSLASH       VK_OEM_5
#define VKFW_KEY_RIGHT_BRACKET   VK_OEM_6
#define VKFW_KEY_QUOTE           VK_OEM_7

#define VKFW_MOUSE_BUTTON_LEFT   VK_LBUTTON
#define VKFW_MOUSE_BUTTON_RIGHT  VK_RBUTTON
#define VKFW_MOUSE_BUTTON_MIDDLE VK_MBUTTON
#define VKFW_MOUSE_XBUTTON1      VK_XBUTTON1
#define VKFW_MOUSE_XBUTTON2      VK_XBUTTON2

#define VKFW_KEY_BACKSPACE       VK_BACK
#define VKFW_KEY_TAB             VK_TAB
#define VKFW_KEY_CLEAR           VK_CLEAR
#define VKFW_KEY_RETURN          VK_RETURN
#define VKFW_KEY_SHIFT           VK_SHIFT
#define VKFW_KEY_CONTROL         VK_CONTROL
#define VKFW_KEY_ALT             VK_MENU
#define VKFW_KEY_PAUSE           VK_PAUSE
#define VKFW_KEY_CAPSLOCK        VK_CAPITAL
#define VKFW_KEY_ESCAPE          VK_ESCAPE
#define VKFW_KEY_SPACE           VK_SPACE
#define VKFW_KEY_PAGEUP          VK_PRIOR
#define VKFW_KEY_PAGEDOWN        VK_NEXT
#define VKFW_KEY_END             VK_END
#define VKFW_KEY_HOME            VK_HOME
#define VKFW_KEY_LEFT            VK_LEFT
#define VKFW_KEY_UP              VK_UP
#define VKFW_KEY_RIGHT           VK_RIGHT
#define VKFW_KEY_DOWN            VK_DOWN
#define VKFW_KEY_SNAPSHOT        VK_SNAPSHOT
#define VKFW_KEY_INSERT          VK_INSERT
#define VKFW_KEY_DELETE          VK_DELETE
#define VKFW_KEY_SUPER_LEFT      VK_LWIN
#define VKFW_KEY_SUPER_RIGHT     VK_RWIN
#define VKFW_KEY_APPS            VK_APPS
#define VKFW_KEY_NUMPAD0         VK_NUMPAD0
#define VKFW_KEY_NUMPAD1         VK_NUMPAD1
#define VKFW_KEY_NUMPAD2         VK_NUMPAD2
#define VKFW_KEY_NUMPAD3         VK_NUMPAD3
#define VKFW_KEY_NUMPAD4         VK_NUMPAD4
#define VKFW_KEY_NUMPAD5         VK_NUMPAD5
#define VKFW_KEY_NUMPAD6         VK_NUMPAD6
#define VKFW_KEY_NUMPAD7         VK_NUMPAD7
#define VKFW_KEY_NUMPAD8         VK_NUMPAD8
#define VKFW_KEY_NUMPAD9         VK_NUMPAD9
#define VKFW_KEY_MULTIPLY        VK_MULTIPLY
#define VKFW_KEY_SUBTRACT        VK_SUBTRACT
#define VKFW_KEY_DECIMAL         VK_DECIMAL
#define VKFW_KEY_DIVIDE          VK_DIVIDE

#define VKFW_KEY_F1              VK_F1
#define VKFW_KEY_F2              VK_F2
#define VKFW_KEY_F3              VK_F3
#define VKFW_KEY_F4              VK_F4
#define VKFW_KEY_F5              VK_F5
#define VKFW_KEY_F6              VK_F6
#define VKFW_KEY_F7              VK_F7
#define VKFW_KEY_F8              VK_F8
#define VKFW_KEY_F9              VK_F9
#define VKFW_KEY_F10             VK_F10
#define VKFW_KEY_F11             VK_F11
#define VKFW_KEY_F12             VK_F12
#define VKFW_KEY_F13             VK_F13
#define VKFW_KEY_F14             VK_F14
#define VKFW_KEY_F15             VK_F15
#define VKFW_KEY_F16             VK_F16
#define VKFW_KEY_F17             VK_F17
#define VKFW_KEY_F18             VK_F18
#define VKFW_KEY_F19             VK_F19
#define VKFW_KEY_F20             VK_F20
#define VKFW_KEY_F21             VK_F21
#define VKFW_KEY_F22             VK_F22
#define VKFW_KEY_F23             VK_F23
#define VKFW_KEY_F24             VK_F24

#define VKFW_KEY_NUMLOCK         VK_NUMLOCK
#define VKFW_KEY_SCROLL          VK_SCROLL

#define VKFW_KEY_SHIFT_LEFT      VK_LSHIFT
#define VKFW_KEY_SHIFT_RIGHT     VK_RSHIFT
#define VKFW_KEY_CONTROL_LEFT    VK_LCONTROL
#define VKFW_KEY_CONTROL_RIGHT   VK_RCONTROL
#define VKFW_KEY_ALT_LEFT        VK_LMENU
#define VKFW_KEY_ALT_RIGHT       VK_RMENU

#define VKFW_KEY_VOLUME_MUTE     VK_VOLUME_MUTE
#define VKFW_KEY_VOLUME_DOWN     VK_VOLUME_DOWN
#define VKFW_KEY_VOLUME_UP       VK_VOLUME_UP

#endif // !VKFW_WIN32_INPUT
