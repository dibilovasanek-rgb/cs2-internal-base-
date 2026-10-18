#pragma once
#include "../../math/vector.h"
#include "../../../memory/memory.h"
class CViewSetup {
public:
    MEM_PAD(0x498);
    float flFov;
    float flFovViewmodel;
    vec3_t vecOrigin;
    MEM_PAD(0xC);
    qangle_t angView;
    MEM_PAD(0x10);
    float flAspectRatio;
    MEM_PAD(0x79);
    std::uint8_t nViewFlags;
};
static_assert(offsetof(CViewSetup, flFov) == 0x498);
static_assert(offsetof(CViewSetup, angView) == 0x4B8);
static_assert(offsetof(CViewSetup, flAspectRatio) == 0x4D4);
static_assert(offsetof(CViewSetup, nViewFlags) == 0x551);
