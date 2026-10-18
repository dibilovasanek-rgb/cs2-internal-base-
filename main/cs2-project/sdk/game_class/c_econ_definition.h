#pragma once
#include "../../memory/memory.h"

class c_econ_item_definition {
public:
	bool is_weapon() {
		return get_stickers_supported_count() >= 4;
	}

	bool is_agent() {
		static auto type_custom_player = fnv1a::hash("#Type_CustomPlayer");
		if (fnv1a::hash(m_item_type_name ? m_item_type_name : "") != type_custom_player)
			return false;

		return get_stickers_supported_count() >= 1;
	}

	bool is_knife(bool exclude_default) {
		static auto csgo_type_knife = fnv1a::hash("#CSGO_Type_Knife");
		if (fnv1a::hash(m_item_type_name ? m_item_type_name : "") != csgo_type_knife)
			return false;

		return exclude_default ? m_definition_index >= 500 : true;
	}

	bool is_glove(bool exclude_default) {
		static auto type_hands = fnv1a::hash("#Type_Hands");
		if (fnv1a::hash(m_item_type_name ? m_item_type_name : "") != type_hands)
			return false;

		const bool default_glove = m_definition_index == 5028 || m_definition_index == 5029;
		return exclude_default ? !default_glove : true;
	}

	bool is_custom_player()
	{
		auto CSGO_Type_Agents = fnv1a::hash("#Type_CustomPlayer");

		if (fnv1a::hash(this->m_item_type_name) != CSGO_Type_Agents)
			return false;

		if (m_definition_index >= 5036 && m_definition_index < 5106)
			return false;

		if (m_definition_index >= 5200 && m_definition_index <= 5205)
			return false;

		if (m_definition_index >= 5300 && m_definition_index <= 5305)
			return false;

		if (m_definition_index == 5600)
			return false;

		return true;
	}
	bool is_weapon_case() {
		static auto type_weapon_case = fnv1a::hash("#CSGO_Type_WeaponCase");
		return fnv1a::hash(m_item_type_name ? m_item_type_name : "") == type_weapon_case;
	}

	bool is_key() {
		static auto tool_weapon_case_key_tag = fnv1a::hash("#CSGO_Tool_WeaponCase_KeyTag");
		return fnv1a::hash(m_item_type_name ? m_item_type_name : "") == tool_weapon_case_key_tag;
	}

	const char* get_model_name() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x148);
	}

	int get_stickers_supported_count() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x168);
	}

	const char* get_simple_weapon_name() {
		//return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x210);

		if (!m_inventory_image) return "";
        const char* slash=std::strrchr(m_inventory_image,'/');return slash ? slash+1 : m_inventory_image;
	}

	int get_loadout_slot() {
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x338);
	}
public:

	//char pad0[0x8]; // vtable
	//void* m_pKVItem;
	//uint16_t m_nDefIndex;
	//CUtlVector<uint16_t> m_nAssociatedItemsDefIndexes;
	//bool m_bEnabled;
	//const char* m_szPrefab;
	//uint8_t m_unMinItemLevel;
	//uint8_t m_unMaxItemLevel;
	//uint8_t m_nItemRarity;
	//uint8_t m_nItemQuality;
	//uint8_t m_nForcedItemQuality;
	//uint8_t m_nDefaultDropItemQuality;
	//uint8_t m_nDefaultDropQuantity;
	//CUtlVector<void*> m_vecStaticAttributes;
	//uint8_t m_nPopularitySeed;
	//void* m_pPortraitsKV;
	//const char* m_pszItemBaseName;
	//bool m_bProperName;
	//const char* m_pszItemTypeName;
	//uint32_t m_unItemTypeID;
	//const char* m_pszItemDesc;

	std::byte pad_001[0x8];  // vtable
	void* m_kv_item; // 0x8
	std::uint16_t m_definition_index; // 0x10
	std::byte pad_002[0x1e];
	bool m_enabled; // 0x30
	std::byte pad_003[0xf];
	std::uint8_t m_min_item_level; // 0x40
	std::uint8_t m_max_item_level; // 0x41
	std::uint8_t m_item_rarity; // 0x42
	std::uint8_t m_item_quality; // 0x43
	std::uint8_t m_forced_item_quality; // 0x44
	std::uint8_t m_default_drop_item_quality; // 0x45
	std::uint8_t m_default_drop_quantity; // 0x46
	std::byte pad_004[0x19];
	std::uint8_t m_popularity_seed; // 0x60
	std::byte pad_005[0x7];
	void* m_portraits_kv; // 0x68
	char* m_item_base_name; // 0x70
	bool m_proper_name; // 0x78
	std::byte pad_006[0x7];
	char* m_item_type_name; // 0x80
	std::byte pad_007[0x8];
	char* m_item_desc; // 0x90
	std::uint32_t m_expiration_time_stamp; // 0x98
	std::uint32_t m_creation_time_stamp; // 0x9c
	char* m_inventory_model; // 0xa0
	char* m_inventory_image; // 0xa8
	std::byte pad_008[0x18];
	int m_inventory_image_position[2]; // 0xc8
	int m_inventory_image_size[2]; // 0xd0

};

struct c_alternate_icon_data
{
	const char* m_simple_name;
	const char* m_large_simple_name;
	std::byte pad_001[0x10];
};


class c_paint_kit
{
public:
	char pad_0x0000[0xE0]; //0x0000

	int32_t paint_kit_id()
	{
		return *reinterpret_cast<int32_t*>((uintptr_t)(this));
	}

	const char* paint_kit_name()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x8);
	}

	const char* paint_kit_description_string()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x10);
	}

	const char* paint_kit_description_tag()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x18);
	}

	int32_t paint_kit_rarity()
	{
		return *reinterpret_cast<int32_t*>((uintptr_t)(this) + 0x44);
	}

	bool uses_old_model()
	{
		return *reinterpret_cast<bool*>((uintptr_t)(this) + 0xB2);
	}
};

class c_sticker_kit
{
public:
	int m_id;
	int m_rarity;
	const char* m_name;
	const char* m_description;
	const char* m_item_name;
	const char* m_material_path;
	const char* m_material_path_no_drips;
	const char* m_inventory_image;
	int m_tournament_id;
	int m_tournament_team_id;
	int m_tournament_player_id;
	bool m_material_path_is_absolute;
	bool m_cannot_trade;
	std::byte pad_001[0x2];
	float m_rotate_start;
	float m_rotate_end;
	float m_scale_min;
	float m_scale_max;
	float m_wear_min;
	float m_wear_max;
	const char* m_icon_url_small;
	const char* m_icon_url_large;
	void* m_kv_item;
};

inline constexpr std::uint64_t get_alternate_icon_key_for_weapon_paint_wear_item(std::uint16_t def_idx, std::uint32_t paint_id, std::uint32_t wear)
{
	return (def_idx << 16) + (paint_id << 2) + wear;
}