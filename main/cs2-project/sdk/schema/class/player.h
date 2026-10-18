#pragma once
#include "entity.h"
#include "weapon.h"
#include "../../game_class/usercmd.h"
#include "../../game_class/basehandle.h"

class c_weapon_base;
class c_cs_weapon_base_vdata;
class c_player_pawn;

class c_base_player_controller : public c_base_entity
{
public:
	SCHEMA(m_pawn, c_handle<c_player_pawn>, ("CBasePlayerController"), ("m_hPawn"));
	SCHEMA(m_is_local_player_controller, bool, ("CBasePlayerController"), ("m_bIsLocalPlayerController"));
	SCHEMA(m_game_scene_node, c_game_scene_node*, ("C_BaseEntity"), ("m_pGameSceneNode"));
	SCHEMA(m_p_collision, c_collision_property*, ("C_BaseEntity"), ("m_pCollision"));
	SCHEMA(m_steam_id, uint64_t, ("CBasePlayerController"), ("m_steamID"));



};
class c_user_cmd;

class CCSPlayerController_InGameMoneyServices
{
public:
	SCHEMA(m_get_money, int, ("CCSPlayerController_InGameMoneyServices"), ("m_iAccount"));
};

class c_player_controller : public c_base_player_controller
{
public:

	SCHEMA(m_ping, int, ("CCSPlayerController"), ("m_iPing"));
	SCHEMA(m_pawn_health, int, ("CCSPlayerController"), ("m_iPawnHealth"));
	SCHEMA(m_tick_base, uint32_t, ("CBasePlayerController"), ("m_nTickBase"));
	SCHEMA(m_pawn_armor, int, ("CCSPlayerController"), ("m_iPawnArmor"));
	SCHEMA(m_pawn_has_defuser, bool, ("CCSPlayerController"), ("m_bPawnHasDefuser"));
	SCHEMA(m_pawn_has_helmet, bool, ("CCSPlayerController"), ("m_bPawnHasHelmet"));
	SCHEMA(m_pawn_is_alive, bool, ("CCSPlayerController"), ("m_bPawnIsAlive"));
	SCHEMA(m_sanitized_player_name, const char*, ("CCSPlayerController"), ("m_sSanitizedPlayerName"));
	SCHEMA(get_player_pawn_observer, c_base_handle, ("CCSPlayerController"), ("m_hObserverPawn"));
	SCHEMA(handle_player_pawn, c_base_handle, ("CCSPlayerController"), ("m_hPlayerPawn"));
	SCHEMA(m_money_service, CCSPlayerController_InGameMoneyServices*, ("CCSPlayerController"), ("m_pInGameMoneyServices"));
	SCHEMA_OFFSET(c_user_cmd*, m_current_command, "CBasePlayerController", "m_steamID", -0x8);


	static c_player_controller* get_local_controller();
	c_player_pawn* get_player_pawn();
	c_player_pawn* get_observer_pawn();


};
class c_player_weapon_services;

class c_tonemap_controller_2
{
public:
	SCHEMA(m_auto_exposure_min, float, ("C_TonemapController2"), ("m_flAutoExposureMin"));
	SCHEMA(m_auto_exposure_max, float, ("C_TonemapController2"), ("m_flAutoExposureMax"));
};

class c_player_camera_services
{
public:
	SCHEMA(m_post_processing_volumes, c_handle<c_postproccesing>, ("CPlayer_CameraServices"), ("m_PostProcessingVolumes"));
	SCHEMA(m_active_post_processing_volume, c_handle<c_postproccesing>, ("CPlayer_CameraServices"), ("m_hActivePostProcessingVolume"));
	SCHEMA(m_tonemap_controller, c_handle<c_tonemap_controller_2>, ("CPlayer_CameraServices"), ("m_hTonemapController"));
	SCHEMA(m_vec_view_punch, vec3_t, ("CPlayer_CameraServices"), ("m_vecCsViewPunchAngle"));

};

