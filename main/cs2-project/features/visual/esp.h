#pragma once
#include <memory>
#include "../../sdk/schema/class/player.h"
#include "../../dependencies/imgui/imgui_internal.h"
#include "d3d11.h"



enum e_item_definition_index : uint16_t
{
    weapon_none = 0,
    weapon_deagle,
    weapon_elite,
    weapon_fiveseven,
    weapon_glock,
    weapon_ak47 = 7,
    weapon_aug,
    weapon_awp,
    weapon_famas,
    weapon_g3sg1,
    weapon_galilar = 13,
    weapon_m249,
    weapon_m4a1 = 16,
    weapon_mac10,
    weapon_p90 = 19,
    weapon_mp5sd = 23,
    weapon_ump45,
    weapon_xm1014,
    weapon_bizon,
    weapon_mag7,
    weapon_negev,
    weapon_sawedoff,
    weapon_tec9,
    weapon_taser,
    weapon_hkp2000,
    weapon_mp7,
    weapon_mp9,
    weapon_nova,
    weapon_p250,
    weapon_shield,
    weapon_scar20,
    weapon_sg553,
    weapon_ssg08,
    weapon_knifegg,
    weapon_knife,
    weapon_flashbang,
    weapon_hegrenade,
    weapon_smokegrenade,
    weapon_molotov,
    weapon_decoy,
    weapon_incgrenade,
    weapon_c4,
    weapon_healthshot = 57,
    weapon_knife_t = 59,
    weapon_m4a1_silencer,
    weapon_usp_silencer,
    weapon_cz75a = 63,
    weapon_revolver,
    weapon_tagrenade = 68,
    weapon_fists,
    weapon_breachcharge,
    weapon_tablet = 72,
    weapon_melee = 74,
    weapon_axe,
    weapon_hammer,
    weapon_spanner = 78,
    weapon_knife_ghost = 80,
    weapon_firebomb,
    weapon_diversion,
    weapon_frag_grenade,
    weapon_snowball,
    weapon_bumpmine,
    weapon_knife_bayonet = 500,
    weapon_knife_css = 503,
    weapon_knife_flip = 505,
    weapon_knife_gut,
    weapon_knife_karambit,
    weapon_knife_m9_bayonet,
    weapon_knife_tactical,
    weapon_knife_falchion = 512,
    weapon_knife_survival_bowie = 514,
    weapon_knife_butterfly,
    weapon_knife_push,
    weapon_knife_cord,
    weapon_knife_canis,
    weapon_knife_ursus = 519,
    weapon_knife_gypsy_jackknife,
    weapon_knife_outdoor,
    weapon_knife_stiletto = 522,
    weapon_knife_widowmaker,
    weapon_knife_skeleton = 525,
    studded_brokenfang_gloves = 4725,
    glove_studded_bloodhound = 5027,
    glove_t_side,
    glove_ct_side,
    glove_sporty,
    glove_slick,
    glove_leather_wrap,
    glove_motorcycle,
    glove_specialist,
    glove_hydra,
};

class c_dropped_weapons {
public:
    c_weapon_base* weapon;
    bool is_invalid;
    c_base_handle handle;
    vec3_t abs_origin;
    vec3_t mins;
    vec3_t maxs;
    std::string weapon_name;
    int item_index;
};

class c_esp
{
private:
	struct box
	{
		float left;
		float top;
		float right;
		float bottom;
		bool found;
	};

    struct player_info_t {
        bool m_valid = false;

        int m_handle;
        int m_health;
        int m_ammo;
        int m_max_ammo;

        box m_box;

        std::string m_name;
        std::string m_weapon_name;
        c_player_controller* m_controller;
        c_player_pawn* m_pawn;
        std::vector<std::string> m_flags;
    };

    struct sound_info_t {
        vec3_t player_origin;
        bool has_changed;
    };

	class c_specs
	{
	public:
		std::string name;
        ImTextureID image;
	};



     std::unordered_map<c_player_pawn*, float> animated_health;
     std::unordered_map<int, player_info_t> m_player_map;
     std::vector<sound_info_t> sound_info;
     std::mutex m_player_mutex;
public:
    std::vector< c_dropped_weapons> m_dropped_weapons;
    std::unordered_map<uint64_t, ImTextureID> avatar_cache;
    vec3_t pos{};
    const char* GunIcon(const std::string& weapon);

    c_esp::box get_box(c_player_pawn* entity);
    ImTextureID get_avatar(uint64_t steam_id);
    void store_players();
    void spectator(c_player_controller* player_controller, c_player_pawn* player_pawn);
    void draw_esp();
    static c_esp& get_instance() {
        static c_esp instance;
        return instance;
    }
};
inline c_esp* g_esp = &c_esp::get_instance();
