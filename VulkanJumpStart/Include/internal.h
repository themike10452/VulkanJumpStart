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
	VKFWbool initialized{ VKFW_FALSE };

	struct
	{
		LibHandle libHandle;
		std::vector<VKFWstring> requiredInstanceExtensions;
		std::vector<VKFWstring> requiredInstanceLayers;
	} vk;
};

struct _VKFWwindow
{
	WindowHandle handle;

	struct
	{
		VKFWbool	visible;
		VKFWbool	resizable;
		VKFWbool	fullscreen;
		VKFWuint32	width;
		VKFWuint32	height;
		VKFWstring	title;
	} windowConfig;
};

extern _VKFWlibrary _vkfw;

void		_vkfwLoadExportedEntryPoints();
void		_vkfwLoadGlobalLevelEntryPoints();
void		_vkfwLoadRequiredInstanceLayers();
void		_vkfwLoadRequiredInstanceExtensions();
VKFWbool	_vkfwCheckRequiredLayersAvailability();

#endif // !VKFW_INTERNAL_HEADER