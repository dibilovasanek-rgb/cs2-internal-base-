#pragma once
#include "color.h"

class CMaterial2
{
public:
    virtual const char* GetName() = 0;
    virtual const char* GetShareName() = 0;
};

class CAggregateSceneObjectData {
public:
    char pad1[(0x38)];
    Color_t m_rgba;
    char pad2[(0x8)];
};

class CAggregateSceneObject /*: public CSceneObject*/ {
public:
    char pad1[(0x120)];
    int count;
    char pad2[(0x4)];
    CAggregateSceneObjectData* data;
};

class CMeshDrawPrimitive_t {
public:
    char pad1[(0x18)];
    CAggregateSceneObject* m_pObject;
    CMaterial2* m_pMaterial;
    char pad2[(0x28)];
    Color_t m_rgba;
};