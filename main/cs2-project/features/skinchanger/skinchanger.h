#pragma once
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <map>

class c_player_pawn;
class CEconItem;
class c_base_model_entity;
class c_base_view_model;
class ID3D11ShaderResourceView;

struct gui_skins {
	int item_def;
	std::string paint_name;
	std::string name;
	uint64_t skin_key;
	int64_t paint_id;
	int rarity;
	ID3D11ShaderResourceView* texture_id;
	int type;
	std::string path_image;
	bool is_initilization;
	int paint_seed;
	int paint_wear;
	int stattrak;
	std::array<int, 5> stickers;
	std::string custom_name;
	std::uint64_t id;

	bool operator==(uint64_t other_id) const {
		return id == other_id;
	}
};

enum type_item
{
	type_knife,
	type_weapons,
	type_agent,
	type_glove,
	type_sticker
};

class c_skins {
private:
	using fninvalidate_glove_material = void(__fastcall*)(void*, uintptr_t);
	inline static fninvalidate_glove_material fnglove_material;
	uint32_t last_model_hash = 0;
public:
    void UpdateKnife(int stage, c_player_pawn* player);
	void setup();
	void invalidate_glove_material(c_base_view_model* viewmodel);
	void update_glove(int stage, c_player_pawn* pLocalPawn);
    void dump_skins();
	void update_agent();
	void equip_loadout(int team, int slot, uint64_t itemID);
    void frame_stage(int frameStage);
	void on_set_model(c_base_model_entity* pEntity, const char*& model);
	void AddEconItemToList(CEconItem* pItem);
    void Shutdown();
	void remove_item(uint64_t item_id);
	void create_image();
 

	enum material_magic_number : uint32_t
	{
		material_magic_number__gloves = 0xf143b82a,
		material_magic_number__unknown1 = 0x1b52829c,
		material_magic_number__unknown2 = 0xa6ebe9b9,
		material_magic_number__unknown3 = 0x423b2ed4,
		material_magic_number__unknown4 = 0xc8d7255e
	};

	struct DumpedSkin_t
	{
		std::string m_name = "";
		int m_ID = 0;
		int m_rarity = 0;
	};

	struct DumpedItem_t
	{
		std::string m_name = "";
		uint16_t m_defIdx = 0;
		int m_rarity = 0;
		void* m_image = nullptr;
		bool m_unusualItem = false;
		std::vector<DumpedSkin_t> m_dumpedSkins{};
		DumpedSkin_t* pSelectedSkin = nullptr;
	};

	inline static std::vector<uint64_t> g_vecAddedItemsIDs;
	inline static std::vector<DumpedItem_t> vecDumpedItems;

	std::vector<gui_skins> skin_preview;
	std::vector<gui_skins> skin_added;	
	std::vector<gui_skins> item_default;
	std::vector<gui_skins> item_knife_default;
	std::vector<gui_skins> item_stickers;
	std::vector<gui_skins> item_agent;
	std::vector<gui_skins> item_glove;
	std::map<std::uint64_t, std::vector<gui_skins>> item_weapons;

	inline static DumpedItem_t* pSelectedItem;
    bool force_update = false;
	std::uint64_t hash_agent;

};

inline std::unique_ptr<c_skins> g_skins = std::make_unique<c_skins>();
