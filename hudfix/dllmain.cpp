#include "pch.h"
#include "main.h"
#include "IAT.h"
#include "HookedDirect3D9.h"
#include <ctime>
#include <string>


// Globals
DWORD g_hudWidth = 0x44480000; // default 800.0f
DWORD g_mouseOffset = 0xC3C80000; // default -400.0f
int g_hudOffset = 0;
Vec2 g_minimap_pos;

DWORD g_jmpBackAddressHud;
DWORD g_jmpBackAddressMouseOffset;
DWORD g_jmpBackAddressMouseWidth;
DWORD g_jmpBackAddressMinimapPos;
std::ofstream g_logfile;
char g_dlldir[320];

DWORD g_minimapPosHookAddress;


__declspec(dllexport) int main();
int main()
{
    return 0;
}

// to check if the instructions we are replacing are actually what we are expecting them to be
bool check_opcode(void* toHook, BYTE* opcodes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (*((BYTE*)toHook + i) != opcodes[i]) {
            log("Byte %i is 0x%x but expected to be 0x%x", i, *((BYTE*)toHook + i), opcodes[i]);
            return false;
        }
    }
    return true;
}

bool Hook(void* toHook, void* ourFunct, BYTE* opcodes, size_t len) {
    if (len < 5) {
        log("ERROR: toHook is less than 5 bytes");
        return false;
    }

    DWORD curProtection;
    if (!VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection)) {
        log("ERROR: VirtualProtect failed to override protection, error code: %i. aborting...", GetLastError());
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
            log("ERROR: VirtualProtect failed to restore protection, error code: %i. aborting...", GetLastError());
            return false;
        }

        return true;   
    }
    else {
        log("ERROR: Opcode missmatch. aborting...");
        return false;
    }
}

bool replaceMemory(void* dest, void* source, size_t len) {
    DWORD curProtection;
    if (!VirtualProtect(dest, len, PAGE_EXECUTE_READWRITE, &curProtection)) {
        log("ERROR: VirtualProtect failed to override protection, error code: %i. aborting...", GetLastError());
        return false;
    }

    memcpy(dest, source, len);

    DWORD temp;
    if (!VirtualProtect(dest, len, curProtection, &temp)) {
        log("ERROR: VirtualProtect failed to restore protection, error code: %i. aborting...", GetLastError());
        return false;
    }

    return true;
    }



void __declspec(naked) originalHudBuilderSetMinPos() {
    __asm {
        lea ecx, [ebp - 0x84]
        mov[edi + 0x24], esi
    }
}


void __cdecl updateMinimapPos(Vec2* minimap_pos) {
    log("hudBuilder.setMinPos called with %f/%f", minimap_pos->x, minimap_pos->y);
    if (minimap_pos->x >= 0)
        g_minimap_pos.x = minimap_pos->x + g_hudOffset; // offset to the right
    else
        g_minimap_pos.x = minimap_pos->x - g_hudOffset; // offset to the left
    
    g_minimap_pos.y = minimap_pos->y;

    append_hudinit("v_minimap_vec2", g_minimap_pos);
    log("unhooking hudBuilder.setMinPos...");
    replaceMemory((void*)g_minimapPosHookAddress, originalHudBuilderSetMinPos, 9);
}

// Code we are injecting
void __declspec(naked) updateMinimapPosHook() {
    __asm {
        push esi
        call updateMinimapPos
        add esp, 0x04

        // original code
        lea ecx,[ebp - 0x84]
        mov [edi + 0x24], esi
        jmp[g_jmpBackAddressMinimapPos]
    }
}

void __declspec(naked) destretchHud() {
    __asm{
        push ebp
        mov	ebp, esp
        sub	esp, 0x04
        mov[ebp - 0x04], eax
        mov eax, g_hudWidth
        mov [ecx + 0x34], eax
        mov eax, [ebp - 0x04]
        mov	esp, ebp
        pop	ebp

        // original code
        mov edx, [ecx]
        lea eax, [ebp - 0x40]
        jmp [g_jmpBackAddressHud]
    }
}

void __declspec(naked) setMouseOffset() {
    __asm {
        push eax
        mov eax, g_mouseOffset
        mov [ebp - 0x08], eax
        pop	eax
        jmp[g_jmpBackAddressMouseOffset]

        // original code
        // mov [ebp-08],C3C80000
    }
}

