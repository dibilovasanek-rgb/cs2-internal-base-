#include "basehandle.h"
#include "../../core/interfaces/interface.h"
#include "../game_class/interfaces/igameentitysystem.h"
#include "../sdk.h"

c_base_entity* c_base_handle::get() const
{
    if (!is_valid())
        return nullptr;

    auto* system = g_interface && g_interface->m_game_resource_service ? g_interface->m_game_resource_service->get_game_entity_system() : nullptr;
    c_base_entity* ent = system ? system->get<c_base_entity>(*this) : nullptr;
    if (!ent || ent->get_ref_ehandle() != *this)
        return nullptr;

    return ent;
}
