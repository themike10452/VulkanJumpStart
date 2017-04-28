#ifndef VKFW_INTERNAL_HEADER
#define VKFW_INTERNAL_HEADER

#include "platform.h"
#include "types.h"

#include <vector>
#include <assert.h>

#define _VKFW_ENABLE_VALIDATION_LAYERS _DEBUG

typedef struct _VkfwLibrary _VkfwLibrary;
typedef struct _VkfwWindow _VkfwWindow;

struct _VkfwLibrary
{
	VkfwBool						initialized{ VKFW_FALSE };

	struct
	{
		LibHandle					libHandle;
		std::vector<VkfwString>		requiredInstanceExtensions;
		std::vector<VkfwString>		requiredInstanceLayers;
		std::vector<VkfwString>		requiredDeviceExtensions;
	} vk;
};

struct _VkfwWindow
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

extern _VkfwLibrary _vkfw;

#define _VKFW_REQUIRE_INIT( )		\
	if (!_vkfw.initialized)			\
		throw std::runtime_error("vkfw not initialized. Call vkfwInit() first.");

#define _VKFW_REQUIRE_PTR( ptr )	\
	assert(ptr != nullptr);

void		_vkfwLoadExportedEntryPoints();
void		_vkfwLoadGlobalLevelEntryPoints();
void		_vkfwLoadRequiredInstanceLayers();
void		_vkfwLoadRequiredInstanceExtensions();
void		_vkfwLoadRequiredDeviceExtensions();
VkfwBool	_vkfwCheckRequiredLayersAvailability();

#endif // !VKFW_INTERNAL_HEADER