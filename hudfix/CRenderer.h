#pragma once
#include "Windows.h"

class CRenderer
{
public:
	char _0x0000[8];
	/* IDirect3D9* */ void* Direct3D_Device; //0x0008 
	/* IDirect3DDevice9* */ void* Game_Device; //0x000C
	char _0x0010[20];
	int screenWidth; //0x0024
	int screenHeight; //0x0028
	char _0x002C[1648];
	BYTE N00FA8F34; //0x069C 
	BYTE N01061B57; //0x069D 
	BYTE N01053840; //0x069E 
	BYTE bDrawWater; //0x069F 
	BYTE N00FA8F33; //0x06A0 
	BYTE N01048FDA; //0x06A1 
	BYTE N0104978B; //0x06A2 
	BYTE bShowBodies; //0x06A3 
	BYTE N00FA8F32; //0x06A4 
	BYTE bShowGround; //0x06A5 
	BYTE bShowGrass; //0x06A6 
	BYTE bShowTrees; //0x06A7 
	BYTE bShowNameTags; //0x06A8 Nametags
	BYTE N00FBAEB4; //0x06A9 
	BYTE N00FC0334; //0x06AA 
	BYTE N00FBAEB5; //0x06AB 
	__int32 DrawFps; //0x06AC 
	BYTE N00DCC523; //0x06B0 
	BYTE bShowHud; //0x06B1 
	BYTE bDrawSky; //0x06B2 
	BYTE bDrawSunFlare; //0x06B3 
	BYTE bDrawConsole; //0x06B4 
};//Size=0x06B5