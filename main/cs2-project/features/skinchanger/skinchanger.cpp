#include "skinchanger.h"
#include "../../sdk/sdk.h"
#include "../../dependencies/config/Include/config.h"
#include "../../sdk/schema/class//weapon.h"
#include "../../sdk/game_class/c_utl_string_token.h"
#include "../../sdk/game_class/c_econ_definition.h"
#include "../../sdk/game_class/interfaces/i_client.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../sdk/game_class/game_enums.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "..\..\sdk\game_class\interfaces\i_resource_system.h"
#include "../../entry.h"
#include "../../render/render.h"
#include "../../sdk/game_class/interfaces/player_inventory.h"
#include "../../dependencies/vtex_parser/vtex_parser.h"
#include "../../dependencies/tinyformat.h"
#include "../../core/hooks/hooks.h"
#include "..\..\sdk\game_class\interfaces\c_base_file_system.h"
#include "../visual/game_visuals/model_changer.h"

void c_skins::setup() {}
void c_skins::invalidate_glove_material(c_base_view_model*) { FrlsRefreshGloveAnimations(); }

void c_skins::dump_skins()
{
	if (vecDumpedItems.empty() /*&& skin_changer::dumpitems*/)
	{
		auto* system = g_interface->m_client->get_econ_item_system();
        auto* pItemSchema = system ? system->get_econ_item_schema() : nullptr;
        if (!pItemSchema) return;
		const c_utl_map<int, c_econ_item_definition*>& vecItems =
			pItemSchema->get_sorted_item_definition_map();
		c_utl_map<int, c_paint_kit*>& vecPaintKits =
			pItemSchema->get_paint_kits();
		const c_utl_map<uint64_t, c_alternate_icon_data>& vecAlternateIcons =
			pItemSchema->get_alternate_icons_map();
        if (!vecItems.is_valid() || !vecPaintKits.is_valid() || !vecAlternateIcons.is_valid()) {
            std::cout << "Econ schema containers are not ready\n"; return;
        }

		for (const auto& it : vecItems)
		{
			c_econ_item_definition* pItem = it.m_value;
			if (!pItem)
				continue;

			const bool isWeapon = pItem->is_weapon();
			const bool isKnife = pItem->is_knife(true);
			const bool isGloves = pItem->is_glove(true);
			const bool IsAgent = pItem->is_agent();

			const char* itemBaseName = pItem->m_item_base_name;

			if (!itemBaseName || itemBaseName[0] == '\0')
				continue;

			const uint16_t defIdx = pItem->m_definition_index;

			DumpedItem_t dumpedItem{};
			const auto* localized_item=g_interface->m_localize->find_key(pItem->m_item_base_name);
            dumpedItem.m_name = localized_item ? localized_item : pItem->m_item_base_name;

			//dumpedItem.m_image = pItem->m_pKVItem;
			dumpedItem.m_defIdx = defIdx;
			dumpedItem.m_rarity = pItem->m_item_rarity;
			if (isKnife || isGloves || isWeapon || IsAgent)
			{
				dumpedItem.m_unusualItem = true;
			}
			dumpedItem.m_dumpedSkins.emplace_back("Vanilla", 0, IR_ANCIENT);
			// Load the image and set the texture ID.
			//if (dumpedItem.m_image)
			//{
			//	dumpedItem.m_textureID = CreateTextureFromMemory(dumpedItem.m_image, 120, 280);
			//}

			// We filter skins by guns.
			for (const auto& it : vecPaintKits)
			{
				c_paint_kit* pPaintKit = it.m_value;
				if (!pPaintKit || pPaintKit->paint_kit_id() == 0 || pPaintKit->paint_kit_id() == 9001)
					continue;

				const uint64_t skinKey =
					get_alternate_icon_key_for_weapon_paint_wear_item(
						defIdx, pPaintKit->paint_kit_id(), 0);
				if (vecAlternateIcons.find_by_key(skinKey))
				{
					DumpedSkin_t dumpedSkin{};
					const auto* tag=pPaintKit->paint_kit_description_tag();
                    const auto* localized=tag ? g_interface->m_localize->find_key(tag) : nullptr;
                    const auto* paint_name=pPaintKit->paint_kit_name();
                    dumpedSkin.m_name=localized ? localized : (paint_name ? paint_name : "Unnamed");
					dumpedSkin.m_ID = pPaintKit->paint_kit_id();
					dumpedSkin.m_rarity = std::clamp(pItem->m_item_rarity + pPaintKit->paint_kit_rarity() - 1, 0, (pPaintKit->paint_kit_rarity() == 7) ? 7 : 6); // fixed by protected
					dumpedItem.m_dumpedSkins.emplace_back(dumpedSkin);
				}
			}

			//Sort skins by rarity.
			if (!dumpedItem.m_dumpedSkins.empty() && isWeapon)
			{
				std::sort(dumpedItem.m_dumpedSkins.begin(),
					dumpedItem.m_dumpedSkins.end(),
					[](const DumpedSkin_t& a, const DumpedSkin_t& b)
					{
						return a.m_rarity > b.m_rarity;
					});
			}

			vecDumpedItems.emplace_back(std::move(dumpedItem));
		}
        std::cout << "Econ catalogue: " << vecDumpedItems.size() << " items, " << vecPaintKits.m_size << " paint kits\n";
	}
}

