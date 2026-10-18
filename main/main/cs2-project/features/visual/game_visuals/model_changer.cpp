#include "model_changer.h"
#include <unordered_map>
#include "../../../sdk/game_class/interfaces/i_client.h"
#include "../../../sdk/game_class/c_econ_definition.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

#include "../../../dependencies/protobufs/include/google/protobuf/stubs/common.h"


#include <iostream>
#include <filesystem>
#include <algorithm>

#include "../../../sdk/game_class/interfaces/i_resource_system.h"
#include "../../../sdk/schema/class/entity.h"
#include "../../string.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"
#include "../../../dependencies/config/Include/config.h"
#include "../../../sdk/sdk.h"
#include "../../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../../sdk/game_class/c_utl_string_token.h"
#include "../../../sdk/game_class/interfaces/player_inventory.h"

namespace {
bool InstallBundledKaneki(const std::string& playerModelsDirectory) {
    if (playerModelsDirectory.empty()) return false;
    wchar_t modulePath[MAX_PATH]{};
    HMODULE module{};
    static int moduleAnchor = 0;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&moduleAnchor), &module) ||
        !GetModuleFileNameW(module, modulePath, MAX_PATH)) return false;
    const auto bundleRoot = std::filesystem::path(modulePath).parent_path() / "models" / "otakku" / "kaneki";
    if (!std::filesystem::exists(bundleRoot / "kaneki.vmdl_c")) return false;
    // The model's compiled materials reference agents/models/... (not characters/...).
    const auto destination = std::filesystem::path(std::u8string(playerModelsDirectory.begin(), playerModelsDirectory.end())).parent_path() / "agents" / "models" / "otakku" / "kaneki";
    std::error_code ec;
    std::filesystem::create_directories(destination, ec);
    std::filesystem::copy(bundleRoot, destination, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    return !ec && std::filesystem::exists(destination / "kaneki.vmdl_c");
}

void AddJeffreyEpsteinIfInstalled(std::vector<Model_t>& models) {
    const auto gameRoot = STR::GetDirectory();
    if (gameRoot.empty()) return;

    // This is the resource layout used by the original base. Do not expose a
    // selectable entry unless every compiled resource is actually present.
    const auto root = std::filesystem::path(std::u8string(gameRoot.begin(), gameRoot.end())) /
        "csgo" / "models" / "custom" / "agents" / "silver_casual";
    const auto modelFile = root / "silver_casual.vmdl_c";
    const auto material = std::filesystem::path(std::u8string(gameRoot.begin(), gameRoot.end())) /
        "csgo" / "materials" / "custom" / "agents" / "silver_casual" / "silver_casual.vmat_c";
    const auto headMaterial = material.parent_path() / "silver_casual_head.vmat_c";
    if (!std::filesystem::is_regular_file(modelFile) || !std::filesystem::is_regular_file(material) ||
        !std::filesystem::is_regular_file(headMaterial)) return;

    if (std::none_of(models.begin(), models.end(), [](const Model_t& model) {
        return model.strModelPath == "models/custom/agents/silver_casual/silver_casual.vmdl";
    })) {
        models.push_back({ "Jeffrey Epstein", "models/custom/agents/silver_casual/silver_casual.vmdl", 0 });
    }
}

void AddYasirModels(std::vector<Model_t>& models) {
    const auto gameRoot = STR::GetDirectory();
    if (gameRoot.empty()) return;

    const auto yasir = std::filesystem::path(std::u8string(gameRoot.begin(), gameRoot.end())) / "csgo" / "yasir" / "characters";
    std::error_code error;
    if (!std::filesystem::is_directory(yasir, error) || error) return;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(yasir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".vmdl_c")
                continue;

            const auto name = entry.path().filename().string();
            if (name.find("arm") != std::string::npos || name.find("nohitbox") != std::string::npos)
                continue;

            auto path = entry.path().generic_string();
            const auto marker = path.find("characters/");
            if (marker == std::string::npos)
                continue;
            path = path.substr(marker, path.size() - marker - 2); // .vmdl_c -> .vmdl

            if (std::any_of(models.begin(), models.end(), [&path](const Model_t& model) { return model.strModelPath == path; }))
                continue;
            models.push_back({ "Yasir: " + name.substr(0, name.size() - 7), path, 0 });
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "yasir model scan error: " << e.what() << std::endl;
    }
}
}

namespace {
void AddGameModels(std::vector<Model_t>& models, bool knives) {
    auto* system = g_interface->m_client ? g_interface->m_client->get_econ_item_system() : nullptr;
    auto* schema = system ? system->get_econ_item_schema() : nullptr;
    if (!schema) return;
    const auto& items = schema->get_sorted_item_definition_map();
    if (!items.is_valid()) return;
    for (const auto& entry : items) {
        auto* item = entry.m_value;
        if (!item || !(knives ? item->is_knife(false) : item->is_custom_player())) continue;
        const char* model = item->get_model_name();
        if (!model || !*model) continue;
        if (std::any_of(models.begin(), models.end(), [model](const Model_t& value) { return value.strModelPath == model; })) continue;
        const char* label = item->m_item_base_name;
        const char* localized = label ? g_interface->m_localize->find_key(label) : nullptr;
        models.push_back({localized ? localized : (label ? label : model), model, item->m_definition_index});
    }
}
}

