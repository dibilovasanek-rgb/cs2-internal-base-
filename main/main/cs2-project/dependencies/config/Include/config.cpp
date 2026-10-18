#include "Config.h"
#include <fstream>
#include <cstdlib>
#include "../../../features/skinchanger/skinchanger.h"
#include "../../../sdk/game_class/interfaces/player_inventory.h"

void holder_t::add(node_t* node_ptr)
{
	m_nodes.push_back(node_ptr);
}

void holder_t::load(const char* path, const char* file)
{
	std::filesystem::path pathObj(path);
	std::filesystem::path fileObj(file);
	std::filesystem::path config_path = pathObj / fileObj;

	std::ifstream config_file(config_path, std::ios::in | std::ios::binary);
	if (!config_file.is_open())
		return;

	std::string config_str((std::istreambuf_iterator<char>(config_file)),
		std::istreambuf_iterator<char>());

	for (auto x : m_nodes)
		x->load_setting(config_str);

	config_file.close();
}

void holder_t::load(std::string& config)
{
	for (auto x : m_nodes)
		x->load_setting(config);
}

void holder_t::save(std::string& config)
{
	for (auto x : m_nodes)
		x->save_setting(config);
}

void holder_t::save(const char* path, const char* file)
{
	std::filesystem::path pathObj(path);
	std::filesystem::path fileObj(file);
    if (fileObj.empty() || fileObj.filename() != fileObj)
        return;
    std::string config_path = (pathObj / fileObj).string();

	CreateFolder(pathObj.string());

	std::ofstream config_file(config_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!config_file.is_open())
        return;

	std::string config_str;

	for (auto x : m_nodes)
		x->save_setting(config_str);

	WriteToFile(config_str, config_file);
	CloseFile(config_file);
}

std::string c_settings::GetConfigPath() {
	return "configs";
}

void to_json(nlohmann::json& j, const EconItemData& item) {
	j = nlohmann::json{
		{"unDefIndex", item.unDefIndex},
		{"nQuality", item.nQuality},
		{"nRarity", item.nRarity},
		{"paintKit", item.paintKit},
		{"paintSeed", item.paintSeed},
		{"paintWear", item.paintWear},
		{"statTrak", item.statTrak}
	};
}

void from_json(const nlohmann::json& j, EconItemData& item) {
	j.at("unDefIndex").get_to(item.unDefIndex);
	j.at("nQuality").get_to(item.nQuality);
	j.at("nRarity").get_to(item.nRarity);
	j.at("paintKit").get_to(item.paintKit);
	j.at("paintSeed").get_to(item.paintSeed);
	j.at("paintWear").get_to(item.paintWear);
	j.at("statTrak").get_to(item.statTrak);
}

void c_settings::SaveEconItem(CEconItem* pItem, float seed, float wear, int statrack, const std::string& filename) {
	if (!pItem) return;

	EconItemData itemData = {
		pItem->m_unDefIndex,
		pItem->m_nQuality,
		pItem->m_nRarity,
		g_skins->pSelectedItem->pSelectedSkin->m_ID,
		seed,
		wear,
		statrack
	};

	std::string fullPath = GetConfigPath() + "\\" + filename;

	nlohmann::json jsonData;

	std::ifstream inFile(fullPath);
	if (inFile.is_open()) {
		inFile >> jsonData;
		inFile.close();
	}

	jsonData.push_back(itemData);

	std::ofstream outFile(fullPath);
	if (outFile.is_open()) {
		outFile << jsonData.dump(4);  
		outFile.close();
	}
}


std::vector<CEconItem*> c_settings::LoadEconItems(const std::string& filename) {
	std::string fullPath = GetConfigPath() + "\\" + filename;

	std::ifstream inFile(fullPath);
	if (!inFile.is_open()) return {};

	nlohmann::json jsonData;
	inFile >> jsonData;
	inFile.close();

	std::vector<CEconItem*> econItems;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory) return econItems;

	for (const auto& itemData : jsonData) {
		CEconItem* pItem = CEconItem::CreateInstance();
		if (!pItem) continue;

		auto highestIDs = pInventory->GetHighestIDs();

		pItem->m_ulID = highestIDs.first + 1;
		pItem->m_unInventory = highestIDs.second + 1;
		pItem->m_unAccountID = uint32_t(pInventory->get_owner().m_id);
		pItem->m_unDefIndex = itemData["unDefIndex"];
		pItem->m_nQuality = itemData["nQuality"];
		pItem->m_nRarity = itemData["nRarity"];
		pItem->SetPaintKit(itemData["paintKit"]);
		pItem->SetPaintSeed(itemData["paintSeed"]);
		pItem->SetPaintWear(itemData["paintWear"]);

		if (itemData["statTrak"] >= 0)
		{
			pItem->SetStatTrak(itemData["statTrak"]);
		}

		//if (pInventory->AddEconItem(pItem)) {
		//	g_skins->AddEconItemToList(pItem);
		//	econItems.push_back(pItem);
		//}
	}

	return econItems;
}


c_settings settings;
	