void c_skins::update_agent()
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	if (!sdk::m_local_pawn)
		return;

	if (settings.cfg().visuals.b_modelchanger)
	{
		if (g_modelchanger->nSelectedPlayerModel == ~1U)
			return;

		if (g_modelchanger->vecPlayerModels.size() <= g_modelchanger->nSelectedPlayerModel)
			return;

		Model_t pModel = g_modelchanger->vecPlayerModels[g_modelchanger->nSelectedPlayerModel];
		const char* szModelPath = pModel.strModelPath.c_str();

		auto uPlayerModelHash = fnv1a::hash(szModelPath);
		if (g_modelchanger->uLastPlayerModelHash == uPlayerModelHash)
			return;

		g_modelchanger->uLastPlayerModelHash = uPlayerModelHash;

		g_interface->m_resource_system->PreCache1(szModelPath);
		sdk::m_local_pawn->set_model(szModelPath);
		return;
	}

	c_econ_item_definition* item_view_loadout = nullptr;
	for (int i = 0; i <= 56; ++i) {
		c_econ_item_view* item_view_ = pInventory->GetItemInLoadout(sdk::m_local_pawn->m_team_num(), i);
		if (!item_view_)
			continue;

		auto p = item_view_->get_static_data();

		if (p && p->is_custom_player())
		{
			item_view_loadout = p;
			break;
		}
	}

	if (!item_view_loadout)
		return;

	if (fnv1a::hash(settings.cfg().skins.custom_agent != 0 ? item_view_loadout->get_model_name() : g_entry->models_agents[settings.cfg().skins.custom_agent].c_str()) == hash_agent)
		return;

	hash_agent = fnv1a::hash(settings.cfg().skins.custom_agent != 0 ? item_view_loadout->get_model_name() : g_entry->models_agents[settings.cfg().skins.custom_agent].c_str());

	if (settings.cfg().skins.custom_agent == 0) {
		sdk::m_local_pawn->set_model(item_view_loadout->get_model_name());
	}
	else {
		if (!g_entry->models_agents.empty()) {
			g_interface->m_resource_system->PreCache1(g_entry->models_agents[settings.cfg().skins.custom_agent].c_str());
			sdk::m_local_pawn->set_model(g_entry->models_agents[settings.cfg().skins.custom_agent].c_str());
		}
	}
}

