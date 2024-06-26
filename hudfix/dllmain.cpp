#include "pch.h"

#include "HudNodes.h"
#include "CRenderer.h"
#include "CHudManager.h"
#include "Bf2String.h"
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <set>
#include <map>
#include <fstream>

static_assert(sizeof(void*) == 4, "don't");

#define CONFIG_PATH L"hud_config.txt"

// config
std::set<std::string> g_nodesRight;
std::set<std::string> g_nodesLeft;
std::set<std::string> g_nodesFillRight;
std::set<std::string> g_nodesFillLeft;
std::set<std::string> g_nodesMakeMovable;
std::map<std::string, std::set<std::string>> g_nodesClone;
std::map<std::string, std::string> g_nodesOverrideParent;
std::set<std::string> g_nodesKeepStretched;

bool g_offsetMinimap = true;

// logging
std::ofstream g_logfile;
void __cdecl log(const char* fmt, ...);

__declspec(dllexport) int main();
int main()
{
    return 0;
}

bool hook(void* toHook, void* ourFunct, size_t len, bool as_call) {
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
    *(BYTE*)toHook = as_call ? 0xE8 : 0xE9; // E8 - call, E9 - jmp
    *(DWORD*)((DWORD)toHook + 1) = relativeAddress;

    DWORD temp;
    if (!VirtualProtect(toHook, len, curProtection, &temp)) {
        log("ERROR: VirtualProtect failed to restore protection, error code: %i. aborting...", GetLastError());
        return false;
    }

    return true;
}

