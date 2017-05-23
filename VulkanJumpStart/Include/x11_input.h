#ifndef X11_INPUT_HEADER
#define X11_INPUT_HEADER

#include <linux/input.h>

short _vkfwGetKeyStateX11( int key );
short _vkfwGetMouseButtonStateX11( int key );

#define VKFW_KEY_0               KEY_0
#define VKFW_KEY_1               KEY_1
#define VKFW_KEY_2               KEY_2
#define VKFW_KEY_3               KEY_3
#define VKFW_KEY_4               KEY_4
#define VKFW_KEY_5               KEY_5
#define VKFW_KEY_6               KEY_6
#define VKFW_KEY_7               KEY_7
#define VKFW_KEY_8               KEY_8
#define VKFW_KEY_9               KEY_9

#define VKFW_KEY_A               KEY_A
#define VKFW_KEY_B               KEY_B
#define VKFW_KEY_C               KEY_C
#define VKFW_KEY_D               KEY_D
#define VKFW_KEY_E               KEY_E
#define VKFW_KEY_F               KEY_F
#define VKFW_KEY_G               KEY_G
#define VKFW_KEY_H               KEY_H
#define VKFW_KEY_I               KEY_I
#define VKFW_KEY_J               KEY_J
#define VKFW_KEY_K               KEY_K
#define VKFW_KEY_L               KEY_L
#define VKFW_KEY_M               KEY_M
#define VKFW_KEY_N               KEY_N
#define VKFW_KEY_O               KEY_O
#define VKFW_KEY_P               KEY_P
#define VKFW_KEY_Q               KEY_Q
#define VKFW_KEY_R               KEY_R
#define VKFW_KEY_S               KEY_S
#define VKFW_KEY_T               KEY_T
#define VKFW_KEY_U               KEY_U
#define VKFW_KEY_V               KEY_V
#define VKFW_KEY_W               KEY_W
#define VKFW_KEY_X               KEY_X
#define VKFW_KEY_Y               KEY_Y
#define VKFW_KEY_Z               KEY_Z

#define VKFW_KEY_EQUAL           KEY_EQUAL
#define VKFW_KEY_COMMA           KEY_COMMA
#define VKFW_KEY_DASH            KEY_MINUS
#define VKFW_KEY_PERIOD          KEY_DOT
#define VKFW_KEY_SEMICOLON       KEY_SEMICOLON
#define VKFW_KEY_SLASH           KEY_SLASH
#define VKFW_KEY_GRAVE_ACCENT    KEY_GRAVE
#define VKFW_KEY_LEFT_BRACKET    KEY_LEFTBRACE
#define VKFW_KEY_BACKSLASH       KEY_BACKSLASH
#define VKFW_KEY_RIGHT_BRACKET   KEY_RIGHTBRACE
#define VKFW_KEY_QUOTE           KEY_APOSTROPHE

#define VKFW_MOUSE_BUTTON_LEFT   BTN_LEFT
#define VKFW_MOUSE_BUTTON_RIGHT  BTN_RIGHT
#define VKFW_MOUSE_BUTTON_MIDDLE BTN_MIDDLE
//#define VKFW_MOUSE_XBUTTON1      VK_XBUTTON1
//#define VKFW_MOUSE_XBUTTON2      VK_XBUTTON2

