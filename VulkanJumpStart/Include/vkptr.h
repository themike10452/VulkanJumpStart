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

	typedef void(*DestroyFunction)(T);

	VkPtr(const GlobalLevelDeleter pFunc)
	{
		_deleteFunc = [=](T obj) { (*pFunc)(obj, nullptr); };
	}

	VkPtr(const VkPtr<VkInstance>& instance, InstanceLevelDeleter pFunc)
	{
		_deleteFunc = [&instance, pFunc](T obj) { (*pFunc)(instance, obj, nullptr); };
	}

	VkPtr(const VkPtr<VkDevice>& device, DeviceLevelDeleter pFunc)
	{
		_deleteFunc = [&device, pFunc](T obj) { (*pFunc)(device, obj, nullptr); };
	}

	VkPtr(const DestroyFunction pFunc)
	{
		_deleteFunc = [pFunc](T obj) { (*pFunc)(obj); };
	}

	~VkPtr()
	{
		Cleanup();
	}

	const T* operator &() const
	{
		return &_object;
	}

	T* Replace()
	{
		Cleanup();
		return &_object;
	}

	operator T() const
	{
		return _object;
	}

	void operator =(T rhs)
	{
		if (rhs != _object)
		{
			Cleanup();
			_object = rhs;
		}
	}

	template<typename V>
	bool operator == (V rhs)
	{
		return _object == T(rhs);
	}

private:
	T _object{ VK_NULL_HANDLE };
	std::function<void(T)> _deleteFunc;

	void Cleanup()
	{
		if (_object != VK_NULL_HANDLE)
		{
			_deleteFunc(_object);
			_object = VK_NULL_HANDLE;
		}
	}
};

#endif // !VKPTR_HEADER