#ifndef VKFW_INTERNAL_HEADER
#define VKFW_INTERNAL_HEADER

#include "platform.h"
#include "types.h"
#include <vector>

#define _VKFW_ENABLE_VALIDATION_LAYERS _DEBUG

typedef struct _VKFWlibrary _VKFWlibrary;
typedef struct _VKFWwindow _VKFWwindow;

struct _VKFWlibrary
{
	VkfwBool initialized{ VKFW_FALSE };

	struct
	{
		LibHandle libHandle;
		std::vector<char*> requiredInstanceExtensions;
		std::vector<char*> requiredInstanceLayers;
	} vk;
};

struct _VKFWwindow
{
	WindowHandle handle;

	struct
	{
		VkfwBool	visible;
		VkfwBool	resizable;
		VkfwBool	fullscreen;
		VkfwUint32	width;
		VkfwUint32	height;
		VkfwString	title;
	} windowConfig;
};

extern _VKFWlibrary _vkfw;

void		_vkfwLoadExportedEntryPoints();
void		_vkfwLoadGlobalLevelEntryPoints();
void		_vkfwLoadRequiredInstanceLayers();
void		_vkfwLoadRequiredInstanceExtensions();
VkfwBool	_vkfwCheckRequiredLayersAvailability();

#endif // !VKFW_INTERNAL_HEADER