void c_skins::UpdateKnife(int stage, c_player_pawn* player)
{
	if (stage != 6)
		return;

    if (settings.cfg().visuals.b_knifechanger) {
        g_modelchanger->SetWeaponModel(player);
        return;
    }

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	const auto steam_id = pInventory->get_owner().m_id;
	auto pViewModelServices = player->m_view_model_services();
	if (!pViewModelServices)
		return;

	c_csgo_viewmodel* pViewModel = reinterpret_cast<c_csgo_viewmodel*>(
		g_interface->m_game_resource_service->get_game_entity_system()->get(
			pViewModelServices->m_view_model().get_entry_index()));
	if (!pViewModel)
		return;

	const int highestIndex = g_interface->m_game_resource_service->get_game_entity_system()->get_highest_entity_index();

	for (int i = 64 + 1; i <= highestIndex; ++i)
	{
		c_base_entity* pEntity = g_interface->m_game_resource_service->get_game_entity_system()->get<c_base_entity>(i);
		if (!pEntity || !pEntity->is_weapon())
			continue;

		c_weapon_base* pWeapon = reinterpret_cast<c_weapon_base*>(pEntity);
        if (!pWeapon->m_weapon_data()) continue;

		if (pWeapon->m_weapon_data()->m_weapon_type() == WEAPON_FLASHBANG ||
			pWeapon->m_weapon_data()->m_weapon_type() == WEAPONTYPE_GRENADE)
			continue;

		if (pWeapon->GetOriginalOwnerXuid() != steam_id)
			continue;

		if (pWeapon->m_weapon_data() && pWeapon->m_weapon_data()->m_weapon_type() != WEAPONTYPE_KNIFE)
			continue;

		c_attribute_container* pAttributeContainer = pWeapon->m_attribute_manager();
		if (!pAttributeContainer)
			continue;

		c_econ_item_view* pWeaponItemView = pAttributeContainer->m_item();
		if (!pWeaponItemView)
			continue;

		c_econ_item_definition* pWeaponDefinition = pWeaponItemView->get_static_data();
		if (!pWeaponDefinition)
			continue;

		c_econ_item_view* pWeaponInLoadoutItemView = nullptr;

		if (pWeaponDefinition->get_stickers_supported_count() >= 4)
		{
			for (int j = 0; j <= 56; ++j)
			{
				c_econ_item_view* pItemView = pInventory->GetItemInLoadout(pWeapon->m_iOriginalTeamNumber(), j);
				if (!pItemView)
					continue;

				if (pItemView->m_item_definition_index() == pWeaponDefinition->m_definition_index)
				{
					pWeaponInLoadoutItemView = pItemView;
					break;
				}
			}
		}
		else
		{
			pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(pWeapon->m_iOriginalTeamNumber(), pWeaponDefinition->get_loadout_slot());
		}

		if (!pWeaponInLoadoutItemView)
			continue;

		auto it = std::find(skin_added.cbegin(), skin_added.cend(), pWeaponInLoadoutItemView->m_item_id());
		if (it == skin_added.cend())
			continue;

		c_econ_item_definition* pWeaponInLoadoutDefinition = pWeaponInLoadoutItemView->get_static_data();
		if (!pWeaponInLoadoutDefinition)
			continue;

		if (!pWeaponInLoadoutDefinition->is_knife(false) &&
			pWeaponInLoadoutDefinition->m_definition_index != pWeaponDefinition->m_definition_index)
			continue;

		pWeaponItemView->m_bDisallowSOC() = false;
		pWeaponItemView->m_item_id() = pWeaponInLoadoutItemView->m_item_id();
		pWeaponItemView->m_item_id_high() = pWeaponInLoadoutItemView->m_item_id_high();
		pWeaponItemView->m_item_id_low() = pWeaponInLoadoutItemView->m_item_id_low();
		pWeaponItemView->m_account_id() = uint32_t(steam_id);
		pWeaponItemView->m_item_definition_index() = pWeaponInLoadoutDefinition->m_definition_index;

		std::uint32_t hash_ = CUtlStringToken1(std::to_string(pWeaponInLoadoutDefinition->m_definition_index).c_str()).GetHashCode1();
		pWeapon->m_nSubclassID() = hash_;
		pWeapon->UpdateSubClass();

		pWeapon->m_weapon_data()->m_weapon_name() = pWeaponInLoadoutDefinition->get_simple_weapon_name();
		

		const char* knifeModel = pWeaponInLoadoutDefinition->get_model_name();
		pWeapon->set_model(knifeModel);
		c_base_handle hWeapon = pWeapon->get_ref_ehandle();

		if (pViewModel && (pViewModel->m_weapon().get_entry_index() == hWeapon.get_entry_index()))
		{
			pViewModel->set_model(knifeModel);

		}
	}
}

