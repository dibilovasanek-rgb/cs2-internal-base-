#pragma once
#include "../../sdk/input_system/inputsystem.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../sdk/schema/class/player.h"

enum AA_style : int
{
	BACKWARDS,
	CUSTOM,
	JITTER
};

class c_antiaim
{
public:
	void runAA(c_user_cmd* pCmd, c_player_controller* pLocalController, c_player_pawn* pLocalPawn);

	static c_antiaim& get_instance() {
		static c_antiaim instance;
		return instance;
	}
};
inline c_antiaim* g_antiaim = &c_antiaim::get_instance();
