#pragma once
#include <Windows.h>

/*
 * for hooking (and calling) functions directly from import address tables
 */

namespace IAT
{
	void** find_function(const char* function, HMODULE module);
	void* hook_function(const char* function, HMODULE module, void* detour);

	// hide casts in templates
	template<typename T>
	T* find_function(const char* function, HMODULE module)
	{
		return *static_cast<T**>(find_function(function, module));
	}

	template<typename T>
	T* hook_function(const char* function, HMODULE module, T* detour)
	{
		return static_cast<T*>(hook_function(function, module, static_cast<void*>(detour)));
	}
}