void c_model_changer::UpdateWeaponModels() {
    std::vector<Model_t> vecWeaponModelsF;
    AddGameModels(vecWeaponModelsF, true);

    std::string strModelsDirectory = STR::GetDirectory(WeaponModels);

    try {
        std::filesystem::create_directories(std::filesystem::path(std::u8string(strModelsDirectory.begin(), strModelsDirectory.end())));
        for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path(std::u8string(strModelsDirectory.begin(), strModelsDirectory.end())))) {
            if (!entry.is_regular_file())
                continue;

            std::string strFileName = entry.path().filename().string();

            if (strFileName.size() <= 7)
                continue;

            if (strFileName.substr(strFileName.size() - 7) != ".vmdl_c")
                continue;

            std::string strModelPath = entry.path().string();
            strModelPath = strModelPath.substr(0, strModelPath.size() - 2);
            std::replace(strModelPath.begin(), strModelPath.end(), '\\', '/');

            size_t pos = strModelPath.find("weapons/");
            if (pos != std::string::npos)
                strModelPath = strModelPath.substr(pos);

            Model_t pModel;
            pModel.strModelName = strFileName.substr(0, strFileName.size() - 7);
            pModel.strModelPath = strModelPath;
            vecWeaponModelsF.push_back(pModel);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "knifechanger files error: " << e.what() << std::endl;
    }

    g_modelchanger->vecWeaponModels = vecWeaponModelsF;
}

namespace {
std::uint32_t knife_subclass(std::uint16_t definition) {
    switch (definition) {
    case 500: return 3933374535u;
    case 503: return 3787235507u;
    case 505: return 4046390180u;
    case 506: return 2047704618u;
    case 507: return 1731408398u;
    case 508: return 1638561588u;
    case 509: return 2282479884u;
    case 512: return 3412259219u;
    case 514: return 2511498851u;
    case 515: return 1353709123u;
    case 516: return 4269888884u;
    case 517: return 1105782941u;
    case 518: return 275962944u;
    case 519: return 1338637359u;
    case 520: return 3230445913u;
    case 521: return 3206681373u;
    case 522: return 2595277776u;
    case 523: return 4029975521u;
    case 525: return 365028728u;
    case 526: return 3845286452u;
    default: return 0;
    }
}

c_base_entity* KnifeHud(c_player_pawn* pawn, c_base_entity* weapon) {
    const auto armsOffset = g_schema->schema_get_offset("C_CSPlayerPawn", "m_hHudModelArms");
    const auto childOffset = g_schema->schema_get_offset("CGameSceneNode", "m_pChild");
    const auto siblingOffset = g_schema->schema_get_offset("CGameSceneNode", "m_pNextSibling");
    const auto ownerOffset = g_schema->schema_get_offset("CGameSceneNode", "m_pOwner");
    if (!armsOffset || !childOffset || !siblingOffset || !ownerOffset) return nullptr;
    const auto armsHandle = *reinterpret_cast<c_base_handle*>(reinterpret_cast<std::byte*>(pawn) + armsOffset);
    auto* arms = armsHandle.get();
    auto* node = arms ? arms->m_game_scene_node() : nullptr;
    auto* child = node ? *reinterpret_cast<c_game_scene_node**>(reinterpret_cast<std::byte*>(node) + childOffset) : nullptr;
    for (int depth = 0; child && depth < 64; ++depth) {
        auto* owner = *reinterpret_cast<c_base_entity**>(reinterpret_cast<std::byte*>(child) + ownerOffset);
        if (owner && owner->get_owner_handle() == weapon->get_ref_ehandle()) return owner;
        child = *reinterpret_cast<c_game_scene_node**>(reinterpret_cast<std::byte*>(child) + siblingOffset);
    }
    return nullptr;
}
}

