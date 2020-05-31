#include "pch.h"
#include "main.h"
#include "IAT.h"
#include "HookedDirect3D9.h"
#include <ctime> 

// Globals
DWORD hudHookAddressOffset = 0x3AD774;
DWORD mouseOffsetOffset = 0x352B91;
DWORD mouseWidthOffset = 0x352B53;

// original code we are replacing
BYTE hudHookOpcodes[] = { 0x8B, 0x11, 0x8D, 0x45, 0xC0 };                  // mov [ebp + 12], eax ; lea eax, [ebp - 64]
BYTE mouseOffsetOpcodes[] = { 0xC7, 0x45, 0xF8, 0x00, 0x00, 0xC8, 0xC3 };  // mov [ebp-08],C3C80000
BYTE mouseWidthOpcodes[] = { 0xC7, 0x45, 0x0C, 0x00, 0x00, 0x48, 0x44 };   // mov [ebp+0C],44480000

DWORD hudWidth = 0x44480000; // default 800.0f
DWORD mouseOffset = 0xC3C80000; // default -400.0f
DWORD jmpBackAddressHud;
DWORD jmpBackAddressMouseOffset;
DWORD jmpBackAddressMouseWidth;
std::ofstream ofile;


__declspec(dllexport) int main();
int main()
{
    return 0;
}

bool Hook(void* toHook, void* ourFunct, BYTE* opcodes, int len) {
    if (len < 5) {
        log("ERROR: toHook is less than 5 bytes");
        return false;
    }

    DWORD curProtection;
    if (!VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection)) {
        log("ERROR: VirtualProtect failed, error code: %i. aborting...", GetLastError());
        return false;
    }
    
    // just to make sure we dont't fuck anything up
    if (check_opcode(toHook, opcodes, len)) {
        memset(toHook, 0x90, len);
        DWORD relativeAddress = ((DWORD)ourFunct - (DWORD)toHook) - 5;
        *(BYTE*)toHook = 0xE9;
        *(DWORD*)((DWORD)toHook + 1) = relativeAddress;
        DWORD temp;
        if (!VirtualProtect(toHook, len, curProtection, &temp)) {
            log("ERROR: VirtualProtect failed, error code: %i. aborting...", GetLastError());
            return false;
        }

        return true;   
    }
    else {
        log("ERROR: Opcode missmatch. aborting...");
        return false;
    }
}

// check if the instructions we are replacing are actually what we are expecting them to be
bool check_opcode(void* toHook, BYTE* opcodes, int len) {
    for (int i = 0; i < len; i++) {
        if (*((BYTE*)toHook + i) != opcodes[i]) {
            log("Byte %i is 0x%x but expected to be 0x%x", i, *((BYTE*)toHook + i), opcodes[i]);
            return false;
        }
    }
    return true;
}

// Code we are injecting
void __declspec(naked) destretchHud() {
    __asm{
        push ebp
        mov	ebp, esp
        sub	esp, 4
        mov[ebp - 4], eax
        mov eax, hudWidth
        mov [ecx + 52], eax
        mov eax, [ebp - 4]
        mov	esp, ebp
        pop	ebp

        // original code
        mov edx, [ecx]
        lea eax, [ebp - 64]
        jmp [jmpBackAddressHud]
    }
}

void __declspec(naked) setMouseOffset() {
    __asm {
        push eax
        mov eax, mouseOffset
        mov [ebp - 8], eax
        pop	eax
        jmp[jmpBackAddressMouseOffset]

        // original code
        // mov [ebp-08],C3C80000
    }
}

void __declspec(naked) setMouseWidth() {
    __asm {
        push eax
        mov eax, hudWidth
        mov [ebp + 12], eax
        pop	eax
        jmp[jmpBackAddressMouseWidth]

        // original code
        // mov [ebp+0C],44480000
    }
}

// shamelessly stolen code for hooking IDirect3DDevice9 from BadSanta's RFX
// https://github.com/BadSanta12345/RFX

IDirect3D9* (WINAPI* pOrigDirect3DCreate9)(UINT) = nullptr;
IDirect3D9* WINAPI MyDirect3DCreate9(UINT sdk_version)
{
    log("Calling original Direct3DCreate9...");
    IDirect3D9* d3d = pOrigDirect3DCreate9(sdk_version);
    // Hook the whole Direct3D9 interface via a wrapper class
    return d3d ? new HookedDirect3D9(d3d) : 0;
}

auto pOrigLoadLibraryA = LoadLibraryA;
HMODULE WINAPI MyLoadLibraryA(LPCSTR lpLibFileName)
{
    auto hModule = pOrigLoadLibraryA(lpLibFileName);
    if (!pOrigDirect3DCreate9 && _stricmp(".\\RendDX9.dll", lpLibFileName) == 0)
    {
        log("Hooking Direct3DCreate9...");
        pOrigDirect3DCreate9 = IAT::hook_function("Direct3DCreate9", hModule, MyDirect3DCreate9);
    }
    return hModule;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    DWORD fh2Base = (DWORD)GetModuleHandleA(0);
    HMODULE hExe = GetModuleHandle(0);
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:

        // DisableThreadLibraryCalls(hModule);
        char dlldir[320];
        char path[320];

        // open file of logging
        GetModuleFileName(hModule, dlldir, 512);
        for (int i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }
        strcpy_s(path, dlldir);
        strcat_s(path, "hudfixlog.txt");
        ofile.open(path, std::ios::app);

        log("\n---------------------\nBF2 stretched HUD FIX v0.1\n---------------------");

        log("Hooking LoadLibraryA...");
        pOrigLoadLibraryA = IAT::hook_function("LoadLibraryA", hExe, MyLoadLibraryA);
        
        log("Destretching HUD...");
        DWORD hookAddress;
        hookAddress = fh2Base + hudHookAddressOffset;
        jmpBackAddressHud = hookAddress + sizeof(hudHookOpcodes);
        Hook((void*)hookAddress, destretchHud, hudHookOpcodes, sizeof(hudHookOpcodes));

        log("Extending mouse area...");
        hookAddress = fh2Base + mouseOffsetOffset;
        jmpBackAddressMouseWidth = hookAddress + sizeof(mouseOffsetOpcodes);
        Hook((void*)hookAddress, setMouseWidth, mouseOffsetOpcodes, sizeof(mouseOffsetOpcodes));
        
        log("Applying mouse area offset...");
        hookAddress = fh2Base + mouseWidthOffset;
        jmpBackAddressMouseOffset = hookAddress + sizeof(mouseWidthOpcodes);
        Hook((void*)hookAddress, setMouseOffset, mouseWidthOpcodes, sizeof(mouseWidthOpcodes));

        break;
    }

    return TRUE;

}

void __cdecl log(const char* fmt, ...)
{
    if (ofile.is_open())
    {
        if (!fmt) { return; }

        va_list va_alist;

        char timestamp_buf[256]{ 0 };
        time_t t = time(NULL);
        struct tm lt;
        localtime_s(&lt, &t);
        snprintf(timestamp_buf, 256, "%02d/%02d/%02d %02d:%02d:%02d", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min, lt.tm_sec);

        char logbuf[256] = { 0 };

        va_start(va_alist, fmt);
        _vsnprintf_s(logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), 256, fmt, va_alist);
        va_end(va_alist);

        ofile << "[" << timestamp_buf << "] " << logbuf << std::endl;
    }
}