void c_skins::update_glove(int stage, c_player_pawn* pLocalPawn)
{
	if (stage != 6)
		return;
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	if (!pLocalPawn)
		return;

	if (!pLocalPawn->m_is_alive())
		return;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	auto steamID = pInventory->get_owner().m_id;
	c_csgo_viewmodel* pViewModel = (c_csgo_viewmodel*)g_interface->m_game_resource_service->get_game_entity_system()->get(pLocalPawn->m_view_model_services()->m_view_model().get_entry_index());

	c_econ_item_view* pGloveItemView = &pLocalPawn->m_EconGloves();
	if (!pGloveItemView)
		return;

	auto pItemViewLoadout = pInventory->GetItemInLoadout(pLocalPawn->m_team_num(), 41);
	if (!pItemViewLoadout)
		return;

	auto pGloveDefinition = pGloveItemView->get_static_data();
	if (!pGloveDefinition)
		return;

	static std::uint8_t uUpdateFrames = 0U;
	/* Detect if we have to change gloves */
	if (pGloveItemView->m_item_id() != pItemViewLoadout->m_item_id() || force_update)
	{
		uUpdateFrames = 3U;
		pGloveItemView->m_item_definition_index() = pItemViewLoadout->m_item_definition_index();
		pGloveItemView->m_item_id() = pItemViewLoadout->m_item_id();
		pGloveItemView->m_item_id_high() = pItemViewLoadout->m_item_id_high();
		pGloveItemView->m_item_id_low() = pItemViewLoadout->m_item_id_low();
		pGloveItemView->m_account_id() = pItemViewLoadout->m_account_id();
		pGloveItemView->m_bDisallowSOC() = false;
		force_update = false;
	}
	/* Detect if we have to change pGloveItemView */
	if (uUpdateFrames)
	{
		invalidate_glove_material(pViewModel);

		pGloveItemView->m_initialized() = true;
		pLocalPawn->m_need_to_reapply_gloves() = true;

		uUpdateFrames--;
	}
}

void c_skins::equip_loadout(int team, int slot, uint64_t itemID)
{
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	auto it = std::find(skin_added.cbegin(), skin_added.cend(), itemID);
	if (it == skin_added.cend()) return;

	auto pInventoryManager = CCSPlayerInventory::GetInstance();
	if (!pInventoryManager)
		return;

	c_econ_item_view* pItemViewToEquip = pInventoryManager->GetItemViewForItem(itemID);
	if (!pItemViewToEquip) return;

	c_econ_item_view* pItemInLoadout = pInventoryManager->GetItemInLoadout(team, slot);
	if (!pItemInLoadout) return;

	c_econ_item_definition* pItemInLoadoutStaticData = pItemInLoadout->get_static_data();
	if (!pItemInLoadoutStaticData) return;


	const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->m_item_definition_index();

	CCSInventoryManager* s = CCSInventoryManager::GetInstance();
	if (!s)
		return;

	s->EquipItemInLoadout(team, slot, defaultItemID);

	CEconItem* pItemInLoadoutSOCData = pInventoryManager->GetSOCDataForItem(pItemInLoadout->m_item_id());
	if (!pItemInLoadoutSOCData) return;

	pInventoryManager->SOUpdated(pInventoryManager->get_owner(),
		(CSharedObject*)pItemInLoadoutSOCData,
		eSOCacheEvent_Incremental);
}

