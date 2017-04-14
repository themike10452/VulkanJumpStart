#ifndef VKPTR_HEADER
#define VKPTR_HEADER

#include <iostream>
#include <functional>

#ifndef VKAPI_ATTR
#include "vulkan.h"
#endif // !VKAPI_PTR

template<typename T>
class VkPtr
{

public:
	typedef void(VKAPI_CALL **GlobalLevelDeleter)(T, const VkAllocationCallbacks*);
	typedef void(VKAPI_CALL **InstanceLevelDeleter)(VkInstance, T, const VkAllocationCallbacks*);
	typedef void(VKAPI_CALL **DeviceLevelDeleter)(VkDevice, T, const VkAllocationCallbacks*);

	VkPtr(const GlobalLevelDeleter pFunc)
	{
		DeleteFunc = [=](T obj) { (*pFunc)(obj, nullptr); };
	}

	VkPtr(const VkPtr<VkInstance> &instance, InstanceLevelDeleter pFunc)
	{
		DeleteFunc = [&instance, pFunc](T obj) { (*pFunc)(instance, obj, nullptr); };
	}

	VkPtr(const VkPtr<VkDevice> &device, DeviceLevelDeleter pFunc)
	{
		DeleteFunc = [&device, pFunc](T obj) { (*pFunc)(device, obj, nullptr); };
	}

	~VkPtr()
	{
		Cleanup();
	}

	const T* operator &() const
	{
		return &object;
	}

	T* Replace()
	{
		Cleanup();
		return &object;
	}

	operator T() const
	{
		return object;
	}

	void operator =(T rhs) const
	{
		if (rhs != object)
		{
			Cleanup();
			object = rhs;
		}
	}

	template<typename V>
	bool operator == (V rhs)
	{
		return object == T(rhs)
	}

private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> DeleteFunc;

	void Cleanup()
	{
		if (object != VK_NULL_HANDLE)
		{
			DeleteFunc(object);
			object = VK_NULL_HANDLE;
		}
	}
};

#endif // !VKPTR_HEADER