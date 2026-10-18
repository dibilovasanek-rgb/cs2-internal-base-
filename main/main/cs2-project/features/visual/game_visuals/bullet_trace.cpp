#include "bullet_trace.h"
#include "../../../memory/current_build.h"
#include "../../../memory/memory.h"
#include <vector>
#include "../../../core/interfaces/interface.h"
#include "../../../sdk/sdk.h"
#include "../../../dependencies/config/Include/config.h"
#include "../../../sdk/game_class/interfaces/i_resource_system.h"

void c_particle_snapshot::draw(int count, void* data)
{
	g_memory->CallVFunc<void>(this, 1U, count, data);
}

void c_particle_manager::create_snapshot(c_strong_handle<c_particle_snapshot>* out_particle_snapshot)
{
    if (!out_particle_snapshot) return;
    *out_particle_snapshot = {};
    __int64 nUnknown = 0;
    g_memory->CallVFunc<void>(this, 41, out_particle_snapshot, &nUnknown);
    if (out_particle_snapshot->pBinding)
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(const_cast<ResourceBinding_t*>(out_particle_snapshot->pBinding)) + 8);
}

void c_particle_manager::draw(c_strong_handle<c_particle_snapshot>* particle_snapshot, int count, void* data)
{
	if (!particle_snapshot || !particle_snapshot->pBinding || !particle_snapshot->pBinding->pData || !data || count <= 0)
		return;
	particle_snapshot->operator->()->draw(count, data);
    auto* binding = const_cast<ResourceBinding_t*>(particle_snapshot->pBinding);
    particle_snapshot->pBinding = nullptr;
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(binding) + 8) == 0 && g_interface->m_resource_handle_utils)
        g_interface->m_resource_handle_utils->DeleteResource(binding);
}

void c_game_particle_manager_system::create_effect_index(unsigned int* effect_index, particle_effect* effect_data)
{
	using fn_t = void(__fastcall*)(c_game_particle_manager_system*, unsigned int*, particle_effect*);
	static fn_t fn = reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL, "40 57 48 83 EC 20 49 8B ?? 48 8B"));
	if (effect_data && effect_data->name && effect_index) {
        g_interface->m_resource_system->PreCache1(effect_data->name);
        create_effect2(effect_index, effect_data->name);
        if (*effect_index == ~0U) std::cerr << "Particle creation failed: " << effect_data->name << std::endl;
    }
}
void c_game_particle_manager_system::create_effect2(unsigned int* pEffectIndex, const char* szName)
{
	using fn_t = void(__fastcall*)(void*, uint32_t*, const char*, int, int64_t, int64_t, int64_t, int);
	static fn_t fn = reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL, "4C 8B DC 53 48 81 EC 90 00 00 00 F2 0F 10 05"));
	if (fn && szName) fn(this, pEffectIndex, szName, 8, 0, 0, 0, 0);
}
void c_game_particle_manager_system::SetEffect(unsigned int effect_index, int unk, void* clr, int unk2)
{
    using fn_t = bool(__fastcall*)(void*, unsigned, int, void*, float);
    static auto fn = reinterpret_cast<fn_t>(CurrentBuild::Address(0xA12520));
    if (fn && effect_index != ~0U && clr) fn(this, effect_index, unk, clr, 0.f);
}
void c_game_particle_manager_system::fnInitEffect(int effect_index, unsigned unk, const c_strong_handle<c_particle_snapshot>* snapshot) {
    using fn_t = bool(__fastcall*)(void*, int, unsigned, const ResourceBinding_t*);
    static auto fn = reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL, "56 48 83 EC 20 41 8B F0 49 8B C1 45 33 C0 4C 8B C9 83 FA FF"));
    if (fn && snapshot && snapshot->pBinding && snapshot->pBinding->pData) fn(this, effect_index, unk, snapshot->pBinding);
}

namespace {
bool ParticleManagerReady() {
    if (!g_interface->game_particle_manager_system) {
        auto** slot = reinterpret_cast<c_game_particle_manager_system**>(CurrentBuild::Address(CurrentBuild::particle_manager));
        g_interface->game_particle_manager_system = slot ? *slot : nullptr;
    }
    return g_interface->game_particle_manager_system && g_interface->m_resource_system;
}
unsigned int SpawnPointEffect(const char* name, const vec3_t& position, Color_t color) {
    if (!ParticleManagerReady()) return ~0U;
    unsigned int index = ~0U;
    particle_effect effect{};
    effect.name = name;
    g_interface->game_particle_manager_system->create_effect_index(&index, &effect);
    if (index == ~0U) return index;
    particle_color rgb{float(color.r), float(color.g), float(color.b)};
    auto point = position;
    g_interface->game_particle_manager_system->SetEffect(index, 16, &rgb, 0);
    g_interface->game_particle_manager_system->SetEffect(index, 0, &point, 0);
    return index;
}
void StopPointEffect(unsigned int index) {
    using fn_t = void(__fastcall*)(void*, int, char, char);
    static auto destroy = reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL, "83 FA FF 0F 84 ? ? ? ? 41 54 41 56 41 57 48 83 EC 40"));
    if (index == ~0U || !destroy || !ParticleManagerReady()) return;
    destroy(g_interface->game_particle_manager_system, index, true, true);
    g_memory->CallVFunc<void>(g_interface->game_particle_manager_system, 3U, static_cast<int>(index));
}
}

