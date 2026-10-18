#include "player.h"
#include "../../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"
#include "../../sdk.h"

c_player_controller* c_player_controller::get_local_controller()
{
    using fn_t=c_player_controller*(__fastcall*)(int);
    static auto fn=reinterpret_cast<fn_t>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL,"85 D2 0F 85 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 20 48 8B F9 49 8B D8 8B CA E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8D 54"))+0x1a,1));
    return fn ? fn(-1) : nullptr;
}

c_player_pawn* c_player_controller::get_player_pawn()
{
	if (!handle_player_pawn().is_valid())
		return nullptr;

	int index = handle_player_pawn().get_entry_index();
	return g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(index);
}

int c_player_pawn::get_lerp_tick(int player_tick_count)
{
	int tick = -1, tick2 = -1, tick3 = -1; // Исправлено на int
	//int64_t player_tick_count_safe = player_tick_count; // Исправлено на int64_t

	//using function_t = char(__fastcall*)(int64_t, int*, int*, int*, int64_t); // Исправлено
	//static const auto fn = reinterpret_cast<function_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B F1 0F 29 74 24"));

	//printf("[DEBUG] Function address: 0x%p\n", fn);
	//if (!fn) {
	//	printf("[ERROR] Pattern scan failed!\n");
	//	return -1;
	//}

	//int64_t game_scene_node = reinterpret_cast<int64_t>(m_game_scene_node());
	//printf("[DEBUG] game_scene_node: 0x%p\n", game_scene_node);
	//if (!game_scene_node) {
	//	printf("[ERROR] m_game_scene_node() is NULL!\n");
	//	return -1;
	//}

	//memset(&tick, 0, sizeof(tick));
	//memset(&tick2, 0, sizeof(tick2));
	//memset(&tick3, 0, sizeof(tick3));

	//printf("[DEBUG] Before call: tick = %d, tick2 = %d, tick3 = %d\n", tick, tick2, tick3);

	//char result = fn(game_scene_node, &tick, &tick2, &tick3, player_tick_count_safe);

	//printf("[DEBUG] After call: tick = %d, tick2 = %d, tick3 = %d\n", tick, tick2, tick3);

	//if (!result) {
	//	printf("[ERROR] Function returned 0\n");
	//	return -1;
	//}

	return tick;
}



std::uint32_t c_player_pawn::get_owner_handle_idx()
{
	std::uint32_t Result = -1;
	if (this && m_p_collision() && !(m_p_collision()->GetSolidFlags() & 4))
		Result = this->get_owner_handle().get_entry_index();

	return Result;
}

c_player_pawn* c_player_controller::get_observer_pawn()
{
	if (!get_player_pawn_observer().is_valid())
		return nullptr;

	int index = get_player_pawn_observer().get_entry_index();
	return g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(index);
}

std::uint16_t c_player_pawn::GetCollisionMask()
{
	if (this && m_p_collision())
		return m_p_collision()->CollisionMask(); // Collision + 0x38

	return 0;
}

bool c_player_pawn::is_throwing() {
	c_weapon_base* active_weapon = this->ActiveWeapon();

	if (!active_weapon)
		return false;

	c_base_cs_grenade* grenade = reinterpret_cast<c_base_cs_grenade*>(active_weapon);
	c_cs_weapon_base_vdata* weapon_data = active_weapon->m_weapon_data();

	if (!grenade || !weapon_data)
		return false;

	if (weapon_data->m_weapon_type() == 0x9/*WEAPONTYPE_GRENADE*/ && grenade->m_throw_time() != 0.f)
		return true;

	return false;
}

c_weapon_base* c_player_pawn::ActiveWeapon()
{
	if (!this)
		return nullptr;

	c_player_weapon_services* WeaponServices = this->m_weapon_services();
	if (!WeaponServices || (uintptr_t)WeaponServices >= 0x7f7fffff7f7fffff)
		return nullptr;

	auto ActiveWeapon = g_interface->m_game_resource_service->get_game_entity_system()->get<c_weapon_base>(WeaponServices->m_active_weapon());
	if (!ActiveWeapon)
		return nullptr;

	return ActiveWeapon;
}

bool c_player_pawn::has_c4()
{
	return this->m_weapon_services() && this->m_weapon_services()->weapon_owns_this_type("weapon_c4");
}
c_cs_player_view_model_service* c_base_player_pawn::m_view_model_services() {
    static thread_local c_cs_player_view_model_service service;
    service.handle=c_base_handle{};
    auto* pawn=reinterpret_cast<c_player_pawn*>(this);
    auto* weapon=pawn->ActiveWeapon();
    auto* system=g_interface->m_game_resource_service ? g_interface->m_game_resource_service->get_game_entity_system() : nullptr;
    if (weapon && system) for(int i=1,highest=system->get_highest_entity_index();i<=highest;++i) {
        auto* entity=system->get<c_base_entity>(i);
        if(entity && entity->find_class(fnv1a::hash("C_CS2HudModelWeapon")) && entity->get_owner_handle()==weapon->get_ref_ehandle()) { service.handle=entity->get_ref_ehandle();break; }
    }
    return &service;
}

int i_engine_client::get_local_player() {
    auto* controller=c_player_controller::get_local_controller();
    return controller ? controller->get_ref_ehandle().get_entry_index() : 0;
}
