#pragma once
#include "game_class/usercmd.h"
#include "schema/class/player.h"

namespace sdk {
	inline c_user_cmd* m_cmd;
	inline c_player_pawn* m_local_pawn;
	inline c_player_controller* m_local_controller;
	inline ImVec2 ScreenSize = { 0,0 };
	inline qangle_t angleStor = {};

	inline bool on_round_start = false;
};
