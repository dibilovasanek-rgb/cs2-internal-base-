#include "world.h"
#include "world_colors.h"
#include "../../../dependencies/config/Include/config.h"
#include "../../../sdk/entity_listener/entity_list.h"
#include "../../../sdk/schema/class/player.h"
#include "../../../sdk/sdk.h"
#include "../../../render/render.h"
#include "../esp.h"
#include "../../rage/autowall.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"
#include "../../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../../sdk/game_class/interfaces/i_global_vars.h"
#include "../../../sdk/math/math.h"
#include "../../rage/ragebot.h"
#include "../../../sdk/game_class/caggregatescene.h"
#include "chams.h"
#include "../../../core/hooks/hooks.h"
#include <chrono>

void c_world::hitmarker(Color_t clr_)
{
	if (!settings.cfg().visuals.b_hitmarker)
		return;
	auto current_time = std::chrono::steady_clock::now();
	auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_hit_time).count();

	if (elapsed_time < 1000) {
		float alpha = 1.0f - (static_cast<float>(elapsed_time) / 1000.0f);
		ImU32 color = ImColor(255, 255, 225, static_cast<int>(alpha * 255));

		auto screen = ImGui::GetIO().DisplaySize;
		ImVec2 center = { screen.x / 2, screen.y / 2 };

		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x - 10, center.y - 10), ImVec2(center.x - 5, center.y - 5), color, 1);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x + 10, center.y - 10), ImVec2(center.x + 5, center.y - 5), color, 1);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x - 10, center.y + 10), ImVec2(center.x - 5, center.y + 5), color, 1);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x + 10, center.y + 10), ImVec2(center.x + 5, center.y + 5), color, 1);
	}

}

void c_world::trigger_hitmarker() {
	last_hit_time = std::chrono::steady_clock::now();
}

namespace {
WorldColors::Cache wall_colors;
struct WallBuffer { std::byte* data; size_t count; };
WallBuffer wall_buffer() {
    static auto** slot = reinterpret_cast<std::byte**>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(SCENESYSTEM_DLL, "4C 8B 35 ? ? ? ? C1 E0 05 48 63 D0 49 03 56 18 49 3B 56 08 76 ? 49 8B CE FF 15 ? ? ? ? 84 C0 75 ? 49 8B F4")), 3));
    auto* queue = slot ? *slot : nullptr;
    if (!queue) return {};
    auto* data = *reinterpret_cast<std::byte**>(queue + 0x18);
    auto end = *reinterpret_cast<uintptr_t*>(queue + 8), begin = reinterpret_cast<uintptr_t>(data);
    return begin && end > begin && end - begin <= (1u << 27) ? WallBuffer{data, (end - begin) / 0x20} : WallBuffer{};
}
bool wall_range(void* aggregate, size_t& first, size_t& count) {
    auto* data = aggregate ? *reinterpret_cast<std::byte**>(static_cast<std::byte*>(aggregate) + 8) : nullptr;
    if (!data) return false;
    const int n = *reinterpret_cast<int*>(data + 4), index = *reinterpret_cast<int*>(data + 0x38);
    if (n <= 0 || n > (1 << 20) || index < 0 || index > (1 << 22) - n) return false;
    first = index; count = n; return true;
}
}
void c_world::update_walls(void* aggregate) {
    size_t first{}, count{};
    if (!wall_range(aggregate, first, count)) return;
    const auto buffer = wall_buffer(); const auto color = settings.cfg().worldModulation.worldColor;
    wall_colors.Range(buffer.data, buffer.count, first, count, settings.cfg().worldModulation.b_worldModulation, {color.r, color.g, color.b});
}
void c_world::refresh_walls() {
    const auto buffer = wall_buffer(); const auto color = settings.cfg().worldModulation.worldColor;
    wall_colors.Refresh(buffer.data, buffer.count, settings.cfg().worldModulation.b_worldModulation, {color.r, color.g, color.b});
}
void c_world::reset_walls() { wall_colors.Reset(); }
void c_world::skybox() {}
void c_world::skybox(c_envsky*) {}

