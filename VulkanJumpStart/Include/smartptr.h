#ifndef SMARTPTR_HEADER
#define SMARTPTR_HEADER

#include <iostream>
#include <functional>

template<typename T>
class SmartPtr
{

public:
	typedef void(*DestroyFunction)(T);

	SmartPtr(const DestroyFunction pFunc)
	{
		DeleteFunc = [pFunc](T obj) { (*pFunc)(obj); };

		if (std::is_pointer<T>::value)
			object = nullptr;
	}

	~SmartPtr()
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

	void operator =(T rhs)
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
		return object == T(rhs);
	}

private:
	T object;
	std::function<void(T)> DeleteFunc;

	void Cleanup()
	{
		if (object != nullptr)
		{
			DeleteFunc(object);
		}
	}
};

#endif // !SMARTPTR_HEADER