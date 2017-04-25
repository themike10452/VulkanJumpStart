#include "vulkan/vulkan.h"
#include "types.h"

struct QueueFamilyIndices
{
	VKFWint32 graphics	= -1;
	VKFWint32 present	= -1;

	VKFWbool complete() { return graphics > -1 && present > -1; }
};