void c_world::lighting(C_SceneLightObject* light_object)
{
	Color_t lighintg_color = { 255,0,255 };// C_GET(Color_t, Vars.colLightning); /*g_cfg->world.m_lighting;*/

	light_object->r = lighintg_color.r / 255.0f * 1;//C_GET(float, Vars.flLightingIntensity);
	light_object->g = lighintg_color.g / 255.0f * 1;//C_GET(float, Vars.flLightingIntensity);
	light_object->b = lighintg_color.b / 255.0f * 1;//C_GET(float, Vars.flLightingIntensity);
}

void c_world::exposure(c_player_pawn* pawn)
{
	if (!g_interface->m_engine->is_in_game())
		return;
	g_hook->last_func = "exposure";
	static const auto update_exposure = reinterpret_cast<void(__fastcall*)(c_player_camera_services*, int)>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ?"));

	

	float exposure = settings.cfg().worldModulation.fl_exposure;//static_cast<float>(C_GET(float, Vars.flExposure) /*g_cfg->world.m_exposure*/);

	

	if (!pawn || !pawn->m_camera_services())
		return;

	c_player_camera_services* camera_service = pawn->m_camera_services();
	if (!camera_service)
		return;

	c_postproccesing* post_processing_volume = reinterpret_cast<c_postproccesing*>(g_interface->m_game_resource_service->get_game_entity_system()->get(camera_service->m_active_post_processing_volume().get_entry_index()));
	if (!post_processing_volume)
		return;

	

	this->exposure(post_processing_volume);
	if (update_exposure) update_exposure(camera_service, 0);
}

void c_world::exposure(c_postproccesing* post_processing)
{
	float exposure = settings.cfg().worldModulation.fl_exposure * 0.01f;//C_GET(float, Vars.flExposure) * 0.01f;

	if (!post_processing)
		return;
	try {
		post_processing->ExposureControl() = true;
		post_processing->FadeSpeedDown() = post_processing->FadeSpeedUp() = 0;

		post_processing->MinExposure() = post_processing->MaxExposure() = exposure;
	}
	catch (...) {}
}

void c_world::run_onshot() {
    if (!settings.cfg().visuals.b_onhits || !g_interface->m_globalVars || !g_render->m_draw_list) return;
    auto rotate = [](const vec3_t& v, const vec4_t& q) {
        const float dot = q.x*v.x + q.y*v.y + q.z*v.z;
        const float scale = q.w*q.w - q.x*q.x - q.y*q.y - q.z*q.z;
        return vec3_t{2.f*dot*q.x + scale*v.x + 2.f*q.w*(q.y*v.z-q.z*v.y),
                      2.f*dot*q.y + scale*v.y + 2.f*q.w*(q.z*v.x-q.x*v.z),
                      2.f*dot*q.z + scale*v.z + 2.f*q.w*(q.x*v.y-q.y*v.x)};
    };
    for (auto it=hit_capibara.begin();it!=hit_capibara.end();) {
        if (it->time < g_interface->m_globalVars->flCurrentTime) { it=hit_capibara.erase(it); continue; }
        const auto color=settings.cfg().visuals.onhit_color.GetU32();
        for (int i=0;i<19;++i) {
            auto* box=it->hitbox_data.hitbox_array[i].hitbox;
            if (!box) continue;
            const auto& bone=it->hitbox_position.hitbox_array[i];
            const auto start=bone.position+rotate(box->m_vec_min*bone.scale,bone.rotate);
            const auto end=bone.position+rotate(box->m_vec_maxs*bone.scale,bone.rotate);
            vec2_t a{},b{},radius_point{};
            if (!g_render->world_to_screen(start,a) || !g_render->world_to_screen(end,b)) continue;
            float radius=2.f;
            if (box->m_shape_radius>0.f && g_render->world_to_screen(start+vec3_t{box->m_shape_radius*bone.scale,0.f,0.f},radius_point)) {
                const auto dx=radius_point.x-a.x,dy=radius_point.y-a.y;
                radius=std::clamp(std::sqrt(dx*dx+dy*dy),1.f,64.f);
            }
            g_render->m_draw_list->AddLine(a.im_vec2(),b.im_vec2(),color,radius*2.f);
            g_render->m_draw_list->AddCircleFilled(a.im_vec2(),radius,color);
            g_render->m_draw_list->AddCircleFilled(b.im_vec2(),radius,color);
        }
        ++it;
    }
}