#define VKFW_KEY_BACKSPACE       KEY_BACKSPACE
#define VKFW_KEY_TAB             KEY_TAB
#define VKFW_KEY_RETURN          KEY_ENTER
#define VKFW_KEY_PAUSE           KEY_PAUSE
#define VKFW_KEY_CAPSLOCK        KEY_CAPSLOCK
#define VKFW_KEY_ESCAPE          KEY_EXIT
#define VKFW_KEY_SPACE           KEY_SPACE
#define VKFW_KEY_PAGEUP          KEY_PAGEUP
#define VKFW_KEY_PAGEDOWN        KEY_PAGEDOWN
#define VKFW_KEY_END             KEY_END
#define VKFW_KEY_HOME            KEY_HOME
#define VKFW_KEY_LEFT            KEY_LEFT
#define VKFW_KEY_UP              KEY_UP
#define VKFW_KEY_RIGHT           KEY_RIGHT
#define VKFW_KEY_DOWN            KEY_DOWN
//#define VKFW_KEY_SNAPSHOT        KEY_PRINTSCREEN
#define VKFW_KEY_INSERT          KEY_INSERT
#define VKFW_KEY_DELETE          KEY_DELETE
#define VKFW_KEY_SUPER_LEFT      KEY_LEFTMETA
#define VKFW_KEY_SUPER_RIGHT     KEY_RIGHTMETA
//#define VKFW_KEY_NUMPAD0         KEY_NUMPAD0
//#define VKFW_KEY_NUMPAD1         KEY_NUMPAD1
//#define VKFW_KEY_NUMPAD2         KEY_NUMPAD2
//#define VKFW_KEY_NUMPAD3         KEY_NUMPAD3
//#define VKFW_KEY_NUMPAD4         KEY_NUMPAD4
//#define VKFW_KEY_NUMPAD5         KEY_NUMPAD5
//#define VKFW_KEY_NUMPAD6         KEY_NUMPAD6
//#define VKFW_KEY_NUMPAD7         KEY_NUMPAD7
//#define VKFW_KEY_NUMPAD8         KEY_NUMPAD8
//#define VKFW_KEY_NUMPAD9         KEY_NUMPAD9
//#define VKFW_KEY_MULTIPLY        KEY_MULTIPLY
//#define VKFW_KEY_SUBTRACT        KEY_SUBTRACT
//#define VKFW_KEY_DECIMAL         KEY_DECIMAL
//#define VKFW_KEY_DIVIDE          KEY_DIVIDE

#define VKFW_KEY_F1              KEY_F1
#define VKFW_KEY_F2              KEY_F2
#define VKFW_KEY_F3              KEY_F3
#define VKFW_KEY_F4              KEY_F4
#define VKFW_KEY_F5              KEY_F5
#define VKFW_KEY_F6              KEY_F6
#define VKFW_KEY_F7              KEY_F7
#define VKFW_KEY_F8              KEY_F8
#define VKFW_KEY_F9              KEY_F9
#define VKFW_KEY_F10             KEY_F10
#define VKFW_KEY_F11             KEY_F11
#define VKFW_KEY_F12             KEY_F12
#define VKFW_KEY_F13             KEY_F13
#define VKFW_KEY_F14             KEY_F14
#define VKFW_KEY_F15             KEY_F15
#define VKFW_KEY_F16             KEY_F16
#define VKFW_KEY_F17             KEY_F17
#define VKFW_KEY_F18             KEY_F18
#define VKFW_KEY_F19             KEY_F19
#define VKFW_KEY_F20             KEY_F20
#define VKFW_KEY_F21             KEY_F21
#define VKFW_KEY_F22             KEY_F22
#define VKFW_KEY_F23             KEY_F23
#define VKFW_KEY_F24             KEY_F24

#define VKFW_KEY_NUMLOCK         KEY_NUMLOCK
#define VKFW_KEY_SCROLL          KEY_SCROLLLOCK

#define VKFW_KEY_SHIFT_LEFT      KEY_LEFTSHIFT
#define VKFW_KEY_SHIFT_RIGHT     KEY_RIGHTSHIFT
#define VKFW_KEY_CONTROL_LEFT    KEY_LEFTCTRL
#define VKFW_KEY_CONTROL_RIGHT   KEY_RIGHTCTRL
#define VKFW_KEY_ALT_LEFT        KEY_LEFTALT
#define VKFW_KEY_ALT_RIGHT       KEY_RIGHTALT

#define VKFW_KEY_VOLUME_MUTE     KEY_MUTE
#define VKFW_KEY_VOLUME_DOWN     KEY_VOLUMEDOWN
#define VKFW_KEY_VOLUME_UP       KEY_VOLUMEUP

#endif // !VKFW_WIN32_INPUT
