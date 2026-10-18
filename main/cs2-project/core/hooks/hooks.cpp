#include "hooks.h"
#include "../../memory/current_build.h"
#include "../../memory/native_rtti.h"
#include "../../dependencies/minhook/minhook.h"
#include "../../features/skinchanger/skinchanger.h"
#include "../../sdk/sdk.h"
#include "../../sdk/game_class/interfaces/event.h"
#include "../../dependencies/config/Include/config.h"
#include "../../render/render.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../features/rage/ragebot.h"
#include "../../features/rage/legit.h"
#include "../../sdk/entity_listener/entity_list.h"
#include "../../dependencies/discord-rpc/discord_rpc_init.h"
#include "../../sdk/game_class/c_scene_object.h"
#include "../../features/visual/game_visuals/chams.h"
#include "../../features/visual/game_visuals/world.h"
#include "../../features/prediction/eng_pred.h"
#include "../../features/movement/movement.h"
#include "../../CRC.h"
#include "../../sdk/protobuffs/command_crc.h"
#include "../../sdk/input_system/inputsystem.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../sdk/game_class/interfaces/i_global_vars.h"
#include "../../entry.h"
#include "..\..\sdk\game_class\interfaces\i_resource_system.h"
#include "../../features/visual/esp.h"
#include "../../sdk/game_class/c_scene_skybox.h"
#include "../../sdk/game_class/c_model.h"
#include "../../features/visual/game_visuals/custom_sky.h"
#include "../../sdk/math/math.h"
#include "../../features/rage/anti_aim.h"
#include "../../sdk/game_class/c_grenade.h"
#include "../../sdk\game_class\interfaces\i_material_system.h"
#include "../../sdk\game_class\interfaces\csgoinput.h"
#include "../../features/visual/game_visuals/bullet_trace.h"
#include "../../features/visual/game_visuals/model_changer.h"
#include "../../render/liquid_glass.h"