void c_world::DrawScopeOverlay(ImDrawList* draw)
{
	g_hook->last_func = "DrawScopeOverlay";
	static float animationProgress = 0.0f;
	static bool wasScoped = false;

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive())
		return;

	bool isScoped = sdk::m_local_pawn->m_is_scoped();

	float animationSpeed = 0.05f;
	if (isScoped)
		animationProgress = min(1.0f, animationProgress + animationSpeed);
	else
		animationProgress = max(0.0f, animationProgress - animationSpeed);

	if (animationProgress <= 0.0f)
		return;

	ImVec2 screen = ImGui::GetIO().DisplaySize;
	ImVec2 center = ImVec2(screen.x * 0.5f, screen.y * 0.5f);

	int offset = screen.y / 120;
	int size = (screen.y / 2) * 30/*<- ��� ������*/ * .01f - offset;
	size = min(size, int(screen.y / 2) - offset - 1);
	int gap = 5;
	Color_t color = Color_t(0, 0, 0);
	switch (settings.cfg().misc.crosshairs)
	{
	case 0:
		draw->AddLine(ImVec2(0, screen.y / 2), ImVec2(screen.x, screen.y / 2), color.GetU32());
		draw->AddLine(ImVec2(screen.x / 2, 0), ImVec2(screen.x / 2, screen.y), color.GetU32());
		break;

	case 1:
	{
		float animOffset = 100.0f * animationProgress;
		int fadeAlpha = (int)(255 * animationProgress);

		draw->AddLine({ center.x - animOffset - 10, center.y }, { center.x - 10, center.y }, ImColor(255, 255, 255, fadeAlpha), 1);
		draw->AddLine({ center.x - 10, center.y }, { center.x - 5, center.y }, ImColor(255, 255, 255, fadeAlpha / 2), 1);
		draw->AddLine({ center.x + 10, center.y }, { center.x + animOffset + 10, center.y }, ImColor(255, 255, 255, fadeAlpha), 1);
		draw->AddLine({ center.x + 10, center.y }, { center.x + 5, center.y }, ImColor(255, 255, 255, fadeAlpha / 2), 1);
		draw->AddLine({ center.x, center.y - animOffset - 10 }, { center.x, center.y - 10 }, ImColor(255, 255, 255, fadeAlpha), 1);
		draw->AddLine({ center.x, center.y - 10 }, { center.x, center.y - 5 }, ImColor(255, 255, 255, fadeAlpha / 2), 1);
		draw->AddLine({ center.x, center.y + 10 }, { center.x, center.y + animOffset + 10 }, ImColor(255, 255, 255, fadeAlpha), 1);
		draw->AddLine({ center.x, center.y + 10 }, { center.x, center.y + 5 }, ImColor(255, 255, 255, fadeAlpha / 2), 1);
		break;
	}
	case 2:
	{
		float animOffset = 100.0f * animationProgress;
		int fadeAlpha = (int)(255 * animationProgress);
		ImColor clr = ImColor(255, 255, 255, fadeAlpha);
		ImColor clr2 = clr;
		clr2.Value.w = 0;

		g_render->rect_filled_multi_color(draw, ImVec2(screen.x * 0.5f, screen.y * 0.5f - gap - size), ImVec2(screen.x * 0.5f + 1, screen.y * 0.5f - gap), clr2, clr2, clr, clr); // top
		g_render->rect_filled_multi_color(draw, ImVec2(screen.x * 0.5f, screen.y * 0.5f + gap), ImVec2(screen.x * 0.5f + 1, screen.y * 0.5f + gap + size), clr, clr, clr2, clr2); // bottom
		g_render->rect_filled_multi_color(draw, ImVec2(screen.x * 0.5f - gap - size, screen.y * 0.5f), ImVec2(screen.x * 0.5f - gap, screen.y * 0.5f + 1), clr2, clr, clr2, clr); // left
		g_render->rect_filled_multi_color(draw, ImVec2(screen.x * 0.5f + gap, screen.y * 0.5f), ImVec2(screen.x * 0.5f + gap + size, screen.y * 0.5f + 1), clr, clr2, clr, clr2); // right
		break;
	}

	default:
		draw->AddLine(ImVec2(0, screen.y / 2), ImVec2(screen.x, screen.y / 2), color.GetU32());
		draw->AddLine(ImVec2(screen.x / 2, 0), ImVec2(screen.x / 2, screen.y), color.GetU32());
		break;
	}
}

