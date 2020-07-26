#pragma once
#include <Windows.h>
#include <fstream>

struct Vec2 {
    FLOAT x = 0.0f;
    FLOAT y = 0.0f;
}typedef Vec2;

void __cdecl log(const char* fmt, ...);
void update_offsets(float);
void clear_hudinit();
void append_hudinit(const char*, Vec2);
void append_hudinit(const char*, int);
void append_hudinit(const char*, const char*);
