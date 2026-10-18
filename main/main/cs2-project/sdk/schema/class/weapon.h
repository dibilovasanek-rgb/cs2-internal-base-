#pragma once
#include "entity.h"
#include "../../game_class/c_econ_definition.h"
#include "../../game_class/game_enums.h"
class c_weapon_i
{
private:
	using FnGetSpread = float(__fastcall*)(void*);
	using FnUpdateAccuracyPenalty = void(__fastcall*)(void*);

public:
	inline static FnGetSpread get_spread;
	inline static FnUpdateAccuracyPenalty update_accuracypenality;

	static void setup()
	{
		get_spread = reinterpret_cast<FnGetSpread>(g_memory->find_pattern(CLIENT_DLL, "48 63 91 ? ? ? ? 48 8B 81 ? ? ? ? 85 D2 78 ? 48 83 FA 02 73 ? F3 0F 10 84 90 50 07 00 00 C3 F3 0F 10 80 50 07 00 00 C3"));
		update_accuracypenality = reinterpret_cast<FnUpdateAccuracyPenalty>(g_memory->find_pattern(CLIENT_DLL, "40 57 41 56 48 83 EC 68 48 8B F9 E8 ? ? ? ? 4C 8B F0 48 85 C0"));
	}
};

enum e_weapon_type : uint32_t
{
	WEAPONTYPE_KNIFE = 0x0,
	WEAPONTYPE_PISTOL = 0x1,
	WEAPONTYPE_SUBMACHINEGUN = 0x2,
	WEAPONTYPE_RIFLE = 0x3,
	WEAPONTYPE_SHOTGUN = 0x4,
	WEAPONTYPE_SNIPER_RIFLE = 0x5,
	WEAPONTYPE_MACHINEGUN = 0x6,
	WEAPONTYPE_C4 = 0x7,
	WEAPONTYPE_TASER = 0x8,
	WEAPONTYPE_GRENADE = 0x9,
	WEAPONTYPE_EQUIPMENT = 0xa,
	WEAPONTYPE_STACKABLEITEM = 0xb,
	WEAPONTYPE_FISTS = 0xc,
	WEAPONTYPE_BREACHCHARGE = 0xd,
	WEAPONTYPE_BUMPMINE = 0xe,
	WEAPONTYPE_TABLET = 0xf,
	WEAPONTYPE_MELEE = 0x10,
	WEAPONTYPE_SHIELD = 0x11,
	WEAPONTYPE_ZONE_REPULSOR = 0x12,
	WEAPONTYPE_UNKNOWN = 0x13,
};

class c_cs_weapon_base_vdata
{
public:
	SCHEMA(m_imaxclip1, int32_t, ("CBasePlayerWeaponVData"), ("m_iMaxClip1"));
	SCHEMA(m_imaxclip2, int32_t, ("CBasePlayerWeaponVData"), ("m_iMaxClip2"));
	SCHEMA(m_idefaultclip1, int32_t, ("CBasePlayerWeaponVData"), ("m_iDefaultClip1"));
	SCHEMA(m_idefaultclip2, int32_t, ("CBasePlayerWeaponVData"), ("m_iDefaultClip2"));
	SCHEMA(m_iweight, int32_t, ("CBasePlayerWeaponVData"), ("m_iWeight"));
	SCHEMA(m_weapon_name, const char*, ("CCSWeaponBaseVData"), ("m_szName"));
	SCHEMA(m_damage, int, ("CCSWeaponBaseVData"), ("m_nDamage"));
	SCHEMA(m_armor_ratio, float, ("CCSWeaponBaseVData"), ("m_flArmorRatio"));
	SCHEMA(m_range, float, ("CCSWeaponBaseVData"), ("m_flRange"));
	SCHEMA(m_range_modifier, float, ("CCSWeaponBaseVData"), ("m_flRangeModifier"));
	SCHEMA(m_penetration, float, ("CCSWeaponBaseVData"), ("m_flPenetration"));
	SCHEMA(m_headshot_multiplier, float, ("CCSWeaponBaseVData"), ("m_flHeadshotMultiplier"));
	SCHEMA(m_spread, float, ("CCSWeaponBaseVData"), ("m_flSpread"));
	SCHEMA(m_weapon_bullets, float, ("CCSWeaponBaseVData"), ("m_nNumBullets"));
	SCHEMA(m_weapon_type, int, ("CCSWeaponBaseVData"), ("m_WeaponType"));
	SCHEMA(m_cycle_time, float, ("CCSWeaponBaseVData"), ("m_flCycleTime"));
	SCHEMA(m_scope_fov_1, int, ("CCSWeaponBaseVData"), ("m_nZoomFOV1"));
	SCHEMA(m_scope_fov_2, int, ("CCSWeaponBaseVData"), ("m_nZoomFOV2"));
	SCHEMA(m_fl_zoom_time_0, float, ("CCSWeaponBaseVData"), ("m_flZoomTime0"));
	SCHEMA(m_fl_zoom_time_1, float, ("CCSWeaponBaseVData"), ("m_flZoomTime1"));
	SCHEMA(m_fl_zoom_time_2, float, ("CCSWeaponBaseVData"), ("m_flZoomTime2"));

};

