#include "pch.h"
#include "HudNodes.h"


meme::HudNode::HudNode(Node* node) {
    this->node = (DWORD*)node;
    this->name = node->name;
    this->x = &(node->x);
    this->y = &(node->y);
    this->width = &(node->width);
    this->height = &(node->height);
}

meme::HudNode::HudNode(SplitNode* node) {
    this->node = (DWORD*)node;
    this->name = 0;
    this->x = 0;
    this->y = 0;
    this->width = 0;
    this->height = 0;
}

bool meme::HudNode::isSplitNode() {
    if (*node == MEME_SPLIT_NODE)
        return true;
    else
        return false;
}

bool meme::HudNode::setNodePos(float posX, float posY) {
    if (isSplitNode()) {
        return false;
    }
    else {
        *x = posX;
        *y = posY;
        return true;
    }
}

bool meme::HudNode::setNodeSize(float sizeX, float sizeY) {
    if (isSplitNode()) {
        return false;
    }
    else {
        *width = sizeX;
        *height = sizeY;
        return true;
    }
}

float meme::HudNode::getNodePosX() {
    if (isSplitNode()) {
        return 0;
    }
    else {
        return *x;
    }
}

float meme::HudNode::getNodePosY() {
    if (isSplitNode()) {
        return 0;
    }
    else {
        return *y;
    }
}

float meme::HudNode::getNodeSizeX() {
    if (isSplitNode()) {
        return 0;
    }
    else {
        return *width;
    }
}

float meme::HudNode::getNodeSizeY() {
    if (isSplitNode()) {
        return 0;
    }
    else {
        return *height;
    }
}

char* meme::HudNode::getNodeName() {
    return name;
}

DWORD* meme::HudNode::getNodePtr() {
    return node;
}

meme::HudNode* meme::findNode(DWORD* node, const char* name) {
    meme::HudNode* ret = nullptr;
    switch (*node) {
    case MEME_NODE:
        if (strcmp(((meme::Node*)node)->name, name) == 0) {
            ret = new meme::HudNode((meme::Node*)node);
            return ret;
        }
        if (((meme::Node*)node)->pChild != NULL) {
            if ((ret = findNode((DWORD*)((meme::Node*)node)->pChild, name)) != nullptr)
                return ret;
        }
        if (((meme::Node*)node)->pSibling != NULL) {
            if ((ret = findNode((DWORD*)((meme::Node*)node)->pSibling, name)) != nullptr)
                return ret;
        }
        break;
    case MEME_SPLIT_NODE:
        if (strcmp(((meme::SplitNode*)node)->name, name) == 0) {
            ret = new meme::HudNode((meme::SplitNode*)node);
            return ret;
        }
        if (((meme::SplitNode*)node)->pChild != NULL) {
            if ((ret = findNode((DWORD*)((meme::SplitNode*)node)->pChild, name)) != nullptr)
                return ret;
        }
        if (((meme::SplitNode*)node)->pSibling != NULL) {
            if ((ret = findNode((DWORD*)((meme::SplitNode*)node)->pSibling, name)) != nullptr)
                return ret;
        }
        break;
    case MEME_MAP_NODE:
        if (strcmp("Minimap", name) == 0) {  // actually you cannot create MapNode that isn't named 'Minimap' #justBF2Things
            ret = new meme::HudNode((meme::Node*)node);
            return ret;
        }
    default:
        if (((meme::UnknownNode*)node)->pSibling != NULL) {
            if ((ret = findNode((DWORD*)((meme::UnknownNode*)node)->pSibling, name)) != nullptr)
                return ret;
        }
        break;
    }

    return ret;
}

meme::HudNode* meme::findNode(const char* name) {
    meme::HudNode* ret;
    meme::RootNode* global = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x08) + 0x04);
    if ((DWORD)(global->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)global->pSibling, name)) != nullptr)
            return ret;

    }
    meme::RootNode* bottomRightAnimate = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x04) + 0x08) + 0x24) + 0x04) + 0x04);
    if ((DWORD)(bottomRightAnimate->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)bottomRightAnimate->pSibling, name)) != nullptr)
            return ret;
    }
    meme::RootNode* bottomRightStatic = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x04) + 0x08) + 0x04) + 0x08) + 0x04);
    if ((DWORD)(bottomRightStatic->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)bottomRightStatic->pSibling, name)) != nullptr)
            return ret;
    }
    meme::RootNode* bottomLeftAnimate = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x24) + 0x04) + 0x04);
    if ((DWORD)(bottomLeftAnimate->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)bottomLeftAnimate->pSibling, name)) != nullptr)
            return ret;
    }
    meme::RootNode* bottomLeftStatic = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x04) + 0x08) + 0x04);
    if ((DWORD)(bottomLeftStatic->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)bottomLeftStatic->pSibling, name)) != nullptr)
            return ret;
    }
    meme::RootNode* topLayer = (meme::RootNode*)*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(*(DWORD*)0xA10890 + 0xFC) + +0x04) + 0x34) + 0x08) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04) + 0x08) + 0x04) + 0x04) + 0x04) + 0x04) + 0x04) + 0x04) + 0x04) + 0x08) + 0x08) + 0x04);
    if ((DWORD)(topLayer->vptr) == MEME_ROOT_NODE) {
        if ((ret = findNode((DWORD*)topLayer->pSibling, name)) != nullptr)
            return ret;
    }

    return nullptr;
}