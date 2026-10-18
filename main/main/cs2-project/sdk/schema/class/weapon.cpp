#include "weapon.h"
#include "../../game_class/interfaces/igameentitysystem.h"
#include "player.h"

bool c_player_weapon_services::weapon_owns_this_type(const char* name) {
    auto* system = g_interface->m_game_resource_service ? g_interface->m_game_resource_service->get_game_entity_system() : nullptr;
    auto& weapons = m_my_weapons();
    if (!system || !name || !weapons.m_elements || weapons.m_size <= 0 || weapons.m_size > 64) return false;
    for (int i = 0; i < weapons.m_size; ++i) {
        auto* weapon = system->get<c_base_entity>(weapons.m_elements[i]);
        auto* identity = weapon ? weapon->m_entity() : nullptr;
        const auto* designer = identity ? identity->m_designer_name() : nullptr;
        if (designer && std::strcmp(designer,name) == 0) return true;
    }
    return false;
}

float c_base_player_weapon::get_max_speed() {
    auto* data=static_cast<std::byte*>(GetVData());
    const auto offset=g_schema->schema_get_offset("CCSWeaponBaseVData","m_flMaxSpeed");
    const auto mode_offset=g_schema->schema_get_offset("C_CSWeaponBase","m_weaponMode");
    if (!data || !offset || !mode_offset) return 0.f;
    const int mode=std::clamp(*reinterpret_cast<int*>(reinterpret_cast<std::byte*>(this)+mode_offset),0,1);
    return *reinterpret_cast<float*>(data+offset+4*mode);
}
