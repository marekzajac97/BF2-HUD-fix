#pragma once

namespace dice {
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
		char N00FA8F34; //0x069C 
		char N01061B57; //0x069D 
		char N01053840; //0x069E 
		char bDrawWater; //0x069F 
		char N00FA8F33; //0x06A0 
		char N01048FDA; //0x06A1 
		char N0104978B; //0x06A2 
		char bShowBodies; //0x06A3 
		char N00FA8F32; //0x06A4 
		char bShowGround; //0x06A5 
		char bShowGrass; //0x06A6 
		char bShowTrees; //0x06A7 
		char bShowNameTags; //0x06A8 Nametags
		char N00FBAEB4; //0x06A9 
		char N00FC0334; //0x06AA 
		char N00FBAEB5; //0x06AB 
		__int32 DrawFps; //0x06AC 
		char N00DCC523; //0x06B0 
		char bShowHud; //0x06B1 
		char bDrawSky; //0x06B2 
		char bDrawSunFlare; //0x06B3 
		char bDrawConsole; //0x06B4 
	};//Size=0x06B5
}