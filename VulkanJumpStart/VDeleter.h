#pragma once

#include <iostream>
#include <functional>
#include <vulkan\vulkan.h>

template<typename T>
class VDeleter
{
public:
	VDeleter() : VDeleter([](T, VkAllocationCallbacks*) {}) {}

	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deleteFunc)
	{
		this->DeleteFunc = [=](T obj) { deleteFunc(obj, nullptr); };
	}

	VDeleter(const VDeleter<VkInstance> &instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleteFunc)
	{
		this->DeleteFunc = [&instance, deleteFunc](T obj) { deleteFunc(instance, obj, nullptr); };
	}

	VDeleter(const VDeleter<VkDevice> &device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleteFunc)
	{
		this->DeleteFunc = [&device, deleteFunc](T obj) { deleteFunc(device, obj, nullptr); };
	}

	~VDeleter()
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
			try {
				DeleteFunc(object);
			}
			catch (std::exception e)
			{
				std::cout << e.what() << std::endl;
			}
			object = VK_NULL_HANDLE;
		}
	}
};