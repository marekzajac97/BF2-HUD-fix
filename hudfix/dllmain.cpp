#include "pch.h"
#include "main.h"
#include "HudNodes.h"
#include "CRenderer.h"
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <set>
#include <fstream>

#define CONFIG_PATH "hud_config.txt"

// Globals
const DWORD g_hudManagerRefreshHookEntry = 0x75C9D6;
const DWORD g_hudManagerRefreshHookExit = 0x75C9D6 + 5;

std::set<std::string> g_nodesRight;
std::set<std::string> g_nodesLeft;
std::set<std::string> g_nodesFillRight;
std::set<std::string> g_nodesFillLeft;
bool g_offsetMinimap = true;
std::ofstream g_logfile;

__declspec(dllexport) int main();
int main()
{
    return 0;
}

bool hook(void* toHook, void* ourFunct, size_t len) {
    if (len < 5) {
        log("ERROR: toHook is less than 5 bytes");
        return false;
    }

    DWORD curProtection;
    if (!VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection)) {
        log("ERROR: VirtualProtect failed to override protection, error code: %i. aborting...", GetLastError());
        return false;
    }

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

void onHudLoaded() {
    CRenderer** renderer = (CRenderer**)0x9A557C;
    if (renderer == NULL && *renderer == NULL) {
        log("ERROR: CRenderer object not found");
        return;
    }
    float aspect_ratio = (float)(*renderer)->screenWidth / (float)(*renderer)->screenHeight;
    float newWidth = 600 * aspect_ratio;
    float newMouseOffset = -newWidth / 2;
    DWORD hudWidth = *(reinterpret_cast<DWORD*>(&newWidth));
    DWORD mouseOffset = *(reinterpret_cast<DWORD*>(&newMouseOffset));
    float hudOffset = (int)((600 * aspect_ratio - 800) / 2);

    // this will squish the entire HUD into 4:3 area in the screen center
    log("Changing HUD width to %f (0x%x)", newWidth, hudWidth);
    replaceMemory((void*)0x7AD489, (void*)&hudWidth, 4);
    // after destreatching HUD mouse movement will be restricted, this will extend it to the correct width
    log("Extending mouse area to %f (0x%x)", newWidth, hudWidth);
    replaceMemory((void*)0x752B56, (void*)&hudWidth, 4);
    // this is for offsettig the extended mouse area to the left so it'll fit in the screen
    log("Applying mouse area offset to %f (%x)", newMouseOffset, mouseOffset);
    replaceMemory((void*)0x752B94, (void*)&mouseOffset, 4);
    // fixes bug with corssiar going nuts when proning/standing up
    BYTE tmp1 = 0xEB;
    replaceMemory((void*)0x7A5763, &tmp1, 1);
    // fixes AA markers being in wrong position
    // BYTE tmp2[] = { 0xD9, 0x05, 0xF4, 0xF9, 0x4B, 0x00, 0x90, 0x90, 0x90 };
    // replaceMemory((void*)0x7CCDD3, tmp2, 9);

    meme::HudNode* tmp;
    log("Applying node offsets");

    // Anchor nodes left
    for (std::set<std::string>::iterator it = g_nodesLeft.begin(); it != g_nodesLeft.end(); ++it) {
        if ((tmp = meme::findNode((*it).c_str())) != nullptr)
            tmp->setNodePos(tmp->getNodePosX() - hudOffset, tmp->getNodePosY());
        else
            log("ERROR: %s Not Found!", (*it).c_str());
    }

    // Anchor nodes right
    for (std::set<std::string>::iterator it = g_nodesRight.begin(); it != g_nodesRight.end(); ++it) {
        if ((tmp = meme::findNode((*it).c_str())) != nullptr)
            tmp->setNodePos(tmp->getNodePosX() + hudOffset, tmp->getNodePosY());
        else
            log("ERROR: %s Not Found!", (*it).c_str());
    }

    // Create right filler for fullscreen overlays
    for (std::set<std::string>::iterator it = g_nodesFillRight.begin(); it != g_nodesFillRight.end(); ++it) {
        if ((tmp = meme::findNode((*it).c_str())) != nullptr)
            tmp->setNodeSize(hudOffset, tmp->getNodeSizeY());
        else
            log("ERROR: %s Not Found!", (*it).c_str());
    }

    // Create left filler for fullscreen overlays
    for (std::set<std::string>::iterator it = g_nodesFillLeft.begin(); it != g_nodesFillLeft.end(); ++it) {
        if ((tmp = meme::findNode((*it).c_str())) != nullptr) {
            tmp->setNodeSize(hudOffset, tmp->getNodeSizeY());
            tmp->setNodePos(tmp->getNodePosX() - hudOffset, tmp->getNodePosY());
        }
        else
            log("ERROR: %s Not Found!", (*it).c_str());
    }

    if (g_offsetMinimap) {
        tmp = meme::findNode("Minimap");
        if (tmp != nullptr) {
            meme::MapNode* mapnode = (meme::MapNode*)tmp->getNodePtr();
            mapnode->resetX += hudOffset;
        }
        else
            log("ERROR: Minimap Not Found!");
    }
}