void c_world::DrawBomb(ImDrawList* draw) {
	g_hook->last_func = "DrawBomb";
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (settings.cfg().visuals.b_bomb_esp) {
		auto entity_c4 = g_entlistener->get("C_PlantedC4");
		if (entity_c4.empty())
			return;
		for (auto entity : entity_c4)
		{
			if (!entity)
				continue;
			c_planted_c4* c4 = reinterpret_cast<c_planted_c4*>(entity);
			if (!c4)
				continue;

			if (c4->is_c4_activated()) {
				auto gamenode = c4->m_game_scene_node();
				if (!gamenode)
					continue;
				vec3_t pos = gamenode->m_vec_abs_origin();
				vec2_t WTSpos;
				if (!g_render->world_to_screen(pos, WTSpos))
					continue;
				Color_t color = { 177, 0,0 };
				if (c4->if_bomb_defused())
					color = { 0,177,0 };
				g_render->text({ WTSpos.x, WTSpos.y - 10 }, color, center_outline, g_render->font_weapons, "o", 15);

			}
		}
	}
}

void c_world::penitrations(ImDrawList* draw)
{
	g_hook->last_func = "penitrations";
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (settings.cfg().visuals.b_show_penetration) {

		if (!sdk::m_local_pawn || !g_ragebot->m_best_target || !g_ragebot->m_best_target->m_lag_record)
			return;

		c_player_pawn* pawn_ent = g_ragebot->m_best_target->m_pawn;
		if (!pawn_ent)
			return;
		//g_ragebot->debug(pawn_ent);

		if (pawn_ent == sdk::m_local_pawn)
			return;

		auto local_ent_base = g_interface->m_game_resource_service->get_game_entity_system()->get(sdk::m_local_pawn->get_ref_ehandle());
		if (!local_ent_base)
			return;

		auto weapon_data = sdk::m_local_pawn->ActiveWeapon()->m_weapon_data();
		if (!weapon_data || (uintptr_t)weapon_data <= 0x111)
			return;

		if (!pawn_ent->m_game_scene_node())
			return;

		auto game_scene_node = pawn_ent->m_game_scene_node();
		if (!game_scene_node)
			return;

		auto* skeleton = game_scene_node->get_skeleton_instance();
        auto* bones = skeleton ? skeleton->m_model_state().bones : nullptr;
        const int count = skeleton ? skeleton->get_bone_count() : 0;
        if (!bones || count <= 0) return;

        for (const auto& link : skeleton_links) {
            if (link[0] >= count) continue;
			vec3_t bonPos = { 0,0,0 };
			vec2_t WTSbonPos = { 0,0 };

			if (!g_ragebot->m_best_target || !g_ragebot->m_best_target->m_lag_record)
				continue;

			bonPos = bones[link[0]].position;

			float dmg = g_autowall->penetration_damage(
				sdk::m_local_pawn->get_eye_pos(),
				bonPos,
				local_ent_base,
				sdk::m_local_pawn,
				pawn_ent,
				pawn_ent,
				pawn_ent->m_item_services()->m_has_heavy_armor(),
				pawn_ent->m_team_num(),
				weapon_data
			);

			float dist = sdk::m_local_pawn->get_eye_pos().dist_to(bonPos);
			float radius = 180 / (1.0f + dist * 0.1f);

			if (/*settings.cfg().ragebot.b_ragebot ? */dmg >= settings.cfg().ragebot.r_min_damage/* : dmg >= settings.cfg().ragebot.semi_r_min_damage*/) {
				if (g_render->world_to_screen(bonPos, WTSbonPos))
				    draw->AddCircleFilled(WTSbonPos.im_vec2(), radius, settings.cfg().visuals.penetration_color.GetU32(), 50);
			}
		}
	}
}

