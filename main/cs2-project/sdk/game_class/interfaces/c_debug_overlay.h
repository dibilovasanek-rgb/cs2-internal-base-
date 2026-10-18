#pragma once
#include "../../math/vector.h"
#include "../../../memory/memory.h"
#include "../color.h"

class c_debug_overlay {
public:
    void add_box(const vec3_t& end, const vec3_t& mins, const vec3_t& maxs, const vec3_t& a3, Color_t clr)
    {
        g_memory->CallVFunc<void>(this, 48U, end, mins, maxs, a3, (int)clr.r, (int)clr.g, (int)clr.b, (int)clr.a, static_cast<double>(4.f));
    }
};