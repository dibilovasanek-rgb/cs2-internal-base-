#pragma once
#include "../../../memory/memory.h"
#include "../utl.h"
#include "../c_econ_definition.h"
#include "c_debug_overlay.h"

class c_econ_item_schema {
public:
    std::byte pad_001[0x120];
    c_utl_map<int, c_econ_item_definition*> m_sorted_item_definition_map;
    std::byte pad_002[0x128];
    c_utl_map<uint64_t, c_alternate_icon_data> m_alternate_icons_map;
    c_utl_map<int, c_paint_kit*> m_paint_kits;
    c_utl_map<int, c_sticker_kit*> m_sticker_kits;
    void* get_attribute_definition_interface(int index) { return g_memory->CallVFunc<void*>(this,27U,index); }
    auto& get_sorted_item_definition_map() { return m_sorted_item_definition_map; }
    auto& get_alternate_icons_map() { return m_alternate_icons_map; }
    auto& get_paint_kits() { return m_paint_kits; }
};
static_assert(offsetof(c_econ_item_schema,m_sorted_item_definition_map)==0x120);
static_assert(offsetof(c_econ_item_schema,m_alternate_icons_map)==0x270);
static_assert(offsetof(c_econ_item_schema,m_paint_kits)==0x298);
static_assert(offsetof(c_econ_item_schema,m_sticker_kits)==0x2C0);

class c_econ_item_system {
public:
	c_econ_item_schema* get_econ_item_schema() {
		return *reinterpret_cast<c_econ_item_schema**>((uintptr_t)(this) + 0x8);
	}
};

class c_local_data
{
public:
	char pad_0000[8]; //0x0000
	float N0000006F; //0x0008
	vec3_t m_eye_pos; //0x000C
}; //Size: 0x0018

class c_networked_client_info
{
public:
	int32_t unk; //0x0000
	int32_t m_render_tick; //0x0004
	float m_render_tick_fraction; //0x0008
	int32_t m_player_tick_count; //0x000C
	float m_player_tick_fraction; //0x0010
	char pad_0014[4]; //0x0014
	c_local_data* m_local_data; //0x0018
	char pad_0020[32]; //0x0020
}; //Size: 0x0040

class i_client {
public:
	c_econ_item_system* get_econ_item_system() {
		using fn_t=c_econ_item_system*(__fastcall*)();
        static auto fn=reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL,"48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 81"));
        return fn ? fn() : nullptr;
	}

	float get_lerp_time() {
		return g_memory->CallVFunc<float>(this, 166);
	}

	c_debug_overlay* get_scene_debug_overlay() {
		return g_memory->CallVFunc<c_debug_overlay*>(this, 163U);
	}
};