class c_player_movement_service {
public:
	SCHEMA(m_max_speed, float, "CPlayer_MovementServices", "m_flMaxspeed");
	SCHEMA(m_surface_friction, float, "CPlayer_MovementServices_Humanoid", "m_flSurfaceFriction");
	SCHEMA(m_stamina, float, "CCSPlayer_MovementServices", "m_flStamina");
	SCHEMA(m_last_movements_impulse, vec3_t, "CPlayer_MovementServices", "m_vecLastMovementImpulses");


};

class c_player_observer_services
{	
public:
	SCHEMA(get_observer_mode, uint8_t, "CPlayer_ObserverServices", "m_iObserverMode");
	SCHEMA(get_observer_targer, c_base_handle, "CPlayer_ObserverServices", "m_hObserverTarget");
	SCHEMA(get_observer_last_mode, int, "CPlayer_ObserverServices", "m_iObserverLastMode");
	SCHEMA(get_forced_observer_mode, int, "CPlayer_ObserverServices", "m_bForcedObserverMode");
	/* SCHEMA(get_observer_mode, uint8_t, "CPlayer_ObserverServices" "m_iObserverMode");
	 SCHEMA(get_observer_targer, c_base_handle, "CPlayer_ObserverServices" "m_hObserverTarget");
	 SCHEMA(get_observer_last_mode, int, "CPlayer_ObserverServices" "m_iObserverLastMode");
	 SCHEMA(get_forced_observer_mode, int, "CPlayer_ObserverServices" "m_bForcedObserverMode");*/
};

class c_cs_player_item_services
{
public:
	SCHEMA(m_has_defuser, bool, ("CCSPlayer_ItemServices"), ("m_bHasDefuser"));
	SCHEMA(m_has_helmet, bool, ("CCSPlayer_ItemServices"), ("m_bHasHelmet"));
	bool m_has_heavy_armor() const { return false; }
};

class c_cs_player_view_model_service {
public:
	c_base_handle handle;
    c_base_handle& m_view_model() { return handle; }
};

class c_base_player_pawn : public c_base_model_entity
{
public:
	SCHEMA(m_weapon_services, c_player_weapon_services*, ("C_BasePlayerPawn"), ("m_pWeaponServices"));
	SCHEMA(m_camera_services, c_player_camera_services*, ("C_BasePlayerPawn"), ("m_pCameraServices"));
	SCHEMA(m_item_services, c_cs_player_item_services*, ("C_BasePlayerPawn"), ("m_pItemServices"));
	SCHEMA(get_observer_service, c_player_observer_services*, ("C_BasePlayerPawn"), ("m_pObserverServices"));
	SCHEMA(get_controller_handle, c_base_handle, ("C_BasePlayerPawn"), ("m_hController"));
	// SCHEMA_ADD_FIELD(CBaseHandle, GetControllerHandle, "C_BasePlayerPawn->m_hController");
	//CPlayer_ObserverServices*, GetObserverServices, "C_BasePlayerPawn->m_pObserverServices")
	SCHEMA(m_movement_services, c_player_movement_service*, ("C_BasePlayerPawn"), ("m_pMovementServices"));
	c_cs_player_view_model_service* m_view_model_services();

};