void c_skins::frame_stage(int frameStage)
{
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	if (frameStage != FRAME_NET_UPDATE_POSTDATAUPDATE_END && frameStage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	c_game_entity_system* pEntitySystem = g_interface->m_game_resource_service->get_game_entity_system();
	if (!pEntitySystem)
		return;

	const uint64_t steamID = pInventory->get_owner().m_id;

	c_player_pawn* pLocalPawn = sdk::m_local_pawn;
	if (!pLocalPawn || !pLocalPawn->m_is_alive())
		return;

	c_cs_player_view_model_service* pViewModelServices = pLocalPawn->m_view_model_services();
	if (!pViewModelServices || (uintptr_t)pViewModelServices == 0xffffffffffffffff)
		return;

	c_csgo_viewmodel* pViewModel = g_interface->m_game_resource_service->get_game_entity_system()->get<c_csgo_viewmodel>(pViewModelServices->m_view_model());
	if (!pViewModel)
		return;

	UpdateKnife(frameStage, pLocalPawn);
	update_glove(frameStage, pLocalPawn);
	update_agent();
	int highestIndex = pEntitySystem->get_highest_entity_index();
	for (int i = 64 + 1; i <= highestIndex; ++i)
	{
		c_base_entity* pEntity = pEntitySystem->get(i);
		if (!pEntity || !pEntity->is_weapon())
			continue;
		c_weapon_base* pWeapon{};

		pWeapon = reinterpret_cast<c_weapon_base*>(pEntity);
        if (!pWeapon->m_weapon_data()) continue;
		if (pWeapon->GetOriginalOwnerXuid() != steamID)
			continue;
        if (settings.cfg().visuals.b_knifechanger && pWeapon->is_knife()) continue;

		if (pWeapon->m_weapon_data()->m_weapon_type() == WEAPON_FLASHBANG || pWeapon->m_weapon_data()->m_weapon_type() == WEAPONTYPE_GRENADE || pWeapon->m_weapon_data()->m_weapon_type() == WEAPONTYPE_C4)
			continue;

		c_attribute_container* pAttributeContainer = pWeapon->m_attribute_manager();
		if (!pAttributeContainer)
			continue;

		c_econ_item_view* pWeaponItemView = pAttributeContainer->m_item();
		if (!pWeaponItemView)
			continue;

		c_econ_item_definition* pWeaponDefinition = pWeaponItemView->get_static_data();
		if (!pWeaponDefinition)
			continue;

		c_game_scene_node* pWeaponSceneNode = pWeapon->m_game_scene_node();
		if (!pWeaponSceneNode)
			continue;

		// No idea how to check this faster with the new loadout system.
		c_econ_item_view* pWeaponInLoadoutItemView = nullptr;

		if (pWeaponDefinition->is_weapon())
		{
			for (int i = 0; i <= 56; ++i)
			{
				c_econ_item_view* pItemView = pInventory->GetItemInLoadout(pWeapon->m_iOriginalTeamNumber(), i);
				if (!pItemView)
					continue;

				if (pItemView->m_item_definition_index() == pWeaponDefinition->m_definition_index)
				{
					pWeaponInLoadoutItemView = pItemView;
					break;
				}
			}
		}
		else
		{
			pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
				pWeapon->m_iOriginalTeamNumber(),
				pWeaponDefinition->get_loadout_slot());
		}

		if (!pWeaponInLoadoutItemView)
			continue;

		// Check if skin is added by us.
		auto it = std::find(skin_added.cbegin(), skin_added.cend(), pWeaponInLoadoutItemView->m_item_id());
		if (it == skin_added.cend())
			continue;

		c_econ_item_definition* pWeaponInLoadoutDefinition = pWeaponInLoadoutItemView->get_static_data();
		if (!pWeaponInLoadoutDefinition)
			continue;

		// Example: Will not equip FiveSeven skin on CZ. Not applies for knives.
		const bool isKnife = pWeaponInLoadoutDefinition->is_knife(false);
		pWeaponItemView->m_bDisallowSOC() = false;
		pWeaponInLoadoutItemView->m_bDisallowSOC() = false;

		pWeaponItemView->m_item_id() = pWeaponInLoadoutItemView->m_item_id();
		pWeaponItemView->m_item_id_high() = pWeaponInLoadoutItemView->m_item_id_high();
		pWeaponItemView->m_item_id_low() = pWeaponInLoadoutItemView->m_item_id_low();
		pWeaponItemView->m_account_id() = uint32_t(pInventory->get_owner().m_id);
		pWeaponItemView->m_item_definition_index() = pWeaponInLoadoutDefinition->m_definition_index;

		// pWeaponItemView->m_bIsStoreItem() = true;
		//pWeaponItemView->m_bIsTradeItem() = true;

		// Displays nametag and stattrak on the gun.
		// Found by: https://www.unknowncheats.me/forum/members/2377851.html
		/*   if (!pWeapon->m_bUIWeapon()) {
			pWeapon->AddStattrakEntity();
			pWeapon->AddNametagEntity();
		}*/
		c_base_handle hWeapon = pWeapon->get_ref_ehandle();
		if (isKnife)
		{
			if (pViewModel && (pViewModel->m_weapon().get_entry_index() == hWeapon.get_entry_index() || pViewModel->m_weapon() == hWeapon))
			{
				if (pWeapon->m_weapon_data()->m_weapon_type() == 9 or pWeapon->m_weapon_data()->m_weapon_type() == 7)
					continue;
				pWeaponItemView->m_item_definition_index() = pWeaponInLoadoutDefinition->m_definition_index;

				const char* knifeModel = pWeaponInLoadoutDefinition->get_model_name();

				std::uint32_t Hash = CUtlStringToken1(std::to_string(pWeaponInLoadoutDefinition->m_definition_index).c_str()).GetHashCode1();
				pWeapon->m_nSubclassID() = Hash;
				pWeapon->UpdateSubClass();
				pWeapon->m_weapon_data()->m_weapon_name() = pWeaponInLoadoutDefinition->get_simple_weapon_name();
				

				c_game_scene_node* pViewModelSceneNode = pViewModel->m_game_scene_node();
				if (pViewModelSceneNode)
				{
					pWeaponSceneNode->set_mesh_group_mask(2);
					pViewModelSceneNode->set_mesh_group_mask(2);
				}
				pWeapon->set_model(knifeModel);
				pViewModel->set_model(knifeModel);

			}
		}
		else
		{
			// Use legacy weapon models only for skins that require them.
			// Probably need to cache this if you really care that much about
			// performance
			//const char* model = pWeaponInLoadoutDefinition->get_model_name();
			//pWeapon->set_model(model);
			//if (pViewModel && pViewModel->m_weapon() == hWeapon)
			//{
			//	pViewModel->set_model(model);
			//}

			c_econ_item_system* EconItemSystem = g_interface->m_client->get_econ_item_system();
			c_econ_item_schema* pItemSchema = EconItemSystem->get_econ_item_schema();

			auto paint_kit = pItemSchema->get_paint_kits().find_by_key(pWeaponInLoadoutItemView->GetCustomPaintKitIndex());
			bool uses_old_model = paint_kit.has_value() && paint_kit.value()->uses_old_model();

			pWeaponSceneNode->set_mesh_group_mask(1 + uses_old_model);
			if (pViewModel && pViewModel->m_weapon().get_entry_index() == hWeapon.get_entry_index())
			{
				c_game_scene_node* pViewModelSceneNode = pViewModel->m_game_scene_node();

				pViewModelSceneNode->set_mesh_group_mask(1 + uses_old_model);
			}
		}
	}
}

