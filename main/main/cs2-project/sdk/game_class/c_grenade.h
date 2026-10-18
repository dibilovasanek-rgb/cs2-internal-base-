#pragma once
#include "../math/vector.h"

struct grenade_positions
{
    int size;
    char pad[0x4];
    vec3_t* data;
    std::uint64_t unk;
};

class c_grenade_trace
{
public:
    char pad[0xCE0];
    float duration;
    char pad2[0x1C + 0x10];
    std::uint64_t size;
    grenade_positions* data;
};