const char* get_icon(const std::string& weapon)
{
	std::map<std::string, const char*> gunIcons = {
		{ "Nova", "T" },
		{ "AK-47", "A" },
		{ "AWP", "C" },
		{ "M4A4", "M" },
		{ "Desert Eagle", "F" },
		{ "MP9", "R" },
		{ "UMP-45", "b" },
		{ "Glock-18", "g" },
	};

	/*if (sdk::m_local_pawn->m_team_num() == 2 && weapon.compare("weapon_knife"))
		return "]";
	else if (sdk::m_local_pawn->m_team_num() == 1 && weapon.compare("weapon_knife"))
		return "[";*/
	gunIcons[("Stock Knife")] = "]";
	gunIcons[("Stock Knife")] = "[";
	gunIcons[("Desert Eagle")] = "A";
	gunIcons[("Dual Berettas")] = "B";
	gunIcons[("Five-SeveN")] = "C";
	gunIcons[("Glock-18")] = "D";
	gunIcons[("P2000")] = "E";
	gunIcons[("P250")] = "F";
	gunIcons[("USP-S")] = "G";
	gunIcons[("Tec-9")] = "H";
	gunIcons[("CZ75-Auto")] = "I";
	gunIcons[("R8 Revolver")] = "J";
	gunIcons[("MAC-10")] = "K";
	gunIcons[("UMP-45")] = "L";
	gunIcons[("PP-Bizon")] = "M";
	gunIcons[("MP7")] = "N";
	gunIcons[("MP9")] = "O";
	gunIcons[("MP5-SD")] = "x";
	gunIcons[("P90")] = "P";
	gunIcons[("Galil AR")] = "Q";
	gunIcons[("FAMAS")] = "R";
	gunIcons[("M4A4")] = "S";
	gunIcons[("M4A1-S")] = "T";
	gunIcons[("AUG")] = "U";
	gunIcons[("SG 553")] = "V";
	gunIcons[("AK-47")] = "W";
	gunIcons[("G3SG1")] = "X";
	gunIcons[("SCAR-20")] = "Y";
	gunIcons[("AWP")] = "Z";
	gunIcons[("SSG 08")] = "a";
	gunIcons[("XM1014")] = "b";
	gunIcons[("Sawed-Off")] = "c";
	gunIcons[("MAG-7")] = "d";
	gunIcons[("Nova")] = "e";
	gunIcons[("Negev")] = "f";
	gunIcons[("M249")] = "g";
	gunIcons[("Taser")] = "h";
	gunIcons[("Flashbang")] = "i";
	gunIcons[("High Explosive Grenade")] = "j";
	gunIcons[("Smoke Grenade")] = "k";
	gunIcons[("Molotov")] = "l";
	gunIcons[("Decoy Grenade")] = "m";
	gunIcons[("Incendiary Grenade")] = "n";
	gunIcons[("C4 Explosive")] = "o";
	gunIcons[("Defuse Kit")] = "r";

	auto it = gunIcons.find(weapon);
	if (it != gunIcons.end())
	{
		return it->second;
	}

	return "";
}