bool c_hook::init()
{
	if (MH_Initialize() != MH_OK)
	{
		std::cout << "failed to MH_Initialize" << std::endl;
		return false;
	}

	static const auto resize_buffers_target = g_memory->get_virtual(m_swap_chain->pDXGISwapChain, 13);
	if (!g_memory->hook_function("hkResizeBuffers", &hk_resize_buffers, resize_buffers_target)) return false;

	//static const auto create_swap_chain_target = g_memory->get_virtual(m_dxgi_factory, 10);
	//g_memory->hook_function("hkCreateSwapChain", &hk_create_swap_chain, create_swap_chain_target);

	static const auto IsRelative = g_memory->get_virtual(g_interface->InputSystem, 76);
	if (!g_memory->hook_function("hkIsRelativeMouseMode", &IsRelativeMouseMode, IsRelative)) return false;

	static const auto present_target = g_memory->get_virtual(m_swap_chain->pDXGISwapChain, 8);
	if (!g_memory->hook_function("hkPresent", &hk_present, present_target)) return false;

	static const auto onaddentity=g_memory->find_pattern(CLIENT_DLL,"48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81");
	if (!g_memory->hook_function("hkOnAddEntity", &on_add_entity, onaddentity)) return false;

	static const auto removeentity=g_memory->find_pattern(CLIENT_DLL,"48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89");
	if (!g_memory->hook_function("hkOnRemoveEntity", &on_remove_entity, removeentity)) return false;

	if (!g_memory->hook_function("hkGetMatrixForView", &get_matrix_for_view, g_memory->find_pattern(CLIENT_DLL, "48 8B C4 48 89 68 ? 48 89 70 ? 57 48 81 EC ? ? ? ? 0F 29 70 ? 49 8B F1"))) return false;


	static const auto create_moveVFunc = g_memory->get_virtual(g_interface->m_csgoinput, 5U);
	if (!g_memory->hook_function("hkCreateMove", &create_move, create_moveVFunc)) return false;

	fnUpdateAimPunch_fn = reinterpret_cast<removed_aimfunction_t>(g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC 20 48 8B 89 98 15 00 00 48 8B DA E8 ? ? ? ? 48 8B C3 48 83 C4 20 5B C3"));

	fnget_hash_seed = reinterpret_cast<get_hash_seed>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 57 48 81 EC ? ? ? ? F3 0F 10 0A"));


	if (!g_memory->hook_function("hkRemoveScope", &RemoveScope, g_memory->find_pattern(CLIENT_DLL, "48 8B C4 53 57 48 83 EC 68 48 8B FA 44 0F 29 40 B8"))) return false;

	if (!g_memory->hook_function("hkFovChange", &FovChnger, g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4"))) return false;

	if (!g_memory->hook_function("hkViewModelChanger", &ViewModelChange, g_memory->find_pattern(CLIENT_DLL, "40 55 53 56 41 56 41 57 48 8B EC 48 83 EC 20 4D 8B F8 4C 8B F2 48 8B F1 E8"))) return false;

    if (!g_memory->hook_function("hkViewmodelTransform", &viewmodel_transform, g_memory->find_pattern(CLIENT_DLL, "48 8B C4 55 56 57 41 57 48 8D A8 28 FF FF FF 48 81 EC B8 01 00 00 48 8B F9 49 8B F0 8B 89 20 05 00 00"))) return false;
    if (!g_memory->hook_function("hkFirstPersonLegs", &first_person_legs, g_memory->find_pattern(CLIENT_DLL, "4C 8B DC 55 53 56 57 41 56 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 8B 42 24 4D 8B F1 F2 0F 10 42 1C"))) return false;
	if (!g_memory->hook_function("hkOverrideView", &OverrideView, g_memory->find_pattern(CLIENT_DLL,"40 53 57 48 83 EC 58 48 8B FA E8 ? ? ? ? 48 8B 05 ? ? ? ? 80 78 58 00"))) return false;

	if (!g_memory->hook_function("hkdraw_object", &draw_object, ([]()->uintptr_t { auto** table=NativeRtti::FindVTable(GetModuleHandleW(SCENESYSTEM_DLL),".?AVCAnimatableSceneObjectDesc@@"); return table ? reinterpret_cast<uintptr_t>(table[1]) : 0; })())) return false;

	if (!g_memory->hook_function("hkAllowCameraAngleChange", &AllowCameraAngleChange, g_memory->find_pattern(CLIENT_DLL, "85 D2 0F 85 00"))) return false;

	if (!g_memory->hook_function("hkLevelInit", &level_init, g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? C6 83 64 01 00 00 01 C6 83 52 01 00 00 00 48 83 C4 20 5B C3"))) return false;

	if (!g_memory->hook_function("hkLevelShutdown", &level_shutdown, g_memory->find_pattern(CLIENT_DLL, "48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4"))) return false;


	if (!g_memory->hook_function("hkFrameStageNotify", &hk_frame_stage_notify, g_memory->get_virtual(g_interface->m_client,36))) return false;


	if (!g_memory->hook_function("hkPopupAccptMatchFound", &PopupAccptMatchFound, g_memory->find_pattern(CLIENT_DLL, "40 56 57 41 57 48 83 EC 40 48 8B 3D ? ? ? ? 4D 85 C0 48 8B 35 ? ? ? ? 4C 8B FA"))) return false;

	uintptr_t addrGlowObject=g_memory->find_pattern(CLIENT_DLL,"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F2 48 8B F9 48 8B 54 24 ? 48 83 C1 40");
	if (!g_memory->hook_function("hkGlowObject", &GlowObject, addrGlowObject)) return false;

	uintptr_t addrisGlowing=g_memory->find_pattern(CLIENT_DLL,"0F B6 41 51 C3");
	if (!g_memory->hook_function("hkisGlowing", &isGlowing, addrisGlowing)) return false;


	if (!g_memory->hook_function("hk_set_model", &set_model, g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC 20 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24"))) return false;

	if (!g_memory->hook_function("hk_draw_crosshair", &draw_crosshair, g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ?? ?? ?? ?? 48 85"))) return false;

	if (!g_memory->hook_function("hk_equip_loadout", &equip_loadout_hook, g_memory->find_pattern(CLIENT_DLL, "48 8B C4 56 57 41 56 41 57 48 83 EC 48 0F B7 F2 4D 8B F9 41 0F B7 F8 4C 8B F1 83 FE 04 0F 83 ? ? ? ? 83 FF 3A 0F 83 ? ? ? ? 48 89 58 08 44 8B"))) return false;


    if (!g_memory->hook_function("hk_set_function_vector", &set_function_vector, reinterpret_cast<uintptr_t>(CurrentBuild::EngineVectorSetter()))) return false;
    if (!g_memory->hook_function("hkdraw_aggregate_scene_object", &hk_draw_aggregate_scene_object, g_memory->find_pattern(SCENESYSTEM_DLL, "48 8B C4 4C 89 40 ? 48 89 50 ? 55 53 41 57"))) return false;
    if (!g_memory->hook_function("hkParticleDrawArray", &particle_draw_array, g_memory->find_pattern(PARTICLES_DLL, "48 89 5C 24 ? 4C 89 4C 24 ? 4C 89 44 24 ? 55"))) return false;
    if (!g_memory->hook_function("hkUpdateWalls", &update_walls, g_memory->find_pattern(SCENESYSTEM_DLL, "48 8B C4 48 89 50 ? 48 89 48 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70"))) return false;
	if (!g_memory->hook_function("hk_skybox_material", &skybox_material, g_memory->find_pattern(SCENESYSTEM_DLL, "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55"))) return false;

	static const auto mouseinput = g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC 20 80 B9 ? ? ? ? 00 48 8B D9 0F 85 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 FF 90 C8 00 00 00");
	if (!g_memory->hook_function("hkmouseinput", &mouse_input_enabled, mouseinput)) return false;


	if (!g_memory->hook_function("remove_smoke", &remove_smoke, g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 48 8B 9C 24 88 00 00 00 4D"))) return false;


	//g_memory->hook_function("should_render", &should_render, g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC ? 48 8B D9 83 FA ? 75"));

	//g_memory->hook_function("draw_view_punch", &draw_view_punch, g_memory->find_pattern(CLIENT_DLL, "40 55 53 56 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B D9"));

	//g_memory->hook_function("hk_create_dlight", &remove_flash, g_memory->find_pattern(CLIENT_DLL, "85 D2 0F 88 ?? ?? ?? ?? 55 56 41 55"));

	//g_memory->hook_function("hkhook_input", &hook_input, g_memory->find_pattern(CLIENT_DLL, "4c 89 4c 24 ?? 55 53 57 41 56 48 8d 6c 24"));

	//g_memory->hook_function("hk_init_grenade", &init_grenade, g_memory->find_pattern(CLIENT_DLL, "40 56 48 83 EC 70"));

	return true;
}

float c_hook::viewmodel_fov() { return 
(settings.cfg().misc.flviewmodelfov); }
void c_hook::remove_smoke(__int64 a1, __int64* a2, int a3, int a4, __int64 a5, __int64 a6) {
    const auto original=g_memory->getOrig<decltype(&remove_smoke)>("remove_smoke");
    if (!settings.cfg().visuals.m_removals.data()[1] && original) original(a1,a2,a3,a4,a5,a6);
}
void c_hook::remove_flash(__int64 a1, int a2, __int64* a3, __int64 a4, __m128* a5) {
    const auto original = g_memory->getOrig<decltype(&remove_flash)>("remove_flash");
    if (!settings.cfg().visuals.m_removals.data()[0] && original) original(a1,a2,a3,a4,a5);
}



void* __fastcall c_hook::draw_view_punch(c_player_camera_services* pCameraServices, float* a2, float* a3, float* a4) {
	static const auto og = g_memory->getOrig<decltype(&draw_view_punch)>("draw_view_punch");

	//if (C_GET(int, Vars.nRecoilRemove) & REC_SHAKE)
		pCameraServices->m_vec_view_punch() = vec3_t();

	return og(pCameraServices, a2, a3, a4);
}

bool __fastcall c_hook::should_render(c_player_pawn* pawn, int splitscreen) {
	static const auto og = g_memory->getOrig<decltype(&should_render)>("should_render");

	if (splitscreen != -1)
		return og(pawn, splitscreen);

	if (/*config.visuals.world.remove_overhead_names->get(0) &&*/ pawn == sdk::m_local_pawn)
		return false;

	//if (/*config.visuals.world.remove_overhead_names->get(1) &&*/ pawn->m_team_num() == sdk::m_local_pawn->m_team_num())
	//	return og(pawn, splitscreen);

	//if (config.visuals.world.remove_overhead_names->get(2) && pawn->IsEnemy() && pawn != ctx.local_player_or_observer)
		//return false;

	return og(pawn, splitscreen);
}

int* c_hook::create_dlight(int* a1, int a2)
{
	static const auto og = g_memory->getOrig<decltype(&create_dlight)>("hk_create_dlight")(a1, a2);
	std::cout << a1 << " " << a2 << std::endl;
	return og;
}

void* c_hook::hook_input(c_cs_input_message* a1, c_csgo_input_history_entry_pb* a2, bool mouseuse, bool idk, void* unk, c_player_pawn* a6)
{
	static const auto og = g_memory->getOrig<decltype(&hook_input)>("hkhook_input")(a1, a2, mouseuse, idk, unk, a6);

	return og;
}

void c_hook::init_grenade(c_base_cs_grenade* grenade)
{
	static const auto og = g_memory->getOrig<decltype(&init_grenade)>("hk_init_grenade");
	auto base_entity = (c_base_entity*)grenade;

	g_esp->pos = grenade->m_game_scene_node()->m_vec_abs_origin();
	//std::cout << pos.x << " " << pos.y << " " << pos.z << "\n";
	//if (grenade->m_redraw())
	//{
	//	//g_misc->grenade_prediction_m.lock();
	//	//g_misc->lines.clear();
	//	//g_misc->grenade_prediction_m.unlock();
	//}
	//else
	//{
	//	auto owner = g_interface->m_game_resource_service->get_game_entity_system()->get(base_entity->get_owner_handle().get_entry_index());
	//	if (owner)
	//	{
	//		auto type = g_memory->CallVFunc<int>(grenade, 406);
	//		using function_t = void* (__fastcall*)(int, c_base_cs_grenade*, float);
	//		static const auto fn = reinterpret_cast<function_t>(g_memory->find_pattern(CLIENT_DLL, "48 8B C4 48 89 50 ? 55 41 56 48 8D 68"));
	//		auto grenade_trace = fn(type, grenade, 30.f);
	//		if (grenade_trace)
	//		{

	//			auto penis = (c_grenade_trace*)grenade_trace;
	//			for (int i = 0; i < penis->size; i++)
	//			{
	//				auto value = penis->data[i];

	//				if (!value.size)
	//					continue;

	//				if (!value.data)
	//					continue;

	//				for (int j{}; j < value.size; j++)
	//				{
	//					std::cout << value.data[j].x << std::endl;
	//					//	g_bullet_trace->add_grenade_trail(&value.data[j], value.size, 10);
	//				}
	//			}
	//			using function_t2 = void* (__fastcall*)(void* ptr);
	//			static const auto delete_entity = reinterpret_cast<function_t2>(g_memory->find_pattern(CLIENT_DLL, "48 89 6C 24 ? 56 48 83 EC 20 48 63 69 18"));
	//			delete_entity(grenade_trace);
	//			delete_entity(grenade);

	//		}
	//	}
	//}

	return og(grenade);
}

void c_hook::mark_interp_latch_flags_dirty(void* a1, unsigned int a2) {
	return;
}

struct c_scene_skybox_object {
public:
	std::byte pad_0000[0x110]; //0x0
	void** pSkyMaterial; // 0x110
	std::byte pad_0118[0x10]; //0x118
	Color_t colorSky;
};

__int64 __fastcall c_hook::skybox_material(std::uintptr_t* a1, std::uintptr_t* a2, std::uintptr_t* a3, int bDrawSkybox, int a5, std::uintptr_t* a6, std::uintptr_t* a7, std::uintptr_t* a8) 
{
    static const auto original = g_memory->getOrig<decltype(&skybox_material)>("hk_skybox_material");
    static std::recursive_mutex draw_mutex;
    const std::lock_guard lock(draw_mutex);
    CMaterial2** binding=nullptr; CMaterial2* old_material=nullptr;
    float* rgb=nullptr; float old_color[3]{};
    if (g_entry->init_cheat && a3 && bDrawSkybox>0 && bDrawSkybox<100 &&
        (settings.cfg().visuals.b_customsky || settings.cfg().worldModulation.b_worldModulation)) {
        auto* descriptor=*reinterpret_cast<std::byte**>(reinterpret_cast<std::byte*>(a3)+static_cast<size_t>(bDrawSkybox)*0x70-0x58);
        if (descriptor) {
            const auto index=g_customsky->nSelectedSkybox;
            if (settings.cfg().visuals.b_customsky && index<g_customsky->vecSkyboxes.size() && g_customsky->vecSkyboxes[index].pSkyboxMaterial) {
                auto** slot=*reinterpret_cast<CMaterial2***>(descriptor+0xD0);
                if (slot && *slot) { binding=slot; old_material=*slot; *slot=g_customsky->vecSkyboxes[index].pSkyboxMaterial; }
            }
            if (settings.cfg().worldModulation.b_worldModulation) {
                rgb=reinterpret_cast<float*>(descriptor+0xE8);
                std::copy_n(rgb,3,old_color);
                const auto color=settings.cfg().worldModulation.skyColor;
                rgb[0]=color.r/255.f;rgb[1]=color.g/255.f;rgb[2]=color.b/255.f;
            }
        }
    }
    auto result=original(a1,a2,a3,bDrawSkybox,a5,a6,a7,a8);
    if (binding) *binding=old_material;
    if (rgb) std::copy_n(old_color,3,rgb);
    return result;
}

void __fastcall c_hook::update_walls(void* a, void* b, void* aggregate) {
    const auto original = g_memory->getOrig<decltype(&update_walls)>("hkUpdateWalls");
    original(a,b,aggregate);
    if (g_entry->init_cheat) g_world->update_walls(aggregate);
}

std::uint64_t get_hash_function2(const char* str, std::int32_t seed)
{
	using function_t = std::int64_t(__fastcall*)(unsigned __int8*, std::int32_t);
	static const auto func = reinterpret_cast<function_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 08 44 0F B6 09 44 8B DA 4C 8B C1 41 8D 41 BF"));
	auto hash = func ? func((unsigned __int8*)str, seed) : 0;
	return hash;
}

void c_hook::set_function_vector(int* a1, unsigned int hash_function, vec4_t* value)
{
	last_hook = "set_function_vector";

	static const auto og = g_memory->getOrig<decltype(&set_function_vector)>("hk_set_function_vector");

	static const int hash = get_hash_function2(("DofRanges"), 0x3141592F);
	unsigned int hash2 = (0x5BD1E995 * (hash ^ 0x73)) ^ ((0x5BD1E995 * (hash ^ 0x73u)) >> 13);
	unsigned int hash3 = (1540483477 * hash2) ^ ((1540483477 * hash2) >> 15);

	if (settings.cfg().worldModulation.b_world_blur)
	{
		vec4_t new_value;
		new_value.y = 0.f;
		new_value.x = -100.f;
		new_value.w = 2000.f;
		new_value.z = 180.f;

		og(a1, hash3, &new_value);
	}

	return og(a1, hash_function, value);
}

void __fastcall c_hook::equip_loadout_hook(void* rcx, uint16_t iTeam, uint16_t iSlot, uint64_t itemId, uint16_t unknown, bool update)
{
	last_hook = "equip_loadout_hook";

	static const auto og = g_memory->getOrig<decltype(&equip_loadout_hook)>("hk_equip_loadout");

    og(rcx, iTeam, iSlot, itemId, unknown, update);
    if (g_entry->init_cheat) g_skins->equip_loadout(iTeam, iSlot, itemId);
}

//void* hooks::should_draw_legs::hk_should_draw_legs(void* a1, void* a2, void* a3, void* a4, void* a5)
//{
//	static auto original = m_should_draw_legs.get_original< decltype(&hk_should_draw_legs) >();
//
//	if (settings.cfg().visuals.m_removals.data()[3])
//		return nullptr;
//
//	return original(a1, a2, a3, a4, a5);
//}



bool __fastcall c_hook::draw_crosshair(c_weapon_base* pWeaponBaseGun)
{
	last_hook = "draw_crosshair";

	static const auto og = g_memory->getOrig<decltype(&draw_crosshair)>("hk_draw_crosshair");

	if (!sdk::m_local_pawn)
		return og(pWeaponBaseGun);

	if (sdk::m_local_pawn->m_is_alive())
		return !sdk::m_local_pawn->m_is_scoped();

	return og(pWeaponBaseGun);
}

bool __fastcall c_hook::is_loadout_allow(void* a1) {
	return true;
}

void __fastcall c_hook::set_model(void* rcx, const char* model)
{
	last_hook = "set_model";

	static const auto og = g_memory->getOrig<decltype(&set_model)>("hk_set_model");
	g_skins->on_set_model((c_base_model_entity*)rcx, model);
	return og(rcx, model);
}

bool __fastcall c_hook::isGlowing(C_GlowProperty* glowprop)
{
	last_hook = "isGlowing";

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return false;

	if (!glowprop || !glowprop->GetParentEntity())
		return false;

	c_entity_identity* identify = reinterpret_cast<c_entity_instance*>(glowprop->GetParentEntity())->m_entity();

	if (!identify) return {};
	const char* class_name = identify->m_designer_name();
	if (!class_name)
		return false;
	auto parent = glowprop->GetParentEntity();
	auto Lpawn = sdk::m_local_pawn;
	std::string sName = class_name;
	if (sName.find("player") != std::string::npos) {

		if (parent != Lpawn && !settings.cfg().visuals.b_glow_esp)
			return false;

		if (!Lpawn || !parent || !parent->is_player_pawn() || !parent->m_is_alive())
			return false;

		if (parent == Lpawn) {
			if (settings.cfg().visuals.b_local_glow && on_thirdperson) {
				//if (parent->m_team_num() != Lpawn->m_team_num()) {
				return true;
				//}
			}
		}

		if (parent->m_team_num() == Lpawn->m_team_num())
			return false;

		return true;
	}
	if (sName.find("weapon") != std::string::npos) {
		if (settings.cfg().visuals.b_weapons_glow) {
			if (!parent->get_owner_handle().get())
				return true;
		}
	}
	return false;
}

void __fastcall c_hook::GlowObject(C_GlowProperty* glowProperty, float* color)
{
	last_hook = "GlowObject";

	static const auto og = g_memory->getOrig<decltype(&GlowObject)>("hkGlowObject");

	og(glowProperty, color);
	if (!glowProperty || !color || !glowProperty->GetParentEntity()) return;
	c_entity_identity* identify = reinterpret_cast<c_entity_instance*>(glowProperty->GetParentEntity())->m_entity();
	if (!identify) return;

	const char* class_name = identify->m_designer_name();
	if (!class_name)
		return;
	std::string sName = class_name;

	if (sName.find("player") != std::string::npos) {
		if (glowProperty->GetParentEntity() == sdk::m_local_pawn && settings.cfg().visuals.b_local_glow) {
			color[0] = settings.cfg().visuals.glow_local_color.r / 255.f;
			color[1] = settings.cfg().visuals.glow_local_color.g / 255.f;
			color[2] = settings.cfg().visuals.glow_local_color.b / 255.f;
			color[3] = settings.cfg().visuals.glow_local_color.a / 255.f;
		}
		else {
			color[0] = settings.cfg().visuals.glow_color.r / 255.f;
			color[1] = settings.cfg().visuals.glow_color.g / 255.f;
			color[2] = settings.cfg().visuals.glow_color.b / 255.f;
			color[3] = settings.cfg().visuals.glow_color.a / 255.f;
		}
	}
	if (sName.find("weapon") != std::string::npos) {
		color[0] = settings.cfg().visuals.glow_weapons_color.r / 255.f;
		color[1] = settings.cfg().visuals.glow_weapons_color.g / 255.f;
		color[2] = settings.cfg().visuals.glow_weapons_color.b / 255.f;
		color[3] = settings.cfg().visuals.glow_weapons_color.a / 255.f;
	}
}

long __fastcall c_hook::hk_wnd_proc(HWND wnd, std::uint32_t msg, WPARAM wparam, LPARAM lparam)
{
	last_hook = "hk_wnd_proc";

	if (g_render->OnWndProc(wnd, msg, wparam, lparam))
		return 1L;

	return ::CallWindowProcW(g_wnd_proc, wnd, msg, wparam, lparam);
}

void* __fastcall c_hook::hk_should_update_sequences(void* a1, void* a2, void* a3) {
    const auto original = g_memory->getOrig<decltype(&hk_should_update_sequences)>("should_update_sequences");
    return original ? original(a1, a2, a3) : nullptr;
}

__int64 __fastcall c_hook::PopupAccptMatchFound(void* unk, const char* event_name, void* unk1, float unk2)
{
	last_hook = "PopupAccptMatchFound";

	static const auto og = g_memory->getOrig<decltype(&PopupAccptMatchFound)>("hkPopupAccptMatchFound");

	if (g_interface->m_engine->is_in_game())
		return og(unk, event_name, unk1, unk2);

	if (!settings.cfg().misc.auto_accept)
		return og(unk, event_name, unk1, unk2);

	if (event_name && std::string(event_name).find("popup_accept_match_found") != std::string::npos) {
		using set_player_ready_t = bool(__fastcall*)(void*, const char*);
		static const auto fn = reinterpret_cast<set_player_ready_t>(g_memory->find_pattern(CLIENT_DLL, "53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 ? BA 02 00 00 00 33 C9"));
		if (fn) fn(nullptr, "");
	}

	return og(unk, event_name, unk1, unk2);
}

bool __fastcall c_hook::create_move(csgoinput* pInput, int nSlot, bool bActive)
{
	last_hook = "create_move";

	static const auto og = g_memory->getOrig<decltype(&create_move)>("hkCreateMove");
	const bool b_Result = og(pInput, nSlot, bActive);

	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return b_Result;

    if (!g_interface->refresh() || !g_entry->init_cheat || !ImGui::GetCurrentContext()) return b_Result;
	sdk::ScreenSize = ImGui::GetIO().DisplaySize;

	sdk::m_local_controller = c_player_controller::get_local_controller();
	if (sdk::m_local_controller == nullptr)
		return b_Result;

	sdk::m_cmd = pInput->get_usercmd(sdk::m_local_controller);
	if (!sdk::m_cmd)
		return b_Result;

    if (!sdk::m_cmd->pb.has_base()) return b_Result;

	sdk::m_local_pawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(sdk::m_local_controller->m_pawn());
	if (sdk::m_local_pawn == nullptr)
		return b_Result;

	auto Ent = g_entlistener->get("CCSPlayerController");
	if (g_interface->m_engine->is_in_game() && Ent.empty())
		g_entlistener->init();

	if (!sdk::m_local_pawn->m_is_alive())
		return b_Result;

	if (sdk::m_cmd->pb.mutable_base()->mutable_viewangles())
		sdk::angleStor = { sdk::m_cmd->pb.base().viewangles().x(),sdk::m_cmd->pb.base().viewangles().y(),sdk::m_cmd->pb.base().viewangles().z() };

	

	// bugged <144 fps
	g_ragebot->revolver_cock(sdk::m_cmd);
	//g_movement->AutoStrafe(sdk::m_cmd, sdk::m_local_pawn);
	g_movement->AutoStrafe(sdk::m_cmd, sdk::m_local_pawn);
	g_movement->bunnyhop(sdk::m_cmd, sdk::m_local_pawn);
	if (settings.cfg().ragebot.b_ragebot || (!settings.cfg().legitbot.b_aim_fov && !settings.cfg().legitbot.b_trigger))
        g_antiaim->runAA(sdk::m_cmd, sdk::m_local_controller, sdk::m_local_pawn);
	g_prediction->run();
    LEGIT::aim(pInput, sdk::m_cmd);

	//g_ragebot->double_tap();

	/*g_ragebot->trigger_bot(sdk::m_cmd);
	g_ragebot->run(pInput, sdk::m_cmd);*/
	g_ragebot->double_tap();
	g_ragebot->run(pInput,sdk::m_cmd);
	
	

	g_prediction->end();

	g_movement->auto_pick(sdk::m_cmd);	

	g_movement->movement_fix(sdk::m_cmd, { sdk::angleStor.x,sdk::angleStor.y,sdk::angleStor.z });

    auto* base = sdk::m_cmd->pb.mutable_base();
    base->mutable_buttons_pb()->set_buttonstate1(sdk::m_cmd->m_button_state.nValue);
    base->mutable_buttons_pb()->set_buttonstate2(sdk::m_cmd->m_button_state.nValueChanged);
    base->mutable_buttons_pb()->set_buttonstate3(sdk::m_cmd->m_button_state.nValueScroll);
    if (settings.cfg().ragebot.b_anti_aim && !(sdk::m_cmd->m_button_state.nValue & (IN_ATTACK | IN_SECOND_ATTACK)))
        sdk::m_cmd->SetSubTickAngle({base->viewangles().x(), base->viewangles().y(), base->viewangles().z()});
    CRC::UpdateMoveCRC(sdk::m_cmd);
	//CRC::Save(pBaseCmd, sdk::m_cmd);
	//if (CRC::CalculateCRC(pBaseCmd) == true) // I don't think it's necessary because there's a protobuff
	//	CRC::Apply(sdk::m_cmd);
	return b_Result;
}

void __fastcall c_hook::hk_frame_stage_notify(void* source_to_client, int stage) {
	static const auto og = g_memory->getOrig<decltype(&hk_frame_stage_notify)>("hkFrameStageNotify");
	last_hook = "hk_frame_stage_notify";
    if (!g_entry->init_cheat) return og(source_to_client, stage);
    if (!g_interface->refresh()) return og(source_to_client, stage);

    sdk::m_local_controller = c_player_controller::get_local_controller();
    sdk::m_local_pawn = sdk::m_local_controller ? sdk::m_local_controller->get_player_pawn() : nullptr;
    if (sdk::m_local_pawn && settings.cfg().visuals.m_removals[0]) {
        const auto duration = g_schema->schema_get_offset("C_CSPlayerPawnBase", "m_flFlashDuration");
        if (duration) *reinterpret_cast<float*>(reinterpret_cast<std::byte*>(sdk::m_local_pawn) + duration) = 0.f;
    }

	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return og(source_to_client, stage);

	og(source_to_client, stage);
	try {
		g_skins->frame_stage(stage);
	}
	catch (...)
	{
		std::cout << "error skinchanger" << std::endl;
	}
	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END && sdk::m_local_pawn) {
        g_modelchanger->SetPlayerModel();
        g_modelchanger->SetWeaponModel(sdk::m_local_pawn);
        FrlsRefreshKnifeAnimations();
    }
	//g_customsky->Initialize();

	switch (stage) {
	case FRAME_NET_UPDATE_END:
		//g_ragebot->store_records(stage);
		g_ragebot->store_records(stage);
		break;
	case FRAME_SIMULATE_END: {
        g_bullet_trace->spawn_particle_map_at_position(settings.cfg().visuals.i_mapeffect, settings.cfg().visuals.c_mapeffect);
        sdk::on_round_start = false;
        g_customsky->PrecacheSkyboxes();
        }
		try {
			g_esp->store_players();
		}
		catch (...)
		{
			std::cout << "error esp" << std::endl;
		}
		for (auto it = g_esp->m_dropped_weapons.begin(); it != g_esp->m_dropped_weapons.end();)
		{
			auto player = g_interface->m_game_resource_service->get_game_entity_system()->get(it->handle.get_entry_index() & 0x7FFF);

			if (!player || !player->find_class(fnv1a::hash("C_BasePlayerWeapon")) || player->get_owner_handle().is_valid()) {
				it = g_esp->m_dropped_weapons.erase(it);
				continue;
			}

			it++;
		}
		for (int i{}; i <= g_interface->m_game_resource_service->get_game_entity_system()->get_highest_entity_index(); i++) {
		auto entity = g_interface->m_game_resource_service->get_game_entity_system()->get(i);

			if (!entity)
				continue;

			if (!entity->find_class(fnv1a::hash("C_BasePlayerWeapon")))
				continue;

			if (!entity->m_game_scene_node())
				continue;

			if (entity->get_owner_handle().is_valid())
				continue;

			auto handle = entity->get_ref_ehandle();

			auto find = std::find_if(g_esp->m_dropped_weapons.begin(), g_esp->m_dropped_weapons.end(), [handle](c_dropped_weapons x) {
				return x.handle == handle;
				});

			if (find != g_esp->m_dropped_weapons.end())
			{
				auto& weapon = *find;
				weapon.handle = handle;
				weapon.abs_origin = entity->m_game_scene_node()->m_vec_abs_origin();
				weapon.mins = entity->m_p_collision()->m_vec_mins();
				weapon.maxs = entity->m_p_collision()->m_vec_maxs();

				c_weapon_base* weapon_ = (c_weapon_base*)(entity);

				if (weapon_->m_attribute_manager() && weapon_->m_attribute_manager()->m_item() && weapon_->m_attribute_manager()->m_item()->get_static_data()) {
					weapon.weapon = weapon_;
					weapon.weapon_name = g_interface->m_localize->find_key(weapon_->m_attribute_manager()->m_item()->get_static_data()->m_item_base_name);
					weapon.item_index = weapon_->m_attribute_manager()->m_item()->m_item_definition_index();
				}
			}
			else
			{
				auto& weapon = g_esp->m_dropped_weapons.emplace_back();
				auto weapon_ = (c_weapon_base*)(entity);
				weapon.weapon = weapon_;
				weapon.handle = handle;
				weapon.abs_origin = entity->m_game_scene_node()->m_vec_abs_origin();
				weapon.mins = entity->m_p_collision()->m_vec_mins();
				weapon.maxs = entity->m_p_collision()->m_vec_maxs();

				if (weapon_->m_attribute_manager() && weapon_->m_attribute_manager()->m_item() && weapon_->m_attribute_manager()->m_item()->get_static_data()) {
					weapon.weapon_name = g_interface->m_localize->find_key(weapon_->m_attribute_manager()->m_item()->get_static_data()->m_item_base_name);
					weapon.item_index = weapon_->m_attribute_manager()->m_item()->m_item_definition_index();
				}
			}
		}

		break;
	default:
		break;
	}
}

void* __fastcall c_hook::level_init(void* self, const char* map) {
    const auto original = g_memory->getOrig<decltype(&level_init)>("hkLevelInit");
    auto* result = original(self, map);
    sdk::m_local_pawn = nullptr; sdk::m_local_controller = nullptr; sdk::m_cmd = nullptr;
    g_interface->refresh(); g_entlistener->on_level_init(); g_esp->avatar_cache.clear();
    g_skins->force_update = true;
    return result;
}

void* __fastcall c_hook::level_shutdown(void* a1)
{
	static const auto og = g_memory->getOrig<decltype(&level_shutdown)>("hkLevelShutdown");
	last_hook = "level_shutdown";

	// reset global variables since it got discarded by the game
	g_interface->m_globalVars = nullptr;
	g_interface->game_particle_manager_system = nullptr;
	g_entlistener->level_shutdown();
    g_world->reset_walls();
    hit_capibara.clear();
    sdk::m_local_pawn = nullptr; sdk::m_local_controller = nullptr; sdk::m_cmd = nullptr;
	if (g_Discord)
		g_Discord->Update(nullptr);
	g_esp->avatar_cache.clear();
	sdk::m_local_pawn = nullptr;
	sdk::m_local_controller = nullptr;
	//g_customsky->init = false;
	//g_ragebot->m_aim_targets.clear();
	if (!g_ragebot->m_aim_targets.empty())
		g_ragebot->m_aim_targets.clear();
	if (!g_ragebot->m_lag_records.empty())
		g_ragebot->m_lag_records.clear();
	//if (g_ragebot->m_best_target)
	//	g_ragebot->m_best_target.reset();

	return og(a1);
}

int get_model_type(const std::string_view& name) {
	if (name.find("sun") != std::string::npos
		|| name.find("clouds") != std::string::npos)
		return MODEL_SUN;

	if (name.find("effects") != std::string::npos)
		return MODEL_EFFECTS;

	return MODEL_OTHER;
}

void* __fastcall c_hook::hk_draw_aggregate_scene_object(void* a1, void* a2, c_base_scene_data* data, int count, void* a5, void* a6, void* a7, void* a8) {
    const auto original=g_memory->getOrig<decltype(&hk_draw_aggregate_scene_object)>("hkdraw_aggregate_scene_object");
    if (!data || count<=0 || count>4096 || !g_entry->init_cheat || !settings.cfg().worldModulation.b_worldModulation)
        return original(a1,a2,data,count,a5,a6,a7,a8);
    std::vector<Color_t> saved; saved.reserve(count);
    for (int i=0;i<count;++i) {
        auto& mesh=data[i]; saved.emplace_back(mesh.r,mesh.g,mesh.b,mesh.a);
        if (!mesh.m_material || !mesh.m_material->GetName()) continue;
        const int type=get_model_type(mesh.m_material->GetName());
        const auto color=type==MODEL_OTHER ? settings.cfg().worldModulation.worldColor : settings.cfg().worldModulation.cloudColor;
        mesh.r=color.r;mesh.g=color.g;mesh.b=color.b;
    }
    auto* result=original(a1,a2,data,count,a5,a6,a7,a8);
    for (int i=0;i<count;++i) { data[i].r=saved[i].r;data[i].g=saved[i].g;data[i].b=saved[i].b;data[i].a=saved[i].a; }
    return result;
}


void* __fastcall c_hook::IsRelativeMouseMode(void* pThisptr, bool bActive)
{
	static const auto og = g_memory->getOrig<decltype(&IsRelativeMouseMode)>("hkIsRelativeMouseMode");
	last_hook = "IsRelativeMouseMode";

	bMainActive = bActive;

	if (bMainWindowOpened)
		return og(pThisptr, false);

	return og(pThisptr, bActive);
}

void* __fastcall c_hook::draw_object(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
    static const auto og = g_memory->getOrig<decltype(&draw_object)>("hkdraw_object");
    last_hook = "draw_object";
    if (!og) return nullptr;
    if (!arrMeshDraw || nDataCount <= 0 || !sdk::m_local_pawn || !sdk::m_local_controller ||
        !g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game() ||
        !g_interface->m_game_resource_service->get_game_entity_system())
        return og(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
    void* result = nullptr;
    for (int i = 0; i < nDataCount; ++i) {
        auto& mesh = arrMeshDraw[i];
        auto* material = mesh.pMaterial;
        auto* secondary = mesh.pMaterialSecondary;
        const auto color = mesh.colValue;
        bool overridden = false;
        if (!material || CRT::StringCompare(material->GetName(), "materials/dev/glowproperty.vmat") != 0)
            overridden = g_chams->OnDrawObject(pAnimatableSceneObjectDesc, pDx11, &mesh, 1, pSceneView, pSceneLayer, pUnk, pUnk2);
        if (!overridden && material && settings.cfg().worldModulation.b_worldModulation) {
            const auto* name = material->GetName();
            if (name && std::string_view(name).find("props") != std::string_view::npos) mesh.colValue = settings.cfg().worldModulation.worldColor;
        }
        if (!overridden) result = og(pAnimatableSceneObjectDesc, pDx11, &mesh, 1, pSceneView, pSceneLayer, pUnk, pUnk2);
        mesh.pMaterial = material; mesh.pMaterialSecondary = secondary; mesh.colValue = color;
    }
    return result;
}

bool __fastcall c_hook::mouse_input_enabled(void* pThisptr)
{
	static const auto og = g_memory->getOrig<decltype(&mouse_input_enabled)>("hkmouseinput");
	last_hook = "mouse_input_enabled";

	return bMainWindowOpened ? false : og(pThisptr);
}

void* __fastcall c_hook::UpdateSkyBox(c_envsky* sky)
{
	static const auto og = g_memory->getOrig<decltype(&UpdateSkyBox)>("hkUpdateSkyBox");
	last_hook = "UpdateSkyBox";

	g_world->skybox(sky);

	return og(sky);
}

void* __fastcall c_hook::UpdatePostProccesing(c_postproccesing* post_processing, int a2)
{
	static const auto og = g_memory->getOrig<decltype(&UpdatePostProccesing)>("hkUpdatePostProc");
	last_hook = "UpdatePostProccesing";

	auto* result = og(post_processing, a2);

	g_world->exposure(post_processing);
	return result;
}

void* __fastcall c_hook::on_add_entity(void* rcx, c_entity_instance* inst, c_base_handle handle)
{
    static const auto og = g_memory->getOrig<decltype(&on_add_entity)>("hkOnAddEntity");
    last_hook = "on_add_entity";
    auto* result = og(rcx, inst, handle);
    if (inst && handle.is_valid()) g_entlistener->add(inst, handle);
    return result;
}
void* __fastcall c_hook::on_remove_entity(void* rcx, c_entity_instance* inst, c_base_handle handle) {
    static const auto og = g_memory->getOrig<decltype(&on_remove_entity)>("hkOnRemoveEntity");
    last_hook = "on_remove_entity";
    if (inst) g_entlistener->remove(inst, handle);
    return og(rcx, inst, handle);
}

void __fastcall c_hook::get_matrix_for_view(void* render_game_system, void* view_render, v_matrix* out_world_to_view, v_matrix* out_view_to_projection, v_matrix* out_world_to_projection, v_matrix* out_world_to_pixels)
{
	static const auto og = g_memory->getOrig<decltype(&get_matrix_for_view)>("hkGetMatrixForView");
	last_hook = "get_matrix_for_view";

	og(render_game_system, view_render, out_world_to_view, out_view_to_projection, out_world_to_projection, out_world_to_pixels);
	m_view_matrix = *out_world_to_projection;
}

HRESULT __stdcall c_hook::hk_present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags)
{
	static const auto og = g_memory->getOrig<decltype(&hk_present)>("hkPresent");
	last_hook = "hk_present";
    if (!g_entry->init_cheat || !bInitialized) return og(swap_chain, sync_interval, flags);
    if (m_swap_chain->pDXGISwapChain != swap_chain) {
		if(bInitialized) ImGui_ImplDX11_Shutdown();
		LiquidGlass::Shutdown();
        if(m_render_view) {m_render_view->Release();m_render_view=nullptr;}
        if(m_context) {m_context->Release();m_context=nullptr;}
        if(m_device) {m_device->Release();m_device=nullptr;}
        auto* previous = m_swap_chain->pDXGISwapChain;
        swap_chain->AddRef(); m_swap_chain->pDXGISwapChain=swap_chain;
        if (previous) previous->Release();
        if(FAILED(swap_chain->GetDevice(__uuidof(ID3D11Device),reinterpret_cast<void**>(&m_device)))) return og(swap_chain,sync_interval,flags);
        m_device->GetImmediateContext(&m_context);
		if(bInitialized) {
			ImGui_ImplDX11_Init(m_device,m_context);
			LiquidGlass::Init(m_device,m_context);
		}
    }

	if (m_render_view == nullptr)
		g_render->create_render_target(m_swap_chain->pDXGISwapChain);

	// set our render target	
	if (m_render_view != nullptr)
		m_context->OMSetRenderTargets(1, &m_render_view, nullptr);

	g_world->exposure(sdk::m_local_pawn);
	g_world->refresh_walls();

	g_render->render_imgui_dx11(m_swap_chain->pDXGISwapChain);

	return og(m_swap_chain->pDXGISwapChain, sync_interval, flags);
}

HRESULT __stdcall c_hook::hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags)
{
	static const auto og = g_memory->getOrig<decltype(&hk_resize_buffers)>("hkResizeBuffers");
	last_hook = "hk_resize_buffers";

	if (g_hook->m_render_view)
	{
		LiquidGlass::ReleaseSnapshot();
		g_hook->m_render_view->Release();
		g_hook->m_render_view = NULL;
	}

	return og(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
}

HRESULT __stdcall c_hook::hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain)
{
	static const auto og = g_memory->getOrig<decltype(&hk_create_swap_chain)>("hkCreateSwapChain");
	last_hook = "hk_create_swap_chain";

	if (g_hook->m_render_view)
	{
		g_hook->m_render_view->Release();
		g_hook->m_render_view = NULL;
	}

	return og(factory, device, desc, swap_chain);
}

float __fastcall c_hook::FovChnger(uintptr_t a1)
{
	const auto Original = g_memory->getOrig<decltype(&FovChnger)>("hkFovChange");
	
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return Original(a1);

	if(!sdk::m_local_pawn)
		return Original(a1);

	if (!sdk::m_local_pawn->is_player_pawn())
		return Original(a1);
	
	//if (!settings.cfg().ragebot.b_ragebot) {
	//	if (sdk::m_local_pawn->is_player_pawn()) {
	//		if (sdk::m_local_pawn->m_is_scoped())
	//			return Original(a1);
	//	}
	//}

	if (settings.cfg().visuals.b_scope_fov)
	{
		if (sdk::m_local_pawn->m_is_alive() && sdk::m_local_pawn->m_is_scoped()) {
			c_weapon_base* active_weapon = sdk::m_local_pawn->ActiveWeapon();
			if (!active_weapon)
				return Original(a1);

			c_weapon_base_gun* pWeaponEntity = reinterpret_cast<c_weapon_base_gun*>(active_weapon);
			switch (pWeaponEntity->m_zoom_level())
			{
			case 1:
				return 90 - settings.cfg().visuals.zoom_1_override;
			case 2:
				return 90 - settings.cfg().visuals.zoom_2_override;
			}
		}

		//std::cout << "1: " << weapon_data->m_fl_zoom_time_0() << " 2: " << weapon_data->m_fl_zoom_time_1() << " 3: " << weapon_data->m_fl_zoom_time_2() << "\n";


		//weapon_data->m_scope_fov_1() = settings.cfg().visuals.zoom_1_override;
		//weapon_data->m_scope_fov_2() = settings.cfg().visuals.zoom_2_override;
	}
	return settings.cfg().misc.p_fov;
}

void __fastcall c_hook::ViewModelChange(float* unk1, float* viewmodelOffsets, float* viewmodelFov)
{
    const auto original = g_memory->getOrig<decltype(&ViewModelChange)>("hkViewModelChanger");
    original(unk1, viewmodelOffsets, viewmodelFov);
    if (reinterpret_cast<void*>(unk1) == sdk::m_local_pawn && viewmodelFov)
        *viewmodelFov = std::clamp(settings.cfg().misc.flviewmodelfov, 40.f, 120.f);
}

void __fastcall c_hook::RemoveScope(void* a1, void* a2)
{
    const auto original = g_memory->getOrig<decltype(&RemoveScope)>("hkRemoveScope");
    original(a1, a2);
    if (a1 == sdk::m_local_pawn && a2 && sdk::m_local_pawn && sdk::m_local_pawn->m_is_scoped())
        *reinterpret_cast<bool*>(static_cast<std::byte*>(a2) + 4) = false;
}

void __fastcall c_hook::AllowCameraAngleChange(csgoinput* pCSGOInput, int a2)
{
	const auto orig = g_memory->getOrig<decltype(&AllowCameraAngleChange)>("hkAllowCameraAngleChange");
	last_hook = "AllowCameraAngleChange";

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return orig(pCSGOInput, a2);

	if (!settings.cfg().ragebot.b_anti_aim)
		return orig(pCSGOInput, a2);

	if (!sdk::m_local_pawn)
		return orig(pCSGOInput, a2);

	if (!sdk::m_local_controller)
		return orig(pCSGOInput, a2);

	if (!sdk::m_local_pawn->m_is_alive())
		return orig(pCSGOInput, a2);

	c_user_cmd* pCmd = sdk::m_cmd;
	if (pCmd == nullptr)
		return orig(pCSGOInput, a2);
	auto pBaseCmd = pCmd->pb.mutable_base();
	if (!pBaseCmd)
		return orig(pCSGOInput, a2);

	if (pBaseCmd->mutable_viewangles() == nullptr)
		return orig(pCSGOInput, a2);

	qangle_t angOriginalAngle = { pBaseCmd->viewangles().x(), pBaseCmd->viewangles().y(), pBaseCmd->viewangles().z() };
	pBaseCmd->mutable_viewangles()->set_x(sdk::angleStor.x);
	pBaseCmd->mutable_viewangles()->set_y(sdk::angleStor.y);
	pBaseCmd->mutable_viewangles()->set_z(sdk::angleStor.z);
	orig(pCSGOInput, a2);
	pBaseCmd->mutable_viewangles()->set_x(angOriginalAngle.x);
	pBaseCmd->mutable_viewangles()->set_y(angOriginalAngle.y);
	pBaseCmd->mutable_viewangles()->set_z(angOriginalAngle.z);
}


void __fastcall c_hook::OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup)
{
	const auto orig = g_memory->getOrig<decltype(&OverrideView)>("hkOverrideView");
	last_hook = "OverrideView";
    orig(pClientModeCSNormal, pSetup);
    if (!pSetup) return;

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!sdk::m_local_pawn)
		return;

    constexpr std::uint8_t explicitAspectFlag = 1u << 1;
    const float requested = settings.cfg().misc.aspect_ratio_value;
    if (settings.cfg().misc.b_aspect_ratio && std::isfinite(requested)) {
        const float ratio = std::clamp(requested, 1.f, 3.f);
        constexpr float halfRadians = 3.14159265358979323846f / 360.f;
        if (std::isfinite(pSetup->flFov) && pSetup->flFov > 0.f && pSetup->flFov < 179.f)
            pSetup->flFov = std::atan(std::tan(pSetup->flFov * halfRadians) * ratio * 0.75f) / halfRadians;
        pSetup->flAspectRatio = ratio;
        pSetup->nViewFlags |= explicitAspectFlag;
    } else {
        pSetup->nViewFlags &= static_cast<std::uint8_t>(~explicitAspectFlag);
    }

	if (!sdk::m_local_pawn->m_is_alive())
		return;

	if (settings.cfg().misc.b_third_person)
	{
		// Thirdperson is a toggle: one key press enables it, the next disables it.
		if (g_inputsystem->IsKeyReleased(settings.cfg().misc.i_key_thirdperson))
			on_thirdperson = !on_thirdperson;
		const bool should_enable = on_thirdperson;

		if (settings.cfg().misc.thirdperson_dist <= 0)
			return;

		static float current_distance = 0.0f;
		const float target_distance = should_enable && sdk::m_local_pawn->m_is_alive() ? settings.cfg().misc.thirdperson_dist : 0.0f;

		const float lerp_speed_open = 10.0f;
		const float lerp_speed_close = 20.0f;
		const float lerp_speed = should_enable ? lerp_speed_open : lerp_speed_close;

		current_distance = std::lerp(current_distance, target_distance, g_interface->m_globalVars->flFrameTime * lerp_speed);

		if (current_distance > 0.10f)
		{
			qangle_t adjusted_cam_view_angle = sdk::angleStor;
			adjusted_cam_view_angle.x *= -1;

			pSetup->vecOrigin = math::CalculateCameraPosition(sdk::m_local_pawn->get_eye_pos(), -current_distance, adjusted_cam_view_angle);

			Ray_t ray{};
			game_trace_t trace{};
			TraceFilter_t filter{ 0x1C3003, sdk::m_local_pawn, nullptr, 4 };

			if (g_interface->m_game_trace->trace_shape(&ray, sdk::m_local_pawn->get_eye_pos(), pSetup->vecOrigin, &filter, &trace)) {
				if (trace.HitEntity != nullptr)
					pSetup->vecOrigin = trace.m_pos;
			}
			pSetup->angView = math::NormalizeAngles(math::CalcAngel(pSetup->vecOrigin, sdk::m_local_pawn->get_eye_pos()));
		}
	}
	/*if (!on_thirdperson) {
		if (!sdk::m_cmd->pb.mutable_base()->has_viewangles())
			return;
		pSetup->angView = sdk::angleStor;
	}*/


}

void __fastcall c_hook::RenderBatchList(void* a1)
{
	static const auto og = g_memory->getOrig<decltype(&RenderBatchList)>("hkRenderBatchList");
	last_hook = "RenderBatchList";

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return og(a1);

	void** SceneObjectDesc = *reinterpret_cast<void***>(reinterpret_cast<uintptr_t>(a1) + 0x38);

	if (SceneObjectDesc) {
		using FnType = const char* (__fastcall*)(void*);
		FnType func = reinterpret_cast<FnType>((*reinterpret_cast<void***>(SceneObjectDesc))[0]);
		const char* result = func(SceneObjectDesc);
		auto classname = std::string_view(result);

		auto v10 = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(a1) + 0x18);

		uint32_t count = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(a1) + 0x30);

		if (v10) {
			for (uint32_t i = 0; i < count; ++i) {
				CMeshDrawPrimitive_t* meshDraw = (CMeshDrawPrimitive_t*)(reinterpret_cast<uintptr_t>(v10) + (0x68 * i));

				if (!meshDraw) continue;

				if (settings.cfg().worldModulation.b_ParticlesEffect && classname.find("CParticleObjectDesc") != std::string::npos) {
					meshDraw->m_rgba = settings.cfg().worldModulation.particleColor;
				}

				if (settings.cfg().worldModulation.b_worldModulation && classname.find("CAnimatableSceneObjectDesc") != std::string::npos) {
					auto materialName = std::string_view(meshDraw->m_pMaterial->GetName());
					if (materialName.find("props") != std::string::npos) {
						meshDraw->m_rgba = settings.cfg().worldModulation.worldColor;
					}
				}

				if (settings.cfg().worldModulation.b_worldModulation && classname.find("CBaseSceneObjectDesc") != std::string::npos) {
					meshDraw->m_rgba = settings.cfg().worldModulation.worldColor;
				}

				if (settings.cfg().worldModulation.b_worldModulation && classname.find("CAggregateSceneObjectDesc") != std::string::npos) {
					if (meshDraw->m_pObject) {
						for (int j = 0; j < meshDraw->m_pObject->count; ++j) {
							CAggregateSceneObjectData& data = meshDraw->m_pObject->data[j];
							data.m_rgba = settings.cfg().worldModulation.worldColor;
						}
					}
				}
			}
		}
	}

	og(a1);
}

void __fastcall c_hook::viewmodel_transform(c_base_entity* model, vec3_t* origin, qangle_t* angles) {
    const auto original = g_memory->getOrig<decltype(&viewmodel_transform)>("hkViewmodelTransform");
    original(model, origin, angles);
    if (!model || !origin || !angles || !sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive()) return;
    auto* owner = model->get_owner_handle().get();
    if (owner != sdk::m_local_pawn && (!owner || owner->get_owner_handle().get() != sdk::m_local_pawn)) return;
    vec3_t forward{}, right{}, up{};
    math::angle_vectors3({angles->x, angles->y, angles->z}, &forward, &right, &up);
    const auto& cfg = settings.cfg().misc;
    *origin = *origin + right * cfg.hand_X + forward * cfg.hand_Y + up * cfg.hand_Z;
    angles->x = std::remainder(angles->x + cfg.rotPitch, 360.f);
    angles->y = std::remainder(angles->y + cfg.rotYaw, 360.f);
    angles->z = std::remainder(angles->z + cfg.rotRoll, 360.f);
}
void* __fastcall c_hook::first_person_legs(void* a, void* b, void* c, void* d, void* e) {
    const auto original = g_memory->getOrig<decltype(&first_person_legs)>("hkFirstPersonLegs");
    if (settings.cfg().visuals.m_removals[2]) return nullptr;
    return original(a,b,c,d,e);
}

namespace {
float* particle_color_data(void* pointer) {
    __try {
        auto* data=static_cast<std::byte*>(pointer);
        if (!data) return nullptr;
        auto* first=*reinterpret_cast<std::byte**>(data+0x48);
        auto* second=first ? *reinterpret_cast<std::byte**>(first+0x18) : nullptr;
        auto* third=second ? *reinterpret_cast<const char***>(second+8) : nullptr;
        if (!third || !*third || strstr(*third,"spectator_utility_trail") || strstr(*third,"smokegrenade")) return nullptr;
        return reinterpret_cast<float*>(data+0x50);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}
}
void* __fastcall c_hook::particle_draw_array(int64_t a,int64_t b,__m128* c,uint64_t* d,void* e) {
    const auto original=g_memory->getOrig<decltype(&particle_draw_array)>("hkParticleDrawArray");
    auto* rgba=settings.cfg().worldModulation.b_ParticlesEffect ? particle_color_data(reinterpret_cast<void*>(b)) : nullptr;
    float saved[4]{};
    if (rgba) {
        std::copy_n(rgba,4,saved);
        const auto color=settings.cfg().worldModulation.particleColor;
        rgba[0]=color.r/255.f;rgba[1]=color.g/255.f;rgba[2]=color.b/255.f;rgba[3]=color.a/255.f;
    }
    auto* result=original(a,b,c,d,e);
    if (rgba) std::copy_n(saved,4,rgba);
    return result;
}