void __declspec(naked) setMouseWidth() {
    __asm {
        push eax
        mov eax, g_hudWidth
        mov [ebp + 0x0C], eax
        pop	eax
        jmp[g_jmpBackAddressMouseWidth]

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
    DWORD bf2Base = (DWORD)GetModuleHandleA(0);
    HMODULE hExe = GetModuleHandle(0);
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        // open file of logging
        GetModuleFileName(hModule, g_dlldir, 512);
        for (int i = strlen(g_dlldir); i > 0; i--) { if (g_dlldir[i] == '\\') { g_dlldir[i + 1] = 0; break; } }

        clear_hudinit();

        char log_path[320];
        strcpy_s(log_path, g_dlldir);
        strcat_s(log_path, "hudfixlog.txt");
        g_logfile.open(log_path, std::ios::app);
        log("\n---------------------\nBF2 stretched HUD FIX v0.2\n---------------------");


        log("Hooking LoadLibraryA...");
        pOrigLoadLibraryA = IAT::hook_function("LoadLibraryA", hExe, MyLoadLibraryA);

        DWORD hookAddress;

        log("Destretching HUD...");  // this will squish the entire HUD into 4:3 area in the screen center
        DWORD hudHookAddressOffset = 0x3AD774;
        BYTE hudHookOpcodes[] = { 0x8B, 0x11, 0x8D, 0x45, 0xC0 };                  // mov [ebp + 12], eax ; lea eax, [ebp - 64]
        hookAddress = bf2Base + hudHookAddressOffset;
        g_jmpBackAddressHud = hookAddress + sizeof(hudHookOpcodes);
        Hook((void*)hookAddress, destretchHud, hudHookOpcodes, sizeof(hudHookOpcodes));

        log("Extending mouse area...");  // after destreatching HUD mouse movement will be restricted, this will extend it to the correct width
        DWORD mouseOffsetOffset = 0x352B91;
        BYTE mouseOffsetOpcodes[] = { 0xC7, 0x45, 0xF8, 0x00, 0x00, 0xC8, 0xC3 };  // mov [ebp-08],C3C80000
        hookAddress = bf2Base + mouseOffsetOffset;
        g_jmpBackAddressMouseWidth = hookAddress + sizeof(mouseOffsetOpcodes);
        Hook((void*)hookAddress, setMouseWidth, mouseOffsetOpcodes, sizeof(mouseOffsetOpcodes));
        
        log("Applying mouse area offset...");  // this is for offsettig the extended mouse area to the left so it'll fit in the screen
        DWORD mouseWidthOffset = 0x352B53;
        BYTE mouseWidthOpcodes[] = { 0xC7, 0x45, 0x0C, 0x00, 0x00, 0x48, 0x44 };   // mov [ebp+0C],44480000
        hookAddress = bf2Base + mouseWidthOffset;
        g_jmpBackAddressMouseOffset = hookAddress + sizeof(mouseWidthOpcodes);
        Hook((void*)hookAddress, setMouseOffset, mouseWidthOpcodes, sizeof(mouseWidthOpcodes));

        log("Hooking hudBuilder.setMiniPos..."); // this will just read initial minimap position so we'll know what offset to apply later
        DWORD minimapPosOffset = 0x35F2B9;
        BYTE minimapPosOpcodes[] = { 0x8D, 0x8D, 0x7C, 0xFF, 0xFF, 0xFF, 0x89, 0x77, 0x24 }; // mov [edi + 36], esi; call BF2.exe+47F34C
        g_minimapPosHookAddress = bf2Base + minimapPosOffset;
        g_jmpBackAddressMinimapPos = g_minimapPosHookAddress + sizeof(minimapPosOpcodes);
        Hook((void*)g_minimapPosHookAddress, updateMinimapPosHook, minimapPosOpcodes, sizeof(minimapPosOpcodes));

        log("DONE. Fucking up BF2 finished");
        break;
    }

    return TRUE;
}

void __cdecl log(const char* fmt, ...)
{
    if (g_logfile.is_open())
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

        g_logfile << "[" << timestamp_buf << "] " << logbuf << std::endl;
    }
}

void update_offsets(float aspect_ratio) {
    float newWidth = 600 * aspect_ratio;
    float newMouseOffset = -newWidth / 2;
    g_hudWidth = *(reinterpret_cast<DWORD*>(&newWidth));
    g_mouseOffset = *(reinterpret_cast<DWORD*>(&newMouseOffset));
    g_hudOffset = (int)((600 * aspect_ratio - 800) / 2);

    append_hudinit("v_hud_r_offset", g_hudOffset);
    append_hudinit("v_hud_l_offset", -g_hudOffset);
}

void clear_hudinit() {
    log("Updating variables in HudInit.con");
    char path[320];
    strcpy_s(path, g_dlldir);
    strcat_s(path, "mods/bf2/HudInit.con");
    std::ofstream hudinit;
    hudinit.open(path, std::ios::trunc);
    hudinit.close();
}

void append_hudinit(const char* variable_name, Vec2 value) {
    std::string string_value = std::to_string(value.x) + "/" + std::to_string(value.y);
    append_hudinit(variable_name, string_value.c_str());
}

void append_hudinit(const char* variable_name, int value) {
    append_hudinit(variable_name, std::to_string(value).c_str());
}

void append_hudinit(const char* variable_name, const char* value) {
    log("writing '%s = %s' into HudInit.con", variable_name, value);
    char path[320];
    strcpy_s(path, g_dlldir);
    strcat_s(path, "mods/bf2/HudInit.con");
    std::ofstream hudinit;
    hudinit.open(path, std::ios::app);
    if (hudinit.is_open())
    {
        hudinit << variable_name << " = " << value << std::endl;
        hudinit.close();
    }
    else {
        log("ERROR: opening HudInit.con file");
    }
}