bool writeMem(void* dest, void* source, size_t len) {
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

bool fixHud() {
    dice::CHudManager* hudManager = *(dice::CHudManager**)0xA10890;
    if (hudManager == nullptr || hudManager->miniMap == nullptr) {
        log("WARNING: HudManager not initialized");
        return false;
    }

    dice::CRenderer* renderer = *(dice::CRenderer**)0x9A557C;
    if (renderer == nullptr) {
        log("ERROR: Renderer not found");
        return false;
    }
    float aspect_ratio = (float)renderer->screenWidth / (float)renderer->screenHeight;
    float newWidth = 600 * aspect_ratio;
    float newMouseOffset = -newWidth / 2;
    DWORD hudWidth = *(reinterpret_cast<DWORD*>(&newWidth));
    DWORD mouseOffset = *(reinterpret_cast<DWORD*>(&newMouseOffset));
    float hudOffset = (int)((600 * aspect_ratio - 800) / 2);

    // this will squish the entire HUD into 4:3 area in the screen center
    log("Changing HUD width to %f (0x%x)", newWidth, hudWidth);
    writeMem((void*)0x7AD489, (void*)&hudWidth, 4);
    // after destreatching HUD mouse movement will be restricted, this will extend it to the correct width
    log("Extending mouse area to %f (0x%x)", newWidth, hudWidth);
    writeMem((void*)0x752B56, (void*)&hudWidth, 4);
    // this is for offsettig the extended mouse area to the left so it'll fit in the screen
    log("Applying mouse area offset to %f (%x)", newMouseOffset, mouseOffset);
    writeMem((void*)0x752B94, (void*)&mouseOffset, 4);
    // fixes bug with corssiar going nuts when proning/standing up
    BYTE tmp1 = 0xEB;
    writeMem((void*)0x7A5763, &tmp1, 1);
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

    // Keep fullscreen overlays stretched
    for (std::set<std::string>::iterator it = g_nodesKeepStretched.begin(); it != g_nodesKeepStretched.end(); ++it) {
        if ((tmp = meme::findNode((*it).c_str())) != nullptr) {
            tmp->setNodeSize(tmp->getNodeSizeX() * aspect_ratio / 1.333333f, tmp->getNodeSizeY());
            tmp->setNodePos(tmp->getNodePosX() - hudOffset, tmp->getNodePosY());
        }
        else
            log("ERROR: %s Not Found!", (*it).c_str());
    }

    if (g_offsetMinimap) {
        hudManager->miniMap->resetX += hudOffset;
    }

    return true;
}

class BF2Engine;
bool(__thiscall* BF2Engine_initEngine)(BF2Engine* _this) = (bool(__thiscall*)(BF2Engine*))0x408EF0;

class Bf2HudBuilder;
bool(__thiscall* Bf2HudBuilder_createSplitNode)(Bf2HudBuilder* _this, dice::std::string parent, dice::std::string node) = (bool(__thiscall*)(Bf2HudBuilder*, dice::std::string, dice::std::string))0x79C420;
bool(__thiscall* Bf2HudBuilder_createTransformNode)(Bf2HudBuilder* _this, dice::std::string parent, dice::std::string node, int x, int y, int wx, int wy) = (bool(__thiscall*)(Bf2HudBuilder *, dice::std::string, dice::std::string, int, int, int, int))0x79C2E0;
bool(__thiscall* Bf2HudBuilder_createPictureNode)(Bf2HudBuilder* _this, dice::std::string parent, dice::std::string node, int x, int y, int wx, int wy) = (bool(__thiscall*)(Bf2HudBuilder*, dice::std::string, dice::std::string, int, int, int, int))0x79D050;

bool __fastcall Bf2HudBuilder_createPictureNode_hook(Bf2HudBuilder* _this, int, dice::std::string parent, dice::std::string node, int x, int y, int wx, int wy) {
    if (g_nodesOverrideParent.count(node.c_str())) {
        dice::std::string newParent = g_nodesOverrideParent.at(node.c_str()).c_str();
        log("Overriding parent for node %s from %s to %s", node.c_str(), parent.c_str(), newParent.c_str());
        return Bf2HudBuilder_createPictureNode(_this, newParent, node, x, y, wx, wy);
    }
    return Bf2HudBuilder_createPictureNode(_this, parent, node, x, y, wx, wy);
}

bool __fastcall Bf2HudBuilder_createTransformNode_hook(Bf2HudBuilder* _this, int, dice::std::string parent, dice::std::string node, int x, int y, int wx, int wy) {

    dice::std::string* overrideParent;
    dice::std::string newParent;

    if (g_nodesOverrideParent.count(node.c_str())) {
        newParent = g_nodesOverrideParent.at(node.c_str()).c_str();
        log("Overriding parent for node %s from %s to %s", node.c_str(), parent.c_str(), newParent.c_str());
        overrideParent = &newParent;
    }
    else {
        overrideParent = &parent;
    }

    if (g_nodesClone.count(node.c_str())) {
        std::set<std::string>& nodesToCreate = g_nodesClone.at(node.c_str());
        for (const std::string& nodeToCreate : nodesToCreate) {
            log("Creating new TransformNode %s %s", overrideParent->c_str(), nodeToCreate.c_str());
            dice::std::string clonedNode = nodeToCreate.c_str();
            Bf2HudBuilder_createTransformNode(_this, *overrideParent, clonedNode, x, y, wx, wy);
        }
    }

    return Bf2HudBuilder_createTransformNode(_this, *overrideParent, node, x, y, wx, wy);
}

bool __fastcall Bf2HudBuilder_createSplitNode_hook(Bf2HudBuilder* _this, int, dice::std::string parent, dice::std::string node) {
    dice::std::string* overrideParent;
    dice::std::string newParent;

    if (g_nodesOverrideParent.count(node.c_str())) {
        newParent = g_nodesOverrideParent.at(node.c_str()).c_str();
        log("Overriding parent for node %s from %s to %s", node.c_str(), parent.c_str(), newParent.c_str());
        overrideParent = &newParent;
    }
    else {
        overrideParent = &parent;
    }

    if (g_nodesMakeMovable.count(node.c_str())) {
        log("Overriding type of node %s from SplitNode to TransformNode", node.c_str());
        return Bf2HudBuilder_createTransformNode_hook(_this, NULL, *overrideParent, node, 0, 0, 600, 800);
    }
    return Bf2HudBuilder_createSplitNode(_this, *overrideParent, node);
}

bool __fastcall BF2Engine_initEngine_hook(BF2Engine* _this) {
    bool res = BF2Engine_initEngine(_this);
    fixHud();
    return res;
}

void readConfig(const wchar_t* dlldir) {
    wchar_t configPath[MAX_PATH];
    wcscpy_s(configPath, dlldir);
    wcscat_s(configPath, CONFIG_PATH);
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
                if (splited.at(0) == "offset_right" && splited.size() == 2) {
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
                else if (splited.at(0) == "clone" && splited.size() == 3) {
                    g_nodesClone[splited.at(1)].insert(splited.at(2));
                }
                else if (splited.at(0) == "make_movable" && splited.size() == 2) {
                    g_nodesMakeMovable.insert(splited.at(1));
                }
                else if (splited.at(0) == "override_parent" && splited.size() == 3) {
                    g_nodesOverrideParent.insert(::std::make_pair(splited.at(1), splited.at(2)));
                }
                else if (splited.at(0) == "keep_stretched" && splited.size() == 2) {
                    g_nodesKeepStretched.insert(splited.at(1));
                }
            }
        }
        config.close();
    }
}


BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
    HMODULE hExe = GetModuleHandle(0);
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        wchar_t dlldir[MAX_PATH];
        GetModuleFileName(hModule, dlldir, MAX_PATH);
        for (int i = wcslen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }

        readConfig(dlldir);

#ifdef _DEBUG
        wchar_t log_path[MAX_PATH];
        wcscpy_s(log_path, dlldir);
        wcscat_s(log_path, L"hudfixlog.txt");
        g_logfile.open(log_path, std::ios::app);
        log("\n---------------------\nBF2 stretched HUD FIX v0.6\n---------------------");
#endif

        hook((void*)0x40C519, BF2Engine_initEngine_hook, 5, true);
        hook((void*)0x762054, Bf2HudBuilder_createSplitNode_hook, 5, true);
        hook((void*)0x761D8A, Bf2HudBuilder_createTransformNode_hook, 5, true);
        hook((void*)0x7659BF, Bf2HudBuilder_createPictureNode_hook, 5, true);

        break;
    case DLL_PROCESS_DETACH:
        hook((void*)0x40C519, (void*)0x408EF0, 5, true);
        break;
    default:
        break;
    }

    return TRUE;
}

#ifdef _DEBUG
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
#else
void __cdecl log(const char* fmt, ...) { }
#endif // DEBUG

