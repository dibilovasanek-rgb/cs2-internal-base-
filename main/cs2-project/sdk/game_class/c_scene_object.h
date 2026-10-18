#pragma once
#include "color.h"
#include "caggregatescene.h"

class c_scene_light_object {
public:
    char pad_0000[0xE4]; // 0x0
    Color_t m_color; // 0xE4
};

class c_aggregate_scene_object_data {
private:
    char pad_0000[0x38]; // 0x0
public:
    ByteColor    m_color; // 0x38
private:
    char pad_0038[0x9];
};

class c_aggregate_scene_object {
private:
    char pad_0000[0x120];
public:
    int m_count; // 0x120
private:
    char pad_0120[0x4];
public:
    c_aggregate_scene_object_data* m_array; // 0x128
};

class c_scene_object
{
public:
    char pad_0000[184]; //0x0000
    uint8_t r; //0x00B8
    uint8_t g; //0x00B9
    uint8_t b; //0x00BA
    uint8_t a; //0x00BB
    char pad_00BC[196]; //0x00BC
}; //Size: 0x0180

class c_base_scene_data
{
public:
    char pad_0000[24]; //0x0000
    c_scene_object* m_sceneObject; //0x0018
    CMaterial2* m_material; //0x0020
    CMaterial2* m_material2; //0x0028
    char pad_0030[32]; //0x0030
    uint8_t r; //0x0040
    uint8_t g; //0x0041
    uint8_t b; //0x0042
    uint8_t a; //0x0043
    char pad_0044[28]; //0x0044
}; //Size: 0x0068
static_assert(sizeof(c_base_scene_data) == 0x70);
