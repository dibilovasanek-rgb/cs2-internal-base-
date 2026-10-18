#pragma once
#include "../../sdk/sdk.h"


enum EFlags : int
{
	FL_ONGROUND = (1 << 0), // entity is at rest / on the ground
	FL_DUCKING = (1 << 1), // player is fully crouched/uncrouched
	FL_ANIMDUCKING = (1 << 2), // player is in the process of crouching or uncrouching but could be in transition
	FL_WATERJUMP = (1 << 3), // player is jumping out of water
	FL_ONTRAIN = (1 << 4), // player is controlling a train, so movement commands should be ignored on client during prediction
	FL_INRAIN = (1 << 5), // entity is standing in rain
	FL_FROZEN = (1 << 6), // player is frozen for 3rd-person camera
	FL_ATCONTROLS = (1 << 7), // player can't move, but keeps key inputs for controlling another entity
	FL_CLIENT = (1 << 8), // entity is a client (player)
	FL_FAKECLIENT = (1 << 9), // entity is a fake client, simulated server side; don't send network messages to them
	FL_INWATER = (1 << 10), // entity is in water
	FL_FLY = (1 << 11),
	FL_SWIM = (1 << 12),
	FL_CONVEYOR = (1 << 13),
	FL_NPC = (1 << 14),
	FL_GODMODE = (1 << 15),
	FL_NOTARGET = (1 << 16),
	FL_AIMTARGET = (1 << 17),
	FL_PARTIALGROUND = (1 << 18), // entity is standing on a place where not all corners are valid
	FL_STATICPROP = (1 << 19), // entity is a static property
	FL_GRAPHED = (1 << 20),
	FL_GRENADE = (1 << 21),
	FL_STEPMOVEMENT = (1 << 22),
	FL_DONTTOUCH = (1 << 23),
	FL_BASEVELOCITY = (1 << 24), // entity have applied base velocity this frame
	FL_WORLDBRUSH = (1 << 25), // entity is not moveable/removeable brush (part of the world, but represented as an entity for transparency or something)
	FL_OBJECT = (1 << 26),
	FL_KILLME = (1 << 27), // entity is marked for death and will be freed by the game
	FL_ONFIRE = (1 << 28),
	FL_DISSOLVING = (1 << 29),
	FL_TRANSRAGDOLL = (1 << 30), // entity is turning into client-side ragdoll
	FL_UNBLOCKABLE_BY_PLAYER = (1 << 31)
};

class c_movement 
{
public:
	struct {
		bool m_autopeeking;
		bool m_should_retrack;
		bool did_shoot;
		vec3_t m_old_origin;
	} m_autopeek;

	void angle_qangles(const qangle_t& angles, qangle_t* forward, qangle_t* right, qangle_t* up);
	void auto_pick(c_user_cmd* pCmd);
	void bunnyhop(c_user_cmd* pCmd, c_player_pawn* pLocalPawn);
	void bunnyhope(c_user_cmd* pCmd);	
	void AutoStrafe(c_user_cmd* pCmd, c_player_pawn* pLocalPawn);
	float get_max_player_speed();
	void stop_movement(c_user_cmd* user_cmd, c_player_pawn* local_player);
	void auto_stop(c_user_cmd* user_cmd, c_player_pawn* local_player, c_base_player_weapon* active_weapon, bool no_spread);
	void limit_speed(c_user_cmd* user_cmd, c_player_pawn* local_player);
	void run_strafe();
	void movement_fix(c_user_cmd* user_cmd, vec3_t angle);
	//void movement_fix(CUserCmd* user_cmd, CBaseUserCmdPB* pUserCmd, vec3_t angle);

	static c_movement& get_instance() {
		static c_movement instance;
		return instance;
	}
};
inline c_movement* g_movement = &c_movement::get_instance();