void __declspec(naked) toHudManagerRefreshHook() {
    __asm {
        pushfd
        call onHudLoaded
        popfd
        // original code
        mov[esi], al
        mov eax, esi
        pop esi
        jmp[g_hudManagerRefreshHookExit]
    }
}


BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    HMODULE hExe = GetModuleHandle(0);
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        char* dlldir = new char[MAX_PATH];
        GetModuleFileName(hModule, dlldir, MAX_PATH);
        for (int i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }

        bool debug = false;

        char configPath[MAX_PATH];
        strcpy_s(configPath, dlldir);
        strcat_s(configPath, CONFIG_PATH);
        std::ifstream config;
        config.open(configPath, std::ifstream::in);
        if (config.is_open())
        {
            std::string line;
            while (getline(config, line))
            {
                line = line.substr(0, line.find("#")); // comment
                std::vector<std::string> splited;
                std::istringstream iss(line);
                for (std::string line; iss >> line; )
                    splited.push_back(line);

                log("%s", line.c_str());
                if (splited.size() != 0) {
                    if (splited.at(0) == "debug" && splited.size() == 2) {
                        debug = (bool)std::stoi(splited.at(1));
                    }
                    else if (splited.at(0) == "offset_right" && splited.size() == 2) {
                        g_nodesRight.insert(splited.at(1));
                    }
                    else if (splited.at(0) == "offset_left" && splited.size() == 2) {
                        g_nodesLeft.insert(splited.at(1));
                    }
                    else if (splited.at(0) == "fill_left" && splited.size() == 2) {
                        g_nodesFillLeft.insert(splited.at(1));
                    }
                    else if (splited.at(0) == "fill_right" && splited.size() == 2) {
                        g_nodesFillRight.insert(splited.at(1));
                    }
                    else if (splited.at(0) == "offset_minimap" && splited.size() == 2) {
                        g_offsetMinimap = (bool)std::stoi(splited.at(1));
                    }
                }
            }
            config.close();
        }

        if (debug) {
            char log_path[MAX_PATH];
            strcpy_s(log_path, dlldir);
            strcat_s(log_path, "hudfixlog.txt");
            g_logfile.open(log_path, std::ios::app);
            log("\n---------------------\nBF2 stretched HUD FIX v0.4\n---------------------");
        }

        // hudManager.refresh needs to be added at the end of the HudSetupMain.con
        log("Hooking hudManager.refresh");
        hook((void*)g_hudManagerRefreshHookEntry, toHudManagerRefreshHook, g_hudManagerRefreshHookExit - g_hudManagerRefreshHookEntry);

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

        auto now = std::chrono::system_clock::now();
        auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
        auto rest = now - sec;
        auto mili = std::chrono::duration_cast<std::chrono::milliseconds>(rest);

        char timestamp_buf[256]{ 0 };
        time_t t = std::chrono::system_clock::to_time_t(now);
        struct tm lt;
        localtime_s(&lt, &t);
        snprintf(timestamp_buf, 256, "%02d/%02d/%02d %02d:%02d:%02d.%03d", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min, lt.tm_sec, (int)mili.count());

        char logbuf[256] = { 0 };

        va_start(va_alist, fmt);
        _vsnprintf_s(logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), 256, fmt, va_alist);
        va_end(va_alist);

        g_logfile << "[" << timestamp_buf << "] " << logbuf << std::endl;
    }
}