bool c_model_changer::SetWeaponModel(c_player_pawn* player) {
    struct Applied { uint32_t hash{}; int hud = -1; };
    static std::unordered_map<int, Applied> applied;
    if (!player || !player->m_is_alive() || !settings.cfg().visuals.b_knifechanger || nSelectedWeaponModel >= vecWeaponModels.size()) {
        applied.clear(); return false;
    }
    auto* services = player->m_weapon_services();
    if (!services) return false;
    auto& weapons = services->m_my_weapons();
    if (!weapons.m_elements || weapons.m_size < 0 || weapons.m_size > 64) return false;
    const auto& selected = vecWeaponModels[nSelectedWeaponModel];
    const auto hash = fnv1a::hash(selected.strModelPath.c_str());
    for (int i = 0; i < weapons.m_size; ++i) {
        auto handle = weapons.m_elements[i];
        auto* weapon = reinterpret_cast<c_weapon_base*>(handle.get());
        if (!weapon || weapon->get_ref_ehandle() != handle || weapon->get_owner_handle() != player->get_ref_ehandle()) continue;
        auto* attributes = weapon->m_attribute_manager();
        auto* item = attributes ? attributes->m_item() : nullptr;
        if (!item) continue;
        const auto definition = item->m_item_definition_index();
        if (definition != 41 && definition != 42 && definition != 59 && (definition < 500 || definition > 526)) continue;
        auto* hud = KnifeHud(player, weapon);
        const int hudHandle = hud ? hud->get_ref_ehandle().to_int() : -1;
        auto& state = applied[handle.to_int()];
        const auto subclass = knife_subclass(selected.definitionIndex);
        const bool changed = state.hash != hash || (selected.definitionIndex && definition != selected.definitionIndex) || (subclass && weapon->m_nSubclassID() != subclass);
        if (changed) {
            if (selected.definitionIndex) item->m_item_definition_index() = selected.definitionIndex;
            weapon->set_model(selected.strModelPath.c_str());
            if (subclass) { weapon->m_nSubclassID() = subclass; weapon->UpdateSubClass(); }
            state.hash = hash;
        }
        if (hud && (changed || state.hud != hudHandle)) {
            hud->set_model(selected.strModelPath.c_str());
            FrlsQueueKnifeAnimations(weapon);
        }
        state.hud = hudHandle;
    }
    return true;
}

void c_model_changer::UpdatePlayerModels() {
    std::vector<Model_t> vecPlayerModelsF;
    AddGameModels(vecPlayerModelsF, false);

    std::string strModelsDirectory = STR::GetDirectory(PlayerModels);
    const bool kanekiInstalled = InstallBundledKaneki(strModelsDirectory);

    try {
        std::filesystem::create_directories(std::filesystem::path(std::u8string(strModelsDirectory.begin(), strModelsDirectory.end())));
        for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path(std::u8string(strModelsDirectory.begin(), strModelsDirectory.end())))) {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension().string() != ".vmdl_c")
                continue;

            std::string strFileName = entry.path().filename().string();

            // Ignore an older incorrectly installed copy; Kaneki is registered below
            // with its correct agents/models resource path.
            const auto rawPath = entry.path().generic_string();
            if (rawPath.find("/characters/otakku/kaneki/") != std::string::npos)
                continue;

            if (strFileName.find("arm") != std::string::npos || strFileName.find("nohitbox") != std::string::npos)
                continue;

            std::string strModelPath = entry.path().string();
            strModelPath = strModelPath.substr(0, strModelPath.size() - 2);
            std::replace(strModelPath.begin(), strModelPath.end(), '\\', '/');

            size_t pos = strModelPath.find("characters/");
            if (pos != std::string::npos)
                strModelPath = strModelPath.substr(pos);

            // CS2 stores agent resources below characters/agents, while the
            // resource API expects the path without the characters prefix.
            if (strModelPath.rfind("characters/agents/", 0) == 0)
                strModelPath.erase(0, std::string("characters/").size());

            Model_t pModel;
            pModel.strModelName = strFileName.substr(0, strFileName.size() - 7);
            pModel.strModelPath = strModelPath;
            vecPlayerModelsF.push_back(pModel);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "modelchanger files error: " << e.what() << std::endl;
    }

    if (kanekiInstalled) {
        vecPlayerModelsF.push_back({ "Kaneki", "agents/models/otakku/kaneki/kaneki.vmdl", 0 });
    }
    AddJeffreyEpsteinIfInstalled(vecPlayerModelsF);
    AddYasirModels(vecPlayerModelsF);

    g_modelchanger->vecPlayerModels = vecPlayerModelsF;
}

bool c_model_changer::SetPlayerModel() {
    if (!g_interface || !g_interface->m_engine || !g_interface->m_resource_system)
        return false;
    if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
        return false;

    if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive())
        return false;

    if (settings.cfg().visuals.b_modelchanger) {
        if (nSelectedPlayerModel == ~1U)
            return false;

        if (vecPlayerModels.size() <= nSelectedPlayerModel)
            return false;

        Model_t pModel = vecPlayerModels[nSelectedPlayerModel];
        const char* szModelPath = pModel.strModelPath.c_str();

        auto uPlayerModelHash = fnv1a::hash(szModelPath);
        static int lastPawnHandle = -1;
        const int pawnHandle = sdk::m_local_pawn->get_ref_ehandle().to_int();
        if (lastPawnHandle == pawnHandle && uLastPlayerModelHash == uPlayerModelHash) return true;
        lastPawnHandle = pawnHandle;
        

        g_modelchanger->uLastPlayerModelHash = uPlayerModelHash;

        if (!szModelPath || !*szModelPath)
            return false;
        g_interface->m_resource_system->PreCache1(szModelPath);
        sdk::m_local_pawn->set_model(szModelPath);
    }

    return true;
}