class c_econ_item_view
{
public:
	SCHEMA(m_item_definition_index, uint16_t, ("C_EconItemView"), ("m_iItemDefinitionIndex"));
	SCHEMA(m_item_id, uint64_t, ("C_EconItemView"), ("m_iItemID"));
	SCHEMA(m_item_id_low, uint32_t, ("C_EconItemView"), ("m_iItemIDLow"));
	SCHEMA(m_item_id_high, uint32_t, ("C_EconItemView"), ("m_iItemIDHigh"));
	SCHEMA(m_account_id, uint32_t, ("C_EconItemView"), ("m_iAccountID"));
	SCHEMA(m_initialized, bool, ("C_EconItemView"), ("m_bInitialized"));
	SCHEMA(m_custom_name, const char*, ("C_EconItemView"), ("m_szCustomName"));
	SCHEMA(m_custom_name_override, const char*, ("C_EconItemView"), ("m_szCustomNameOverride"));
	SCHEMA(m_bDisallowSOC, bool,"C_EconItemView","m_bDisallowSOC");

	auto GetCustomPaintKitIndex()
	{
		return g_memory->CallVFunc<int>(this, 3u);
	}

	c_econ_item_definition* get_static_data() {
		return g_memory->CallVFunc<c_econ_item_definition*>(this, 16u);
	}
};

class c_attribute_container
{
public:
	SCHEMA_ARRAY(m_item, c_econ_item_view, ("C_AttributeContainer"), ("m_Item"));
};

class c_econ_entity : public c_base_model_entity
{
public:
	SCHEMA_ARRAY(m_attribute_manager, c_attribute_container, "C_EconEntity", "m_AttributeManager");
	SCHEMA(m_fallback_paint_kit, int, ("C_EconEntity"), ("m_nFallbackPaintKit"));
	SCHEMA(m_fallback_seed, int, ("C_EconEntity"), ("m_nFallbackSeed"));
	SCHEMA(m_fallback_wear, float, ("C_EconEntity"), ("m_flFallbackWear"));
	SCHEMA(m_fallback_stat_trak, int, ("C_EconEntity"), ("m_nFallbackStatTrak"));
	SCHEMA(GetOriginalOwnerXuidLow, uint32_t, "C_EconEntity", "m_OriginalOwnerXuidLow");
	SCHEMA(GetOriginalOwnerXuidHigh, uint32_t, "C_EconEntity","m_OriginalOwnerXuidHigh");

	uint64_t GetOriginalOwnerXuid()
	{
		return ((uint64_t)(GetOriginalOwnerXuidHigh()) << 32) |
			GetOriginalOwnerXuidLow();
	}
};

class c_base_player_weapon : public c_econ_entity
{
public:
	SCHEMA(m_next_primary_attack_tick, int32_t, ("C_BasePlayerWeapon"), ("m_nNextPrimaryAttackTick"));
	SCHEMA(m_next_primary_attack_tick_ratio, float, ("C_BasePlayerWeapon"), ("m_flNextPrimaryAttackTickRatio"));
	SCHEMA(m_next_secondary_attack_tick, int32_t, ("C_BasePlayerWeapon"), ("m_nNextSecondaryAttackTick"));
	SCHEMA(m_next_secondary_attack_tick_ratio, float, ("C_BasePlayerWeapon"), ("m_flNextSecondaryAttackTickRatio"));
	SCHEMA(m_iclip1, int32_t, ("C_BasePlayerWeapon"), ("m_iClip1"));
	SCHEMA(m_iclip2, int32_t, ("C_BasePlayerWeapon"), ("m_iClip2"));
	SCHEMA(m_reserve_ammo, int32_t, ("C_BasePlayerWeapon"), ("m_pReserveAmmo"));
	SCHEMA(m_recoil_index, float, ("C_CSWeaponBase"), ("m_flRecoilIndex"));

