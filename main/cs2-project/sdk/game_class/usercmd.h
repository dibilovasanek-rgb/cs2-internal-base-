#pragma once
#include <limits>
#include <cstdint>
#include "..\..\memory\memory.h"
#include "../math/vector.h"
#include "../protobuffs/cs_usercmd.pb.h"
#include "../protobuffs/protobufs.h"

// @source: master/game/shared/in_buttons.h
enum ECommandButtons : int
{
	IN_ATTACK = (1 << 0),
	IN_JUMP = (1 << 1),
	IN_DUCK = (1 << 2),
	IN_FORWARD = (1 << 3),
	IN_BACK = (1 << 4),
	IN_USE = (1 << 5),
	IN_CANCEL = (1 << 6),
	IN_LEFT = (1 << 7),
	IN_RIGHT = (1 << 8),
	IN_MOVELEFT = (1 << 9),
	IN_MOVERIGHT = (1 << 10),
	IN_SECOND_ATTACK = (1 << 11),
	IN_RUN = (1 << 12),
	IN_RELOAD = (1 << 13),
	IN_LEFT_ALT = (1 << 14),
	IN_RIGHT_ALT = (1 << 15),
	IN_SCORE = (1 << 16),
	IN_SPEED = (1 << 17),
	IN_WALK = (1 << 18),
	IN_ZOOM = (1 << 19),
	IN_FIRST_WEAPON = (1 << 20),
	IN_SECOND_WEAPON = (1 << 21),
	IN_BULLRUSH = (1 << 22),
	IN_FIRST_GRENADE = (1 << 23),
	IN_SECOND_GRENADE = (1 << 24),
	IN_MIDDLE_ATTACK = (1 << 25),
	IN_USE_OR_RELOAD = (1 << 26)
};

// @source: server.dll
enum ECommandButtonsServer : std::uint64_t
{
	IN_SPRINT = 1 << 16,
	IN_JOYAUTOSPRINT = 1 << 17,
	IN_SHOWSCORES = 1ULL << 33,
	IN_LOOKATWEAPON = 1ULL << 35
};

struct CInButtonState
{
public:
	MEM_PAD(0x8) // 0x0 VTABLE
	std::uint64_t nValue; // 0x8
	std::uint64_t nValueChanged; // 0x10
	std::uint64_t nValueScroll; // 0x18
};
static_assert(sizeof(CInButtonState) == 0x20);

class c_user_cmd
{
public:
	void* __vfptr; //0x00
	unsigned int m_command_number; //0x004
	c_csgo_user_cmd_pb pb;
	CInButtonState m_button_state; //0x0040
	MEM_PAD(0x10); // 0x78
	bool b_has_been_predicted;
	MEM_PAD(0xF);

	void SetSubTickAngle(const qangle_t& angView)
	{
		for (int i = 0; i < this->pb.input_history_size(); i++)
		{
			auto tick = pb.mutable_input_history(i);
			if (!tick || !tick->mutable_view_angles())
				continue;

			tick->mutable_view_angles()->set_x(angView.x);
			tick->mutable_view_angles()->set_y(angView.y);
			tick->mutable_view_angles()->set_z(angView.z);
		}
	}
};
static_assert(sizeof(c_user_cmd) == 0x98);

static_assert(sizeof(CBaseUserCmdPB)==0x88);
static_assert(sizeof(c_csgo_user_cmd_pb)==0x48);
