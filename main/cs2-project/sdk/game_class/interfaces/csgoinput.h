#pragma once
#include "../../math/vector.h"
#include "../../../memory/memory.h"

class CSubtickInput
{
	uint64_t nButton; // 0x18
	bool bPressed; // 0x20
	float flWhen; // 0x24
	float flAnalogForwardDelta; // 0x28
	float flAnalogLeftDelta; // 0x2C
};

struct c_cs_input_message
{
	int32_t m_frame_tick_count; //0x0000
	float m_frame_tick_fraction; //0x0004
	int32_t m_player_tick_count; //0x0008
	float m_player_tick_fraction; //0x000C
	qangle_t m_view_angles; //0x0010
	qangle_t m_shoot_position; //0x001C
	int32_t m_target_index; //0x0028
	qangle_t m_target_head_position; //0x002C
	qangle_t m_target_abs_origin; //0x0038
	qangle_t m_target_angle; //0x0044
	int32_t m_sv_show_hit_registration; //0x0050
	int32_t m_entry_index_max; //0x0054
	int32_t m_index_idk; //0x0058
};

class c_input_message
{
public:
	int32_t RenderTickCount; //0x0000
	float RenderFraction; //0x0004
	int32_t PlayerTickCount; //0x0008
	float PlayerFraction; //0x000C
	vec3_t Angle; //0x0010
	vec3_t Position; //0x001C
	char pad_0028[40]; //0x0028
	int32_t N000005D8; //0x0050
	char pad_0054[4]; //0x0054
	int32_t N000005D9; //0x0058
	int32_t N000005E5; //0x005C
}; //Size: 0x0060

class c_input_message_pb
{
public:
	c_input_message history[4];
};

class c_input_messages
{
public:
	int64_t size;
	c_input_message_pb* pb; //0x0000
}; //Size: 0x01E0

class c_user_cmd;
class c_player_controller;
class csgoinput
{
public:
	char pad_0000[592]; //0x0000
	bool bBlockShot; //0x0250
	bool bInThirdPerson; //0x0251
	char pad_0252[6]; //0x0252
	qangle_t angThirdPersonAngles; //0x0258
	char pad_0264[20]; //0x0264
	uint64_t nKeyboardPressed; //0x0278
	uint64_t nMouseWheelheelPressed; //0x0280
	uint64_t nUnPressed; //0x0288
	uint64_t nKeyboardCopy; //0x0290
	float flForwardMove; //0x0298
	float flSideMove; //0x029C
	float flUpMove; //0x02A0
	vec2_t nMousePos; //0x02A4
	int32_t SubticksCount; //0x02AC
	CSubtickInput Subticks[12]; //0x02B0
	vec3_t vecViewAngle; //0x03D0
	int32_t nTargetHandle; //0x03DC
	char pad_03E0[560]; //0x03E0
	int32_t nAttackStartHistoryIndex1; //0x0610
	int32_t nAttackStartHistoryIndex2; //0x0614
	int32_t nAttackStartHistoryIndex3; //0x0618
	char pad_061C[4]; //0x061C
	int32_t MessageSize; //0x0620
	char pad_0624[4]; //0x0624
    MEM_PAD(8);
    c_input_message* input_messages;

	c_user_cmd* get_usercmd(c_player_controller* pLocalController)
	{
        if (!pLocalController) return nullptr;
        using fn_t=void*(__fastcall*)(void*);
        static auto fn=reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL,"41 56 41 57 48 83 EC ? 48 8D 54 24"));
        auto* manager=fn ? static_cast<std::byte*>(fn(pLocalController)) : nullptr;
        if (!manager) return nullptr;
        const int sequence=*reinterpret_cast<int*>(manager+0x5910);
        if (sequence<=0) return nullptr;
        auto* cmd=reinterpret_cast<c_user_cmd*>(manager+(sequence%150)*0x98);
        return *reinterpret_cast<int*>(reinterpret_cast<std::byte*>(cmd)+8)==sequence ? cmd : nullptr;
	}

	vec3_t GetViewAngles() {
		using GetViewAngles_t = void* (__fastcall*)(csgoinput*, int);
		static auto fnGetViewAngles = reinterpret_cast<GetViewAngles_t>(g_memory->find_pattern(CLIENT_DLL, "4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3"));

		auto* angles = fnGetViewAngles ? reinterpret_cast<vec3_t*>(fnGetViewAngles(this, 0)) : nullptr;
        return angles ? *angles : vec3_t{};
	}

	void SetViewAngles(vec3_t& ViewAngles) {
		using SetViewAngles_t = void(__fastcall*)(csgoinput*, int, vec3_t&);
		static auto fnSetViewAngles = reinterpret_cast<SetViewAngles_t>(g_memory->find_pattern(CLIENT_DLL, "85 D2 75 ? 48 63 81"));

		if (fnSetViewAngles) fnSetViewAngles(this, 0, ViewAngles);
	}

};