void c_world::active_grande(ImDrawList* draw)
{
	g_hook->last_func = "active_grande";
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;
	if (settings.cfg().visuals.b_weapon_in_world)
	{
		for (int i{}; i <= g_interface->m_game_resource_service->get_game_entity_system()->get_highest_entity_index(); i++) {
			auto entity = g_interface->m_game_resource_service->get_game_entity_system()->get(i);

			if (!entity)
				continue;

			if (!entity->find_class(fnv1a::hash("C_SmokeGrenadeProjectile")))
				continue;

			c_smoke_grenade_projectile* smoke = reinterpret_cast<c_smoke_grenade_projectile*>(entity);

			if (!smoke)
				continue;

			if (smoke->m_did_smoke_effect())
			{
				vec2_t out{};
				const float max_time = 22.f;
				if (!g_render->world_to_screen(smoke->get_smoke_pos(), out))
					return;
				float spawn_time = INTERVAL_PER_TICK * smoke->get_smoke_tick();
				float current_time = (spawn_time + max_time) - g_interface->m_globalVars->flCurrentTime;
				ImVec2 pos = { out.x, out.y };

				if (!sdk::m_local_pawn)
					return;
				float distance = (sdk::m_local_pawn->get_eye_pos() - smoke->get_smoke_pos()).length();

				const float base_width = 25.0f;
				const float base_height = 2.0f;
				const float base_padding = 4.0f;

				float distance_scale = 1.0f;
				if (distance > 100.f) {
					distance_scale = (100.f / distance > 0.5f) ? (100.f / distance) : 0.5f;
				}

				float width = base_width * distance_scale;
				float height = base_height * distance_scale;
				float padding = base_padding * distance_scale;

				float filled_width = (current_time / max_time) * width;

				draw->AddRectFilled(
					{ pos.x - padding, pos.y - padding },
					{ pos.x + width + padding, pos.y + height + padding },
					ImColor(0, 0, 0, 150)
				);
				draw->AddRectFilled(
					{ pos.x, pos.y },
					{ pos.x + filled_width, pos.y + height },
					ImColor(255, 255, 255)
				);

				g_render->text(vec2_t(out.x + 6, out.y - 10), Color_t(255, 255, 255, 255), shadows, g_render->font_visitor, "SMOKE", 10);

			}
		}
	}
}

void c_world::test(ImDrawList* draw)
{
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!sdk::m_local_pawn)
		return;

	if (!sdk::m_local_pawn->m_is_alive())
		return;

	ImVec2 screen_size = ImGui::GetIO().DisplaySize;
	vec2_t center = vec2_t(screen_size.x / 2, screen_size.y / 2);
	draw->AddCircleFilled(ImVec2(center.x, center.y), 2, settings.cfg().visuals.for_test.GetU32());
}

void c_world::weapon_esp()
{
	g_hook->last_func = "weapon_esp";
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;
	if (settings.cfg().visuals.b_weapon_in_world)
	{
		for (auto& it : g_esp->m_dropped_weapons) {
			vec2_t out;
			if (it.is_invalid)
				continue;
			if (!g_render->world_to_screen(it.weapon->m_game_scene_node()->m_vec_abs_origin(), out))
				continue;

			float multiplicator = 1.f;

			if (sdk::m_local_pawn->m_is_alive()) {
				auto delta_ = sdk::m_local_pawn->m_game_scene_node()->m_vec_abs_origin().dist_to(it.abs_origin);
				if (delta_ > 500.f) {
					auto delta = 400.f - std::clamp(delta_ - 500.f, 0.f, 400.f);

					multiplicator = delta / 400.f;
				}
			}
			g_render->text(vec2_t(out.x, out.y + 5), Color_t(255, 255, 255, 255 - (int)multiplicator), shadows, g_render->font_chika, it.weapon_name.c_str(), 9);
			g_render->text(vec2_t(out.x, out.y - 4), Color_t(255, 255, 255, 255 - (int)multiplicator), shadows, g_render->font_weapons, get_icon(it.weapon_name), 10);
		}
	}
}