	float get_max_speed();

	float get_inaccuracy() {
		using fn_get_inaccuracy_t = float(__fastcall*)(void*,float*,float*);
		static fn_get_inaccuracy_t fn = reinterpret_cast<fn_get_inaccuracy_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 44 0F 29 84 24"));
		if (!fn)
			return 0;
		float movement{},air{};return fn(this,&movement,&air);
	}

	float get_spread() {
		if (!c_weapon_i::get_spread)
		{
			std::cout << "fail get_spread" << std::endl;
			return 0;
		}
		return c_weapon_i::get_spread(this);
	}

	void update_accuracy_penality() {
		if (!c_weapon_i::update_accuracypenality)
		{
			std::cout << "fail updateaccuracy" << std::endl;
			return;
		}
		c_weapon_i::update_accuracypenality(this);
	}

};

class c_player_weapon_services
{
public:
	SCHEMA(m_next_attack, float, ("CCSPlayer_WeaponServices"), ("m_flNextAttack"));
	SCHEMA(m_active_weapon, c_handle<c_base_player_weapon>, ("CPlayer_WeaponServices"), ("m_hActiveWeapon"));
	SCHEMA(m_last_weapon, c_handle<c_base_player_weapon>, ("CPlayer_WeaponServices"), ("m_hLastWeapon"));
	SCHEMA(m_my_weapons, c_utl_vector<c_base_handle>, ("CPlayer_WeaponServices"), ("m_hMyWeapons"));

bool weapon_owns_this_type(const char* name);
};
	
class c_weapon_base : public c_base_player_weapon
{
public:
	SCHEMA(m_iOriginalTeamNumber, int ,"C_CSWeaponBase","m_iOriginalTeamNumber");
	SCHEMA(m_postpone_fire_ready_ticks, int32_t,"C_CSWeaponBase","m_nPostponeFireReadyTicks");
	SCHEMA(m_iWatTickOffset, float, "C_CSWeaponBase", "m_flWatTickOffset");

	bool is_knife()
	{
		auto econ_item_view = m_attribute_manager();
		if (!econ_item_view)
			return false;

		auto weapon_data = m_weapon_data();

		if (!weapon_data)
			return false;

		if (econ_item_view->m_item()->m_item_definition_index() == e_item_def_index_t::WEAPON_TASER)
			return false;

		return weapon_data->m_weapon_type() == e_weapon_type::WEAPONTYPE_KNIFE;
	}

	bool is_zeus()
	{
		auto econ_item_view = m_attribute_manager();
		if (!econ_item_view)
			return false;

		return econ_item_view->m_item()->m_item_definition_index() == WEAPONTYPE_TASER;
	}

	bool is_grenade()
	{
		auto weapon_data = m_weapon_data();

		if (!weapon_data)
			return false;

		return weapon_data->m_weapon_type() == e_weapon_type::WEAPONTYPE_GRENADE;
	}

	bool is_c4()
	{
		auto weapon_data = m_weapon_data();
		if (!weapon_data)
			return false;

		return weapon_data->m_weapon_type() == e_weapon_type::WEAPONTYPE_C4;
	}

	c_cs_weapon_base_vdata* m_weapon_data()
	{
		return static_cast<c_cs_weapon_base_vdata*>(GetVData());
	}
};

class CAnimGraphNetworkedVariables;

class CAnimationGraphInstance
{
public:
	char pad_0x0000[0x2E0]; //0x0000
	CAnimGraphNetworkedVariables* pAnimGraphNetworkedVariables; //0x02E0
};

class c_base_view_model : public c_base_entity {
public:
	SCHEMA(m_weapon, c_base_handle, "C_BaseEntity", "m_hOwnerEntity");
};

class c_csgo_viewmodel : public c_base_view_model
{
public:
	char pad_0x0000[0xD60]; //0x0000
	CAnimationGraphInstance* pAnimationGraphInstance; //0x0D08
};

class c_smoke_grenade_projectile {
public:
	SCHEMA(m_did_smoke_effect, bool, "C_SmokeGrenadeProjectile", "m_bDidSmokeEffect");
	SCHEMA(get_smoke_pos, vec3_t, "C_SmokeGrenadeProjectile", "m_vSmokeDetonationPos");
	SCHEMA(get_smoke_tick, int32_t, "C_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin");
};

class c_weapon_base_gun {
public:
	SCHEMA(m_zoom_level, int, "C_CSWeaponBaseGun", "m_zoomLevel");
};