void c_skins::on_set_model(c_base_model_entity* pEntity, const char*& model)
{
	// When you're lagging you may see the default knife for one second and this
	// function fixes that.
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	c_player_pawn* pLocalPawn = sdk::m_local_pawn;
	if (!pLocalPawn)
		return;

	if (!pLocalPawn->m_is_alive())
		return;

	if (!pEntity)
		return;

	c_base_view_model* pViewModel = (c_base_view_model*)pEntity;
	if (!pViewModel)
		return;
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	const uint64_t steamID = pInventory->get_owner().m_id;

	c_weapon_base* pWeapon = g_interface->m_game_resource_service->get_game_entity_system()->get<c_weapon_base>(pViewModel->m_weapon());
	if (!pWeapon || !pWeapon->is_weapon())
		return;

	if (!pWeapon->is_weapon() ||
		pWeapon->GetOriginalOwnerXuid() != steamID)
		return;

	if (pWeapon->m_weapon_data()->m_weapon_type() == WEAPONTYPE_KNIFE == WEAPONTYPE_C4 == WEAPONTYPE_GRENADE)
		return;

	c_attribute_container* pAttributeContainer = pWeapon->m_attribute_manager();
	if (!pAttributeContainer)
		return;

	c_econ_item_view* pWeaponItemView = pAttributeContainer->m_item();
	if (!pWeaponItemView)
		return;

	c_econ_item_definition* pWeaponDefinition = pWeaponItemView->get_static_data();
	if (!pWeaponDefinition)
		return;

	c_econ_item_view* pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
		pWeapon->m_iOriginalTeamNumber(), pWeaponDefinition->get_loadout_slot());
	if (!pWeaponInLoadoutItemView)
		return;

	// Check if skin is added by us.
	auto it = std::find(skin_added.cbegin(), skin_added.cend(),
		pWeaponInLoadoutItemView->m_item_id());
	if (it == skin_added.cend())
		return;

	if (pWeapon->m_weapon_data()->m_weapon_type() != WEAPONTYPE_KNIFE)
		return;
	c_econ_item_definition* pWeaponInLoadoutDefinition =
		pWeaponInLoadoutItemView->get_static_data();

	if (!pWeaponInLoadoutDefinition) // ||
		return;

	pWeapon->UpdateSubClass();

	model = pWeaponInLoadoutDefinition->get_model_name();
}

