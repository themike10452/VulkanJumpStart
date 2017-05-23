#ifndef TYPES_HEADER
#define TYPES_HEADER

#define VKFW_TRUE  1
#define VKFW_FALSE 0

#define VKFW_RELEASED   0x00
#define VKFW_PRESSED    0x01
#define VKFW_TOGGLED    0x02

typedef bool               VkfwBool;
typedef unsigned int       VkfwUint32;
typedef unsigned long      VkfwUint64;
typedef unsigned long long VkfwSize;
typedef int                VkfwInt32;
typedef char*              VkfwString;
typedef char               VkfwChar;

#endif // !TYPES_HEADER
