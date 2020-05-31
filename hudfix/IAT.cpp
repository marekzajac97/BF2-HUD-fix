#include "pch.h"
#include "IAT.h"

namespace IAT
{
	void** find_function(const char* function, HMODULE module)
	{
		int ip = 0;
		if (module == 0)
			module = GetModuleHandle(0);
		PIMAGE_DOS_HEADER pImgDosHeaders = (PIMAGE_DOS_HEADER)module;
		PIMAGE_NT_HEADERS pImgNTHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)pImgDosHeaders + pImgDosHeaders->e_lfanew);
		PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImgDosHeaders + pImgNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		int size = (int)((LPBYTE)pImgDosHeaders + pImgNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);

		if (pImgDosHeaders->e_magic != IMAGE_DOS_SIGNATURE)
		{
			//invalid DOS signature
		}

		for (IMAGE_IMPORT_DESCRIPTOR* iid = pImgImportDesc; iid->Name != NULL; iid++)
		{
			for (int funcIdx = 0; *(funcIdx + (LPVOID*)(iid->FirstThunk + (SIZE_T)module)) != NULL; funcIdx++)
			{
				char* modFuncName = (char*)(*(funcIdx + (SIZE_T*)(iid->OriginalFirstThunk + (SIZE_T)module)) + (SIZE_T)module + 2);
				// Added the IsBadReadPtr because 'anonymous' functions would crash
				// not pretty but it works
				if ((!IsBadReadPtr(modFuncName, 1)) && !_stricmp(function, modFuncName))
					return funcIdx + (LPVOID*)(iid->FirstThunk + (SIZE_T)module);
			}
		}
		return 0;
	}

	void* hook_function(const char* function, HMODULE module, void* detour)
	{
		void** pFunc = find_function(function, module);
		DWORD oldrights;
		VirtualProtect(pFunc, sizeof(LPVOID), PAGE_READWRITE, &oldrights);
		void* retval = *pFunc;
		*pFunc = detour;
		VirtualProtect(pFunc, sizeof(LPVOID), oldrights, nullptr);
		return retval;
	}
}