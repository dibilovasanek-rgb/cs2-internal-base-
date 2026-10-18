#include "entity_list.h"
#include "../../core/interfaces/interface.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"

void c_ent_listener::init()
{
    const std::lock_guard lock(mutex);
    std::cout << ("cache entities") << std::endl;
    if (!g_interface->m_game_resource_service->get_game_entity_system())
        return;
        
    for (int i = 1; i <= g_interface->m_game_resource_service->get_game_entity_system()->get_highest_entity_index(); ++i)
    {
        c_base_entity* ent = static_cast<c_base_entity*>(g_interface->m_game_resource_service->get_game_entity_system()->get_entity_by_index(i));
        if (!ent)
            continue;

        add(ent, ent->get_ref_ehandle());
    }
}

void c_ent_listener::on_level_init()
{
    const std::lock_guard lock(mutex);
    m_players.clear();
    init();
}

void c_ent_listener::add(c_entity_instance* inst, c_base_handle handle)
{
    const std::lock_guard lock(mutex);
    c_base_entity* ent = static_cast<c_base_entity*>(inst);
    if (!ent || handle.get_entry_index() >= 0x7FFF)
        return;

    //if (ent->get_ref_ehandle() != handle) 
    //    return;

    const char* class_name = inst->get_entity_class_name();
    if (!class_name || !*class_name) return;

    if (fnv1a::hash(class_name) == fnv1a::hash("CCSPlayerController") || fnv1a::hash(class_name) == fnv1a::hash("C_EnvSky") || fnv1a::hash(class_name) == fnv1a::hash("C_PlantedC4") || fnv1a::hash(class_name) == fnv1a::hash("C_BaseCSGrenadeProjectile"))
    {

        auto& entries=m_players[get_hash(class_name)];
        if (std::find(entries.begin(),entries.end(),inst)!=entries.end()) return;
        entries.emplace_back(inst);

        std::string name = class_name;
        // The callback already supplies the authoritative handle. Calling
        // CEntityInstance::GetRefEHandle here used an old vtable slot and
        // crashed as soon as map entities (notably C_EnvSky) were created.
        std::cout << ("Added -> " + name + " : " + std::to_string(inst->get_ref_ehandle().to_int())) << std::endl;
    }
}

void c_ent_listener::level_shutdown()
{
    const std::lock_guard lock(mutex);
    m_players.clear();
}

void c_ent_listener::remove(c_entity_instance* inst, c_base_handle handle)
{
    const std::lock_guard lock(mutex);
    for (auto& [hash, entries] : m_players)
        entries.erase(std::remove(entries.begin(), entries.end(), inst), entries.end());
}

std::vector<c_entity_instance*> c_ent_listener::get(const char* class_name)
{
    const std::lock_guard lock(mutex);
    return m_players[get_hash(class_name)];
}
