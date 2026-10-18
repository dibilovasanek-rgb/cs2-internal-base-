#pragma once
#include <cstdint>
#include "../../sdk/schema/class/player.h"
#include "../../sdk/schema/class/weapon.h"

class c_autowall
{
public:
	enum class HitGroup : std::uint32_t {
		INVALID = -1,
		GENERIC = 0,
		HEAD = 1,
		CHEST = 2,
		STOMACH = 3,
		LEFTARM = 4,
		RIGHTARM = 5,
		LEFTLEG = 6,
		RIGHTLEG = 7,
		NECK = 8,
		UNUSED = 9,
		GEAR = 10,
		SPECIAL = 11,
		COUNT = 12
	};

	struct BulletData {
		float damage;
		float penetration;
		float range_modifier;
		float range;
		int penetration_count;
		bool failed;

		BulletData(float dmg, float pen, float range_mod, float rng, int pen_count) noexcept
			: damage(dmg)
			, penetration(pen)
			, range_modifier(range_mod)
			, range(rng)
			, penetration_count(pen_count)
			, failed(false) {
		}
	};

	float penetration_damage(const vec3_t& local_pos, const vec3_t& target_pos, c_base_entity* local, c_player_pawn* local_pawn, c_base_entity* target, c_player_pawn* target_pawn, bool has_heavy_armor, int target_team, c_cs_weapon_base_vdata* weapon_data);
	void scale_damage(const int hitGroup, c_player_pawn* player, const float armorRatio, const float headshotMultiplier, float* damage, bool hasHeavyArmor, int team);
	
	static c_autowall& get_instance() {
		static c_autowall instance;
		return instance;
	}
};
inline c_autowall* g_autowall = &c_autowall::get_instance();