void c_skins::Shutdown()
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	for (const auto& skin : g_skins->skin_added)
	{
		remove_item(skin.id);
	}
	g_skins->skin_added.clear();
}

void c_skins::remove_item(uint64_t item_id) {

	auto& skins = g_skins->skin_added;
	skins.erase(
		std::remove_if(skins.begin(), skins.end(),
			[item_id](const gui_skins& skin) { return skin.id == item_id; }),
		skins.end()
	);

	if (auto pInventory = CCSPlayerInventory::GetInstance()) {
		if (auto pItem = pInventory->GetSOCDataForItem(item_id)) {
			pInventory->RemoveEconItem(pItem);
		}
	}

}

void c_skins::create_image()
{
	auto* econ_item_system = g_interface->m_client->get_econ_item_system()->get_econ_item_schema();

	auto& items = econ_item_system->get_sorted_item_definition_map();
	auto& paint_kits = econ_item_system->get_paint_kits();
	auto& skin_image_path = econ_item_system->get_alternate_icons_map();
	auto& sticker_kits = econ_item_system->m_sticker_kits;

	for (const auto& sticker : sticker_kits)
	{
		if (!strcmp(sticker.m_value->m_item_name, "#StickerKit_Default"))
			continue;

		std::string path = tfm::format("panorama/images/%s_png.vtex_c", sticker.m_value->m_inventory_image);
		std::string name = g_interface->m_localize->find_key(sticker.m_value->m_item_name);

		skin_preview.emplace_back(gui_skins{
			.item_def = sticker.m_value->m_id,
			.paint_name = "",
			.name = name,
			.skin_key = 0,
			.paint_id = -1,
			.rarity = sticker.m_value->m_rarity,
			.texture_id = nullptr,
			.type = type_item::type_sticker,
			.path_image = path,
			.is_initilization = false
			});
	}

	for (const auto& item : items)
	{
		if (!item.m_value->is_custom_player() && !item.m_value->is_knife(true) &&
			!item.m_value->is_weapon() && !item.m_value->is_glove(false))
			continue;

		if (!item.m_value->m_inventory_image)
			continue;

		std::string path = tfm::format("panorama/images/%s_png.vtex_c", item.m_value->m_inventory_image);
		std::string name = g_interface->m_localize->find_key(item.m_value->m_item_base_name);

		int rarity = item.m_value->is_custom_player() ? item.m_value->m_item_rarity : -1;
		auto vtex = VTexParser::Load(path.c_str());

		int type_ = item.m_value->is_custom_player() ? type_item::type_agent :
			item.m_value->is_knife(true) ? type_item::type_knife :
			item.m_value->is_weapon() ? type_item::type_weapons :
			item.m_value->is_glove(false) ? type_item::type_glove : type_item::type_sticker;

		skin_preview.emplace_back(gui_skins{
			.item_def = item.m_value->m_definition_index,
			.paint_name = "",
			.name = name,
			.skin_key = 0,
			.paint_id = -1,
			.rarity = rarity,
			.type = type_,
			.path_image = path,
			.is_initilization = true
			});

		g_render->LoadAndRenderImage(g_hook->m_device, vtex, &skin_preview.back().texture_id);
	}

	for (const auto& item : items)
	{
		if (!item.m_value->is_knife(true) && !item.m_value->is_weapon() && !item.m_value->is_glove(false))
			continue;

		for (const auto& paint_kit : paint_kits)
		{
			const uint64_t skinKey = get_alternate_icon_key_for_weapon_paint_wear_item(
				item.m_value->m_definition_index, paint_kit.m_value->paint_kit_id(), 0);

			//if (auto it = skin_image_path.find_by_key(skinKey); it.has_value())
			{
				/*if (!item.m_value->get_simple_weapon_name())
					continue;
				if (!paint_kit.m_value->paint_kit_name())
					continue;*/
					//std::string path = tfm::format("panorama/images/%s_png.vtex_c", it.value().m_simple_name);	
				std::string path = std::format("panorama/images/econ/default_generated/{}_{}_light_png.vtex_c", item.m_value->get_simple_weapon_name(), paint_kit.m_value->paint_kit_name());
				if (path.find("newcs2") != std::string::npos)
					continue;

				if (!g_interface->m_file_system->exists(path.c_str(), 0))
					continue;

				auto vtex = VTexParser::Load(path.c_str());
				if (vtex.data.empty()) continue;
				std::string name = g_interface->m_localize->find_key(item.m_value->m_item_base_name);
				std::string paint_name = g_interface->m_localize->find_key(paint_kit.m_value->paint_kit_description_string());


				auto rarity = std::clamp(item.m_value->m_item_rarity + paint_kit.m_value->paint_kit_rarity() - 1, 0,
					(paint_kit.m_value->paint_kit_rarity() == 7) ? 7 : 6);

				int type_ = item.m_value->is_custom_player() ? type_item::type_agent :
					item.m_value->is_knife(true) ? type_item::type_knife :
					item.m_value->is_weapon() ? type_item::type_weapons :
					item.m_value->is_glove(false) ? type_item::type_glove : type_item::type_sticker;

				skin_preview.emplace_back(gui_skins{
					.item_def = item.m_value->m_definition_index,
					.paint_name = paint_name,
					.name = name,
					.skin_key = skinKey,
					.paint_id = paint_kit.m_value->paint_kit_id(),
					.rarity = rarity,
					.type = type_,
					.path_image = path,
					.is_initilization = false
					});

				g_render->LoadAndRenderImage(g_hook->m_device, vtex, &skin_preview.back().texture_id);
			}
		}
	}

	std::copy_if(skin_preview.begin(), skin_preview.end(), std::back_inserter(item_default), [](const gui_skins& skin) {
		return skin.paint_id == -1 && skin.type == type_item::type_weapons;
		});
	std::copy_if(skin_preview.begin(), skin_preview.end(), std::back_inserter(item_knife_default), [](const gui_skins& skin) {
		return skin.paint_id == -1 && skin.type == type_item::type_knife;
		});
	std::copy_if(skin_preview.begin(), skin_preview.end(), std::back_inserter(item_stickers), [](const gui_skins& skin) {
		return skin.type == type_item::type_sticker;
		});
	std::copy_if(skin_preview.begin(), skin_preview.end(), std::back_inserter(item_agent), [](const gui_skins& skin) {
		return skin.type == type_item::type_agent;
		});
	std::copy_if(skin_preview.begin(), skin_preview.end(), std::back_inserter(item_glove), [](const gui_skins& skin) {
		return skin.type == type_item::type_glove;
		});

	for (const auto& item : skin_preview)
	{
		if (item.paint_id == -1)
			continue;

		if (item.type != type_item::type_knife && item.type != type_item::type_weapons)
			continue;

		item_weapons[item.item_def].emplace_back(item);
	}
}