class c_player_pawn_base : public c_base_player_pawn
{
public:
	SCHEMA(m_armor_value, int, ("C_CSPlayerPawn"), ("m_ArmorValue"));
	SCHEMA(m_is_scoped, bool, ("C_CSPlayerPawn"), ("m_bIsScoped"));
	SCHEMA(m_gun_game_immunity, bool, ("C_CSPlayerPawn"), ("m_bGunGameImmunity"));
	SCHEMA(m_iID_ent_index, int, ("C_CSPlayerPawn"), ("m_iIDEntIndex"));
	SCHEMA(m_need_to_reapply_gloves, bool, ("C_CSPlayerPawn"), ("m_bNeedToReApplyGloves"));
	c_utl_vector<vec3_t>& m_aim_punch_cache() {
        using fn_t=vec3_t*(__fastcall*)(void*,vec3_t*,bool);
        static auto fn=reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL,"40 53 48 83 EC 20 48 8B 89 98 15 00 00 48 8B DA E8 ? ? ? ? 48 8B C3 48 83 C4 20 5B C3"));
        static thread_local vec3_t angle{};static thread_local c_utl_vector<vec3_t> values;
        angle={};if(fn) fn(this,&angle,true);values.m_size=1;values.m_elements=&angle;return values;
    }

	SCHEMA(m_eye_angle, vec3_t, ("C_CSPlayerPawn"), ("m_angEyeAngles"));

	SCHEMA(m_controller_handle, c_base_handle, ("C_CSPlayerPawnBase"), ("m_hOriginalController"));

};


class c_base_cs_grenade : public c_weapon_base {
public:
	SCHEMA(m_held_by_player, bool, "C_BaseCSGrenade", "m_bIsHeldByPlayer");
	SCHEMA(m_pin_pulled, bool, "C_BaseCSGrenade", "m_bPinPulled");
	SCHEMA(m_throw_time, float, "C_BaseCSGrenade", "m_fThrowTime");
	SCHEMA(m_throw_strength, float, "C_BaseCSGrenade", "m_flThrowStrength");
	SCHEMA(m_redraw, bool, "C_BaseCSGrenade", "m_bRedraw");

};

class c_player_pawn : public c_player_pawn_base
{
public:
	//bool m_is_friend();
	//bool m_has_armour(const int hit_group);
	//void get_bone_position(std::uint32_t bone_index, vec3_t& position, vec3_t& rotation);

	//vec3_t get_hitbox_pos(std::uint32_t bone_index)
	//{
	//    vec3_t position, rotation;
	//    get_bone_position(bone_index, position, rotation);
	//    return position;
	//}
	SCHEMA(m_EconGloves, c_econ_item_view, "C_CSPlayerPawn", "m_EconGloves");
	SCHEMA(m_flash_duration, float, "C_CSPlayerPawn", "m_flFlashDuration");
	SCHEMA(m_is_defusing, bool, "C_CSPlayerPawn", "m_bIsDefusing");
	SCHEMA(m_emit_sound_time, float, "C_CSPlayerPawn", "m_flEmitSoundTime");

	__forceinline vec3_t get_eye_pos() {
        auto* node=m_game_scene_node();
        const auto offset=g_schema->schema_get_offset("C_BaseModelEntity","m_vecViewOffset");
        return node && offset ? node->m_vec_abs_origin()+*reinterpret_cast<vec3_t*>(reinterpret_cast<std::byte*>(this)+offset) : vec3_t{};
	}

	int get_bone_index(const char* name) {
		static const auto fn = reinterpret_cast<int(__fastcall*)(void*, const char*)>(g_memory->GetAbsoluteAddress((uint8_t*)g_memory->find_pattern(CLIENT_DLL, "E8 ? ? ? ? 85 C0 78 ? 4C 8D 4D"), 0x1));
		return fn && name ? fn(this, name) : -1;
	}

	bool hasArmour(const int hitgroup)
	{
		if (!this->m_item_services())
			return false;

		switch (hitgroup)
		{
		case 1:
			return this->m_item_services()->m_has_helmet();
		case 2:
		case 0:
		case 3:
		case 4:
		case 5:
			return true;
		default:
			return false;
		}
	}

	int get_lerp_tick(int player_tick_count);
	std::uint32_t get_owner_handle_idx();
	std::uint16_t GetCollisionMask();
	bool is_throwing();
	c_weapon_base* ActiveWeapon();
	bool has_c4();

	//bool is_visible(c_player_pawn* local);
};

