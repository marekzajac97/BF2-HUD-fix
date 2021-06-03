#pragma once
#include "Windows.h"

#define MEME_NODE               0x935EC8
#define MEME_ROOT_NODE          0x94E5C0
#define MEME_SPLIT_NODE         0x9377D0
#define MEME_MAP_NODE           0x930188

namespace meme {
	struct Node {
		void* vptr; // 0x935EC8
		void* pSibling;
		void* pChild;
		float x;
		float y;
		float width;
		float height;
		unsigned int unk1;
		unsigned int unk2;
		char* name;
	};

	struct SplitNode {
		void* vptr; // 0x9377D0
		void* pSibling;
		void* pChild;
		char* name;
	};

	struct RootNode {
		void* vptr; // 0x94E5C0
		void* pSibling;
		char* name;
	};

	struct MapNode {
		void* vptr;          // 0x00
		void* pSibling;      // 0x04
		unsigned int unk1;   // 0x08
		float currentX;      // 0x0C
		float currentY;      // 0x10
		float currentWidth;  // 0x14
		float currentHeight; // 0x18
		unsigned int unk2[0x1D9];
		float targetX;       // 0x780
		float targetY;       // 0x784
		float resetX;        // 0x788
		float resetY;        // 0x78C
		unsigned int unk3[2];
		float resetWidth;   // 0x798
		float resetHeight;  // 0x79C
	};

	struct UnknownNode {
		void* vptr;
		void* pSibling;
	};

	// hud node wrapper class
	class HudNode {
	private:
		DWORD* node = NULL;
		float* x;
		float* y;
		float* width;
		float* height;
		char* name;
	public:
		HudNode(Node*);
		HudNode(SplitNode*);
		bool isSplitNode();
		bool setNodePos(float, float);
		bool setNodeSize(float, float);
		float getNodePosX();
		float getNodePosY();
		float getNodeSizeX();
		float getNodeSizeY();
		char* getNodeName();
		DWORD* getNodePtr();
	};

	HudNode* findNode(const char* name);
	HudNode* findNode(DWORD* node, const char* name);
}