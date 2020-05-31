#pragma once
#include <Windows.h>
#include <fstream>

extern DWORD hudWidth;
extern DWORD mouseOffset;
void __cdecl log(const char* fmt, ...);
bool check_opcode(void* toHook, BYTE* opcodes, int len);