void cbullet_tracer::spawn_particle_at_position(const vec3_t& position, int ip) {
    static constexpr const char* effects[] = {
        "bin/lightning_kill.vpcf", "bin/skull_kill.vpcf", "bin/coins_kill.vpcf",
        "bin/flower_kill.vpcf", "bin/tombstone_kill.vpcf"
    };
    SpawnPointEffect(effects[std::clamp(ip, 0, 4)], position, Color_t(255,255,255));
}

void cbullet_tracer::spawn_particle_hit_at_position(const vec3_t& position, int ip) {
    static constexpr const char* effects[] = {
        "bin/lightning_hit.vpcf", "bin/skull_hit.vpcf", "bin/coins_hit.vpcf",
        "particles/inferno_fx/explosion_incend_air_falling.vpcf"
    };
    SpawnPointEffect(effects[std::clamp(ip, 0, 3)], position, Color_t(255,255,255));
}

void cbullet_tracer::spawn_particle_map_at_position(int ip, Color_t color) {
    static unsigned int index = ~0U;
    static int previousType = -1;
    static uint32_t previousColor = 0;
    static int previousPawn = -1;
    const bool enabled = settings.cfg().visuals.m_mapeffect;
    const uint32_t packed = uint32_t(color.r) | uint32_t(color.g) << 8 | uint32_t(color.b) << 16 | uint32_t(color.a) << 24;
    const int pawn = sdk::m_local_pawn ? sdk::m_local_pawn->get_ref_ehandle().to_int() : -1;
    if (!enabled) {
        StopPointEffect(index); index = ~0U; previousType = -1;
        return;
    }
    if (!sdk::m_local_pawn || !ParticleManagerReady()) return;
    ip = std::clamp(ip, 0, 3);
    if (index != ~0U && previousType == ip && previousColor == packed && previousPawn == pawn && !sdk::on_round_start) return;
    StopPointEffect(index);
    static constexpr const char* effects[] = {
        "bin/falling_snow1.vpcf", "bin/econ_weather_ash.vpcf", "bin/rain.vpcf", "bin/nomove_stars.vpcf"
    };
    const auto position = sdk::m_local_pawn->get_scene_origin() + vec3_t(0.f,0.f,160.f);
    index = SpawnPointEffect(effects[ip], position, color);
    previousType = ip; previousColor = packed; previousPawn = pawn;
}

std::vector<tracer_info> bullets{};

void cbullet_tracer::add_bullet_trace(vec3_t start, vec3_t end, Color_t clr_)
{
	if (!g_interface->game_particle_manager_system)
	{
		try {
            auto** slot = reinterpret_cast<c_game_particle_manager_system**>(CurrentBuild::Address(CurrentBuild::particle_manager));
            g_interface->game_particle_manager_system = slot ? *slot : nullptr;
		}
		catch (...)
		{

		}
	}

    if (!g_interface->game_particle_manager_system || !g_interface->particle_manager) return;
	try {
		tracer_info bullet{};

		particle_effect particle_effect_{};
		particle_effect_.name = "particles/entity/spectator_utility_trail.vpcf";// "particles/entity/spectator_utility_trail.vpcf";
		particle_effect_.pad[0] = 8;
		g_interface->game_particle_manager_system->create_effect_index(&bullet.effect_index, &particle_effect_);
		if (bullet.effect_index == ~0U) return;
		particle_color clr = { float(clr_.r), float(clr_.g), float(clr_.b) };
		g_interface->game_particle_manager_system->SetEffect(bullet.effect_index, 16, &clr, 0);

		bullet.particle_data_ = {};

		auto dir = (end - start);
		auto stage_2 = start + (dir * 0.3f);
		auto stage_3 = start + (dir * 0.5f);

		vec3_t positions_[] = { start,stage_2, stage_3,end };

		for (int i{}; i < sizeof(positions_) / sizeof(vec3_t); i++) {

			particle_information particle_info{};
			particle_info.time = 4.f;
			particle_info.width = 1.5f;
			particle_info.unk2 = 1.f;
			g_interface->game_particle_manager_system->SetEffect(bullet.effect_index, 3, &particle_info, 0);

			bullet.positions = new vec3_t[i + 1];
			bullet.times = new float[i + 1];

			for (int j{}; j < i + 1; j++) {
				bullet.positions[j] = positions_[j];
				bullet.times[j] = 0.015625f/*TICK_INTERVAL*/ * float(j);
			}

			bullet.particle_data_.positions = bullet.positions;
			bullet.particle_data_.times2 = bullet.times;

			g_interface->particle_manager->create_snapshot(&bullet.handle_snapshot_particle);

			g_interface->game_particle_manager_system->fnInitEffect(bullet.effect_index, 0, &bullet.handle_snapshot_particle);
			g_interface->particle_manager->draw(&bullet.handle_snapshot_particle, i + 1, &bullet.particle_data_);

			delete[] bullet.positions;
			delete[] bullet.times;
		}
	}
	catch (...)
	{
		//L_PRINT(LOG_WARNING) << "HUI TEBE";
	}

}
