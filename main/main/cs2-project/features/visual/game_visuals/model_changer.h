#pragma once

#include <vector>
#include <cstdint>
#include <string>

class c_player_pawn;

struct Model_t {
	std::string strModelName;
	std::string strModelPath;
    uint16_t definitionIndex{};
};

class c_model_changer {
public:
	std::vector<Model_t> vecPlayerModels;
	std::vector<Model_t> vecWeaponModels;
	unsigned long long nSelectedPlayerModel = ~1U;
	unsigned long long nSelectedWeaponModel = ~1U;
	uint32_t uLastWeaponModelHash;
	uint32_t uLastPlayerModelHash;

	int last_weapon_type;

	void UpdateWeaponModels();
	bool SetWeaponModel(c_player_pawn* player);
	void UpdatePlayerModels();
	bool SetPlayerModel();
};

inline c_model_changer* g_modelchanger = new c_model_changer();