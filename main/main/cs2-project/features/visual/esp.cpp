#include "esp.h"
#include "../../sdk/math/vector.h"
#include "../../render/render.h"
#include "../../sdk/entity_listener/entity_list.h"
#include "../../sdk/sdk.h"
#include "../../core/menu/menu.h"
#include "../../sdk/schema/class/weapon.h"
#include "additionally.h"
#include "../../dependencies/config/Include/config.h"
#include "../../dependencies/imgui/imgui_internal.h"
#include "../../sdk/math/math.h"
#include "../rage/ragebot.h"
#include "../rage/autowall.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../sdk/game_class/interfaces/i_global_vars.h"	
#include "../../sdk/game_class/interfaces/steam/isteamclient.h"
#include "../../sdk/game_class/interfaces/steam/isteamfriend.h"
#include "../../sdk/game_class/interfaces/steam/isteamutils.h"
#include "../../core/hooks/hooks.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"

const char* c_esp::GunIcon(const std::string& weapon)
{
	std::map<std::string, const char*> gunIcons = {
		{ "nova", "T" },
		{ "ak47", "A" },
		{ "awp", "C" },
		{ "m4a4", "M" },
		{ "deagle", "F" },
		{ "mp9", "R" },
		{ "ump45", "b" },
		{ "glock", "g" },
	};

	/*if (sdk::m_local_pawn->m_team_num() == 2 && weapon.compare("weapon_knife"))
		return "]";
	else if (sdk::m_local_pawn->m_team_num() == 1 && weapon.compare("weapon_knife"))
		return "[";*/
	gunIcons[("weapon_knife")] = "]";
	gunIcons[("weapon_knife")] = "[";
	gunIcons[("weapon_deagle")] = "A";
	gunIcons[("weapon_elite")] = "B";
	gunIcons[("weapon_fiveseven")] = "C";
	gunIcons[("weapon_five-seven")] = "C";
	gunIcons[("weapon_glock")] = "D";
	gunIcons[("weapon_hkp2000")] = "E";
	gunIcons[("weapon_p2000")] = "E";
	gunIcons[("weapon_p250")] = "F";
	gunIcons[("weapon_usp_silencer")] = "G";
	gunIcons[("weapon_tec9")] = "H";
	gunIcons[("weapon_cz75a")] = "I";
	gunIcons[("weapon_revolver")] = "J";
	gunIcons[("weapon_mac10")] = "K";
	gunIcons[("weapon_mac-10")] = "K";
	gunIcons[("weapon_ump45")] = "L";
	gunIcons[("weapon_bizon")] = "M";
	gunIcons[("weapon_mp7")] = "N";
	gunIcons[("weapon_mp9")] = "O";
	gunIcons[("weapon_mp5sd")] = "x";
	gunIcons[("weapon_p90")] = "P";
	gunIcons[("weapon_galilar")] = "Q";
	gunIcons[("weapon_galil")] = "Q";
	gunIcons[("weapon_famas")] = "R";
	gunIcons[("weapon_m4a4")] = "S";
	gunIcons[("weapon_m4a1_silencer")] = "T";
	gunIcons[("weapon_m4a1")] = "T";
	gunIcons[("weapon_aug")] = "U";
	gunIcons[("weapon_sg556")] = "V";
	gunIcons[("weapon_ak47")] = "W";
	gunIcons[("weapon_g3sg1")] = "X";
	gunIcons[("weapon_scar20")] = "Y";
	gunIcons[("weapon_awp")] = "Z";
	gunIcons[("weapon_ssg08")] = "a";
	gunIcons[("weapon_ssg-08")] = "a";
	gunIcons[("weapon_xm1014")] = "b";
	gunIcons[("weapon_sawedoff")] = "c";
	gunIcons[("weapon_mag7")] = "d";
	gunIcons[("weapon_nova")] = "e";
	gunIcons[("weapon_negev")] = "f";
	gunIcons[("weapon_m249")] = "g";
	gunIcons[("weapon_taser")] = "h";
	gunIcons[("weapon_flashbang")] = "i";
	gunIcons[("weapon_hegrenade")] = "j";
	gunIcons[("weapon_smokegrenade")] = "k";
	gunIcons[("weapon_molotov")] = "l";
	gunIcons[("weapon_decoy")] = "m";
	gunIcons[("weapon_incgrenade")] = "n";
	gunIcons[("weapon_c4")] = "o";
	gunIcons[("defuse kit")] = "r";

	auto it = gunIcons.find(weapon);
	if (it != gunIcons.end())
	{
		return it->second;
	}

	return "";
}

c_esp::box c_esp::get_box(c_player_pawn* entity)
{
	constexpr c_esp::box rect{};

	auto* scene = entity ? entity->m_game_scene_node() : nullptr;
    if (!scene) return {};
    auto abs_origin = scene->m_vec_abs_origin();

	auto m_collision = entity->m_p_collision();
	if (!m_collision)
		return {};

	auto min = m_collision->m_vec_mins() + abs_origin;
	auto max = m_collision->m_vec_maxs() + abs_origin;

	const vec3_t points[] =
	{
		vec3_t(min.x, min.y, min.z),
		vec3_t(min.x, max.y, min.z),
		vec3_t(max.x, max.y, min.z),
		vec3_t(max.x, min.y, min.z),
		vec3_t(max.x, max.y, max.z),
		vec3_t(min.x, max.y, max.z),
		vec3_t(min.x, min.y, max.z),
		vec3_t(max.x, min.y, max.z)
	};

	vec2_t screen_points[8] = {};
	for (int i = 0; i < 8; i++)
		if (!g_render->world_to_screen(points[i], screen_points[i]))
			return {};

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (auto i = 1; i < 8; i++)
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;

		if (top < screen_points[i].y)
			top = screen_points[i].y;

		if (right < screen_points[i].x)
			right = screen_points[i].x;

		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return { left, bottom, right, top, true };
}

vec2_t RotateVertex(const vec2_t& p, const vec2_t& v, float angle)
{
	float c = std::cos(math::deg2rad(angle));
	float s = std::sin(math::deg2rad(angle));

	return {
		p.x + (v.x - p.x) * c - (v.y - p.y) * s,
		p.y + (v.x - p.x) * s + (v.y - p.y) * c
	};
}

ImTextureID c_esp::get_avatar(uint64_t steam_id)
{
	if (avatar_cache.find(steam_id) != avatar_cache.end())
		return avatar_cache[steam_id];

	int iImage = g_interface->m_steam_friends->GetLargeFriendAvatar(steam_id);
	if (iImage == -1)
		return {};

	uint32 uAvatarWidth, uAvatarHeight;
	if (!g_interface->m_steam_utils->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return {};

	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	std::vector<uint8> avatar_rgba(uImageSizeInBytes);
	if (!g_interface->m_steam_utils->GetImageRGBA(iImage, avatar_rgba.data(), avatar_rgba.size()))
		return {};

	ImTextureID avatar = reinterpret_cast<ImTextureID>(g_render->load_texture_from_buffer(avatar_rgba.data(), uAvatarWidth, uAvatarHeight));
	avatar_cache[steam_id] = avatar;
	return avatar;
}

std::vector<std::string> get_flags(c_player_pawn* player, c_player_controller* player_controller)
{
	std::vector<std::string> m_flags;
	if (settings.cfg().visuals.m_flags[0])
	{
		if (player->has_c4())
		{
			m_flags.push_back("C4");
		}
	}

	if (settings.cfg().visuals.m_flags[1])
	{
		if (player->m_item_services()->m_has_defuser())
		{
			m_flags.push_back("KITS");
		}
	}

	if (settings.cfg().visuals.m_flags[2])
	{
		if (player->m_armor_value() > 0)
		{
			if (player->m_item_services()->m_has_helmet())
				m_flags.push_back("HK");
			else
				m_flags.push_back("H");
		}
	}

	//if (settings.cfg().visuals.m_flags[3])
	//{
	//	if (player->m_flash_duration() > 0)
	//	{
	//		m_flags.push_back("FLASHED");
	//	}
	//}

	if (settings.cfg().visuals.m_flags[4])
	{
		if (player->m_weapon_services() && player->m_weapon_services()->weapon_owns_this_type("weapon_taser"))
		{
			m_flags.push_back("ZEUS");
		}
	}

	if (settings.cfg().visuals.m_flags[5])
	{
		if (player->m_is_defusing())
		{
			m_flags.push_back("DEFUSER");
		}
	}

	if (settings.cfg().visuals.m_flags[6])
	{
		if (player->m_gun_game_immunity() > 0)
		{
			m_flags.push_back("IMMUNE");
		}
	}

	if (settings.cfg().visuals.m_flags[7])
	{
		if (player_controller->m_money_service())
		{
			m_flags.push_back(std::string("$") + std::to_string(player_controller->m_money_service()->m_get_money()));
		}
	}

	return m_flags;
}


void c_esp::store_players()
{
	if (!settings.cfg().visuals.b_esp)
		return;
	const std::unique_lock<std::mutex> m(m_player_mutex);

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected()) {
		m_player_map.clear();
		return;
	}
	if (!sdk::m_local_pawn)
		return;

	std::erase_if(m_player_map, [](const auto& kv)
		{
			auto& player_info = kv.second;
			if (player_info.m_valid)
			{
				int current_index = player_info.m_handle == 0xffffffff ? 0x7fff : player_info.m_handle & 0x7fff;

				// in case when handle is still valid
				// entity will be found
				// otherwise it will be fucked OR it will be find wrong entity
				c_base_entity* entity = g_interface->m_game_resource_service->get_game_entity_system()->get(current_index);

				if (entity)
				{
					c_player_controller* player_controller = reinterpret_cast<c_player_controller*>(entity);

					if (!player_controller)
						return true;

					// if entity is wrong (for example it magically became some unknown prop)
					// erase it
					return !player_controller->is_player_pawn();
				}
				else
					return true;
			}

			return false;
		});

	// player esp
	for (c_entity_instance* enemy : g_entlistener->get("CCSPlayerController"))
	{
		c_player_controller* player_controller = reinterpret_cast<c_player_controller*>(enemy);
		if (!player_controller) continue;
		c_player_pawn* player_pawn = player_controller->get_player_pawn();
		if (!player_controller || !player_pawn)
			continue;

		bool is_local_player = (sdk::m_local_pawn && (player_pawn == sdk::m_local_pawn));
		int handle = player_pawn->get_ref_ehandle().to_int();
		int team_num = player_pawn->m_team_num();
		auto player_ptr = m_player_map.find(handle);
		bool is_teammate = (true/*g_cfg->visuals.m_player_esp.m_teammates*/ && (sdk::m_local_pawn->m_team_num() == team_num));

		if (is_local_player || is_teammate || !player_pawn->m_is_alive() || !player_controller->m_pawn_is_alive()) {

			if (player_ptr != m_player_map.end())
				m_player_map.erase(player_ptr);

			continue;
		}

		if (player_ptr == m_player_map.end()) {
			player_ptr = m_player_map.emplace(handle, player_info_t{}).first;
		}

		player_info_t& player_info = player_ptr->second;
		player_info.m_valid = true;
		player_info.m_handle = handle;
		player_info.m_health = player_pawn->m_health();
		player_info.m_box = get_box(player_pawn);
		player_info.m_name = player_controller->m_sanitized_player_name();
		player_info.m_pawn = player_pawn;
		player_info.m_controller = player_controller;
		player_info.m_flags = get_flags(player_pawn, player_controller);

		c_weapon_base* active_weapon = player_pawn->ActiveWeapon();
		if (active_weapon) {
			c_cs_weapon_base_vdata* weapon_data = active_weapon->m_weapon_data();
			if (weapon_data) {
				player_info.m_ammo = active_weapon->m_iclip1();
				player_info.m_max_ammo = weapon_data->m_imaxclip1();
				player_info.m_weapon_name = weapon_data->m_weapon_name();
			}
		}
	}
}

void c_esp::spectator(c_player_controller* player_controller, c_player_pawn* player_pawn)
{
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!settings.cfg().visuals.b_esp || !settings.cfg().visuals.b_spectators)
		return;

	std::vector<c_specs> specs_temp{};

	auto ObserverPawn = player_controller->get_observer_pawn();
	if (ObserverPawn == nullptr)
		return;
	auto obs_service = ObserverPawn->get_observer_service(); // выдаёт значение 0xffffffffffffffff
	if (!obs_service)
		return;

	if (!obs_service->get_observer_targer().is_valid())
		return;

	c_player_pawn* PPawn = (c_player_pawn*)g_interface->m_game_resource_service->get_game_entity_system()->get(obs_service->get_observer_targer().get_entry_index());
	if (PPawn == sdk::m_local_pawn)
	{
		auto qwe = g_interface->m_game_resource_service->get_game_entity_system()->get(player_pawn->get_controller_handle().get_entry_index());
		if (qwe == nullptr)
			return;
		c_player_controller* pPlayer2 = reinterpret_cast<c_player_controller*>(qwe);
		if (pPlayer2 == nullptr)
			return;

		if (!player_pawn->m_is_alive())
		{
			auto name = std::string(pPlayer2->m_sanitized_player_name());
			ImTextureID texture_avatar = get_avatar(player_controller->m_steam_id());

			specs_temp.push_back({ name ,texture_avatar });
		}
		//ImGui::Text(pPlayer2->GetPlayerName());	
	}


	for (const auto& spec : specs_temp)
	{
		if (!bMainWindowOpened) {
			ImGui::PushFont(g_render->font_big_chika);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowTitleAlign = ImVec2(0, 0.5f); // Увеличиваем высоту TitleBar
			ImGui::SetNextWindowBgAlpha(0.45f);
			ImGui::Begin("spectators", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::SetWindowSize({ 200,100 });
			ImGui::PopFont();
			ImGui::Image(spec.image, ImVec2(20, 20));
			ImGui::SameLine();
			ImGui::PushFont(g_fonts->tab);
			ImGui::Text(spec.name.c_str());
			ImGui::End();

			ImGui::PopFont();
			ImGui::PopStyleColor(2);
			//g_render->spectator(spec.name.c_str(), spec.image, offset);
			////if(specs_temp.size() != 1)
			//	offset += 50;
		}
	}
}

bool is_player_visible(c_player_pawn* local_pawn, c_player_pawn* target_pawn)
{
	if (!local_pawn || !target_pawn)
		return false;

	vec3_t local_eyes = local_pawn->get_eye_pos();
	vec3_t target_pos = target_pawn->get_scene_origin();

	vec3_t points[] = {
		target_pos + vec3_t(0, 0, 64.f), target_pos + vec3_t(0, 0, 36.f), target_pos                       
	};

	game_trace_t trace;
	TraceFilter_t filter(0x1C3003, local_pawn, nullptr, 4);
	Ray_t ray;

	for (const auto& point : points)
	{
		g_interface->m_game_trace->trace_shape(&ray, local_eyes, point, &filter, &trace);
		if (trace.HitEntity == target_pawn && trace.IsVisible())
			return true;
	}

	return false;
}

void c_esp::draw_esp()
{
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!sdk::m_local_pawn || !sdk::m_local_controller)
		return;

	g_hook->last_func = "draw_esp";
	const std::unique_lock<std::mutex> m(m_player_mutex);

	auto draw_list = g_render->m_draw_list;
	if (!g_interface->m_globalVars || !draw_list) return;
	float current_time = g_interface->m_globalVars->flCurrentTime;

	for (auto& [_, player_info] : m_player_map)
	{
		if (!player_info.m_valid || !player_info.m_pawn)
			continue;

		bool is_visible = is_player_visible(sdk::m_local_pawn, player_info.m_pawn);

		float sound_time = player_info.m_pawn->m_emit_sound_time();
		bool is_making_sound = sound_time > current_time - 1.0f;

		bool should_draw = false;

		if (settings.cfg().visuals.b_sound_only_esp)
		{
			should_draw = is_visible || is_making_sound;
		}
		else
		{
			should_draw = true;
		}

		if (settings.cfg().visuals.b_onlyvisible && !is_visible)
			should_draw = false;

		if (!should_draw)
			continue;

		if (settings.cfg().visuals.b_oof)
		{
			g_hook->last_func = "oof";
			if (!g_interface->m_globalVars)
				continue;

			vec3_t view_origin, target_pos, delta;
			vec2_t screen_pos, offscreen_pos;
			float leeway_x, leeway_y, radius, offscreen_rotation;
			bool is_on_screen;
			std::array<vec2_t, 3> verts;

			if (sdk::ScreenSize.x == 0)
				continue;

			if (!sdk::angleStor.IsValid())
				continue;

			auto get_offscreen_data = [](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation)
				{
					vec3_t view_angles = { sdk::angleStor.x,sdk::angleStor.y,sdk::angleStor.z };
					vec3_t fwd, right, up(0.f, 0.f, 1.f);
					float front, side, yaw_rad, sa, ca;

					math::angle_vectors(view_angles, fwd);
					fwd.z = 0.f;
					fwd.normalize();

					right = up.cross(fwd);
					front = delta.dot(fwd);
					side = delta.dot(right);

					out_offscreen_pos.x = radius * -side;
					out_offscreen_pos.y = radius * -front;

					out_rotation = math::rad2deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::_PI);

					yaw_rad = math::deg2rad(-out_rotation);
					sa = std::sin(yaw_rad);
					ca = std::cos(yaw_rad);

					out_offscreen_pos.x = (int)((sdk::ScreenSize.x / 2.f) + (radius * sa));
					out_offscreen_pos.y = (int)((sdk::ScreenSize.y / 2.f) - (radius * ca));
				};

			auto scene_node = player_info.m_pawn->m_game_scene_node();
			if (!scene_node)
				continue;
			target_pos = scene_node->m_vec_abs_origin();

			is_on_screen = g_render->world_to_screen(target_pos, screen_pos);

			leeway_x = sdk::ScreenSize.x / 18.f;
			leeway_y = sdk::ScreenSize.y / 18.f;

			if (!is_on_screen || screen_pos.x < -leeway_x || screen_pos.x >(sdk::ScreenSize.x + leeway_x) || screen_pos.y < -leeway_y || screen_pos.y >(sdk::ScreenSize.y + leeway_y))
			{
				auto local_node = sdk::m_local_pawn->m_game_scene_node();
				if (!local_node)
					continue;
				view_origin = local_node->m_vec_abs_origin();
				delta = (target_pos - view_origin).normalized();

				radius = settings.cfg().visuals.oof_radius * (sdk::ScreenSize.y / 480.f);

				get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

				offscreen_rotation = -offscreen_rotation;

				verts[0] = { offscreen_pos.x, offscreen_pos.y };
				verts[1] = { offscreen_pos.x - (6.f + settings.cfg().visuals.oof_size), offscreen_pos.y + (12.f + settings.cfg().visuals.oof_size * 2) };
				verts[2] = { offscreen_pos.x + (6.f + settings.cfg().visuals.oof_size), offscreen_pos.y + (12.f + settings.cfg().visuals.oof_size * 2) };

				verts[0] = RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
				verts[1] = RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
				verts[2] = RotateVertex(offscreen_pos, verts[2], offscreen_rotation);
				static auto swap = false;
				static float alpha = 255.f;
				static auto last_framecount = 0;

				if (alpha == 255.f)
				{
					swap = false;
				}
				if (alpha == 0.f)
				{
					swap = true;
				}
				if (last_framecount != g_interface->m_globalVars->nFrameCount)
				{
					if (swap)
						alpha += 255.f / 0.73f * g_interface->m_globalVars->flFrameTime;
					else
						alpha -= 255.f / 0.73f * g_interface->m_globalVars->flFrameTime;
					alpha = ImClamp(alpha, 0.f, 255.f);
					last_framecount = g_interface->m_globalVars->nFrameCount;
				}
				Color_t color = settings.cfg().visuals.oof_color;
				color.a = (int)alpha;
				ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

				draw_list->AddTriangleFilled(ImVec2(verts[0].x, verts[0].y), ImVec2(verts[1].x, verts[1].y), ImVec2(verts[2].x, verts[2].y), color.GetU32());
			}
		}

		if (!player_info.m_box.found)
			continue;

		if (settings.cfg().visuals.b_box) {
			g_hook->last_func = "b_box";
			draw_list->AddRect({ player_info.m_box.left + 1, player_info.m_box.top + 1 }, { player_info.m_box.right + 1,player_info.m_box.bottom + 1 }, Color_t(0, 0, 0).GetU32());
			draw_list->AddRect({ player_info.m_box.left, player_info.m_box.top }, { player_info.m_box.right, player_info.m_box.bottom }, settings.cfg().visuals.box_color.GetU32());
			draw_list->AddRect({ player_info.m_box.left - 1, player_info.m_box.top - 1 }, { player_info.m_box.right - 1, player_info.m_box.bottom - 1 }, Color_t(0, 0, 0).GetU32());
		}

		if (settings.cfg().visuals.b_healthBar) {
			g_hook->last_func = "b_healthBar";
			float x = player_info.m_box.left - 6.f;
			float y = player_info.m_box.top;
			float height = fabsf(player_info.m_box.bottom - player_info.m_box.top);
			float bar_width = 2.f;
			float target_health = player_info.m_health;
			float& anim_health = animated_health[player_info.m_pawn];
			anim_health = anim_health + (target_health - anim_health) * 0.1f;
			float filled_height = height - ((height * anim_health) / player_info.m_pawn->m_max_health());

			draw_list->AddRectFilled({ x - 1.f, y - 1.f }, { x + bar_width + 1.f, y + height + 1.f }, Color_t(30, 30, 30).GetU32());

			if (settings.cfg().visuals.b_gradienthpbar) {
				const float step = 2.0f;
				for (float current_y = y + filled_height; current_y < y + height; current_y += step) {
					float ratio = (current_y - (y + filled_height)) / (height - filled_height);
					Color_t gradient_color = settings.cfg().visuals.heathbar_color.lerp(settings.cfg().visuals.c_gradienthpbar2, ratio);

					if (settings.cfg().visuals.b_hpbar_glow) {
						Color_t glow_color = gradient_color;
						for (int i = 1; i <= 3; i++) {
							float outer_dist = static_cast<float>(i);
							float alpha = 150.f * (1.f - outer_dist / 4.f);
							if (alpha > 0) {
								glow_color.a = static_cast<int>(alpha);
								draw_list->AddRect({ x - outer_dist, current_y - outer_dist }, { x + bar_width + outer_dist, (std::min)(current_y + step, y + height) + outer_dist }, glow_color.GetU32(), 0.f, ImDrawFlags_RoundCornersAll, 1.f);
							}
						}
					}

					draw_list->AddRectFilled(
						{ x, current_y },
						{ x + bar_width, (std::min)(current_y + step, y + height) },
						gradient_color.GetU32()
					);
				}
			}
			else {
				if (settings.cfg().visuals.b_hpbar_glow) {
					Color_t glow_color = settings.cfg().visuals.heathbar_color;
					for (int i = 1; i <= 3; i++) {
						float outer_dist = static_cast<float>(i);
						float alpha = 150.f * (1.f - outer_dist / 4.f);
						if (alpha > 0) {
							glow_color.a = static_cast<int>(alpha);
							draw_list->AddRect({ x - outer_dist, y + filled_height - outer_dist }, { x + bar_width + outer_dist, y + height + outer_dist }, glow_color.GetU32(), 0.f, ImDrawFlags_RoundCornersAll, 1.f);
						}
					}
				}
				draw_list->AddRectFilled({ x, y + filled_height }, { x + bar_width, y + height }, settings.cfg().visuals.heathbar_color.GetU32());
			}

			if (target_health < 93) {
				g_render->text({ x - 3.f, y + filled_height }, Color_t(255, 255, 255), center_outline, g_render->font_visitor, std::to_string((int)anim_health).c_str(), 8);
			}
		}

		if (settings.cfg().visuals.b_weaponbar && player_info.m_ammo > 0) {
			g_hook->last_func = "b_weaponbar";
			float bbox_width = player_info.m_box.right - player_info.m_box.left;
			float ammo_width = ((bbox_width * static_cast<float>(player_info.m_ammo)) / static_cast<float>(player_info.m_max_ammo));
			ammo_width = std::max<float>(std::min<float>(ammo_width, bbox_width), 0.f);
			float x = player_info.m_box.left;
			float y = player_info.m_box.bottom + 2.f;

			draw_list->AddRectFilled({ x - 1.f, y }, { x + bbox_width + 1.f, y + 4.f }, Color_t(30, 30, 30).GetU32());

			ImVec2 bb_min(x, y + 1.f);
			ImVec2 bb_max(x + ammo_width, y + 3.f);

			if (settings.cfg().visuals.b_gradientammobar) {
				const float step = 2.0f;
				for (float current_x = bb_min.x; current_x < bb_max.x; current_x += step) {
					float ratio = (current_x - bb_min.x) / (bb_max.x - bb_min.x);
					Color_t gradient_color = settings.cfg().visuals.c_weaponbar.lerp(settings.cfg().visuals.c_gradientammobar2, ratio);

					if (settings.cfg().visuals.b_ammobar_glow) {
						Color_t glow_color = gradient_color;
						for (int i = 1; i <= 3; i++) {
							float outer_dist = static_cast<float>(i);
							float alpha = 150.f * (1.f - outer_dist / 4.f);
							if (alpha > 0) {
								glow_color.a = static_cast<int>(alpha);
								draw_list->AddRect({ current_x - outer_dist, bb_min.y - outer_dist }, { (std::min)(current_x + step, bb_max.x) + outer_dist, bb_max.y + outer_dist }, glow_color.GetU32(), 0.f, ImDrawFlags_RoundCornersAll, 1.f);
							}
						}
					}

					draw_list->AddRectFilled({ current_x, bb_min.y }, { (std::min)(current_x + step, bb_max.x), bb_max.y },gradient_color.GetU32());
				}
			}
			else {
				if (settings.cfg().visuals.b_ammobar_glow) {
					Color_t glow_color = settings.cfg().visuals.c_weaponbar;
					for (int i = 1; i <= 3; i++) {
						float outer_dist = static_cast<float>(i);
						float alpha = 150.f * (1.f - outer_dist / 4.f);
						if (alpha > 0) {
							glow_color.a = static_cast<int>(alpha);
							draw_list->AddRect({ bb_min.x - outer_dist, bb_min.y - outer_dist }, { bb_max.x + outer_dist, bb_max.y + outer_dist }, glow_color.GetU32(), 0.f, ImDrawFlags_RoundCornersAll, 1.f);
						}
					}
				}
				draw_list->AddRectFilled(bb_min, bb_max, settings.cfg().visuals.c_weaponbar.GetU32());
			}
		}

		if (settings.cfg().visuals.b_nicknames) {
			g_hook->last_func = "b_nicknames";

			std::string name = player_info.m_name;
			if (name.size() > 15)
				name = name.substr(0, 15) + "...";

			c_player_controller* controller = player_info.m_controller;
			if (!controller)
				return;

			ImTextureID avatar = get_avatar(controller->m_steam_id());

			const float text_size = 12.5f;
			const float avatar_size = text_size + 2.0f;
			const float spacing = 2.0f;

			ImVec2 text_size_vec = g_fonts->tab->CalcTextSizeA(text_size, FLT_MAX, 0.0f, name.c_str());
			float text_width = text_size_vec.x;

			float total_width = text_width;
			bool draw_avatar = settings.cfg().visuals.b_playeravatar && avatar;

			if (draw_avatar)
				total_width += avatar_size + spacing;

			float center_x = player_info.m_box.left + (player_info.m_box.right - player_info.m_box.left) * 0.5f;
			float base_x = center_x - (total_width * 0.5f);
			float text_y = player_info.m_box.top - (avatar_size * 0.5f) - 7.0f;

			if (draw_avatar) {
				ImVec2 avatar_pos(base_x - 1.0f, text_y - 1.0f);
				ImGui::GetForegroundDrawList()->AddImage(avatar, avatar_pos, ImVec2(avatar_pos.x + avatar_size, avatar_pos.y + avatar_size));
				base_x += avatar_size + spacing;
			}

			vec2_t text_pos(base_x, text_y);

			draw_list->AddText(g_fonts->tab, text_size, ImVec2(text_pos.x + 1, text_pos.y + 1), ImColor(0, 0, 0), name.c_str());
			draw_list->AddText(g_fonts->tab, text_size, ImVec2(text_pos.x, text_pos.y), ImColor(255, 255, 255), name.c_str());
		}

		if (settings.cfg().visuals.b_weaponname) {
			g_hook->last_func = "b_weaponname";
			std::string weapon_name = player_info.m_weapon_name;

			size_t weapon_prefix_pos = weapon_name.find("weapon_");
			if (weapon_prefix_pos != std::string::npos) {
				weapon_name.erase(weapon_prefix_pos, 7);
			}

			std::transform(weapon_name.begin(), weapon_name.end(), weapon_name.begin(), ::toupper);

			if (weapon_name.size() > 15)
				weapon_name = weapon_name.substr(0, 15) + "...";

			float box_center_x = player_info.m_box.left + (player_info.m_box.right - player_info.m_box.left) * 0.5f;
			float text_y = player_info.m_box.bottom + 2.0f;

			if (player_info.m_ammo > 0) text_y += 2.0f;
			if (settings.cfg().visuals.b_weaponbar) text_y += 4.f;

			ImVec2 text_size = g_fonts->tab->CalcTextSizeA(10, FLT_MAX, 0.0f, weapon_name.c_str());
			float text_x = box_center_x - text_size.x * 0.5f;

			draw_list->AddText(g_fonts->tab, 10, ImVec2(text_x + 1, text_y + 1), ImColor(0, 0, 0), weapon_name.c_str());
			draw_list->AddText(g_fonts->tab, 10, ImVec2(text_x, text_y), ImColor(255, 255, 255), weapon_name.c_str());
		}

		if (settings.cfg().visuals.b_weaponicon) {
			g_hook->last_func = "b_weaponicon";

			float box_center_x = player_info.m_box.left + (player_info.m_box.right - player_info.m_box.left) * 0.5f;
			float icon_y = player_info.m_box.bottom + 2.0f;

			if (settings.cfg().visuals.b_weaponname) icon_y += 10.f;
			if (settings.cfg().visuals.b_weaponbar) icon_y += 4.f;

			const char* weapon_icon = GunIcon(player_info.m_weapon_name);
			ImVec2 icon_size = g_render->font_weapons->CalcTextSizeA(10, FLT_MAX, 0.0f, weapon_icon);
			float icon_x = box_center_x - icon_size.x * 0.5f;

			draw_list->AddText(g_render->font_weapons, 10, ImVec2(icon_x + 1, icon_y + 1), ImColor(0, 0, 0), weapon_icon);

			draw_list->AddText(g_render->font_weapons, 10, ImVec2(icon_x, icon_y), ImColor(255, 255, 255), weapon_icon);
		}

		if (settings.cfg().visuals.b_skeleton) {
			if (!player_info.m_pawn)
				continue;
			g_hook->last_func = "b_skeleton";
			auto game_scene_node = player_info.m_pawn->m_game_scene_node();
			if (!game_scene_node)
				continue;

			auto* skeleton = game_scene_node->get_skeleton_instance();
            auto* bones = skeleton ? skeleton->m_model_state().bones : nullptr;
            const int count = skeleton ? skeleton->get_bone_count() : 0;
            if (!bones || count <= 0) continue;

            const float main_thickness = 2.0f;
			const float outline_thickness = 1.0f;
			const ImColor outline_color(0, 0, 0, 220);

			for (const auto& link : skeleton_links) {
                if (link[0] >= count || link[1] >= count) continue;
				vec3_t bonPos = bones[link[0]].position;
				vec3_t bonPos2 = bones[link[1]].position;

				vec2_t WTSbonPos, WTSbonPos2;
				if (g_render->world_to_screen(bonPos, WTSbonPos) && g_render->world_to_screen(bonPos2, WTSbonPos2)) {
					draw_list->AddLine(WTSbonPos.im_vec2(), WTSbonPos2.im_vec2(), outline_color, main_thickness + outline_thickness);

					draw_list->AddLine(WTSbonPos.im_vec2(), WTSbonPos2.im_vec2(), settings.cfg().visuals.skeleton_color.GetU32(), main_thickness);
				}
			}
		}

		if (settings.cfg().visuals.b_ping_flag)
		{
			g_hook->last_func = "b_ping_flag";
			if (!sdk::m_local_controller)
				continue;
			float x = player_info.m_box.right, y = player_info.m_box.top;
			int local_ping = sdk::m_local_controller->m_ping();
			int ent_ping = player_info.m_controller->m_ping();

			if (local_ping > ent_ping)
				g_render->text({ x + 10,y }, Color_t(177, 0, 0), center_outline, g_fonts->tab, std::to_string(ent_ping), 7);
			else g_render->text({ x + 10,y }, Color_t(0, 177, 0), center_outline, g_fonts->tab, std::to_string(ent_ping), 7);
		}

		int padding = 0;

		for (auto& flag : player_info.m_flags) {
			g_hook->last_func = "m_flags";
			float x = player_info.m_box.right, y = player_info.m_box.top;

			Color_t text_color(255, 255, 255, 255);

			if (flag == "C4") {
				text_color = Color_t(255, 0, 0, 255);
			}
			else if (flag == "ZEUS") {
				text_color = Color_t(255, 255, 0, 255);
			}
			else if (flag == "DEFUSER") {
				text_color = Color_t(255, 100, 100, 255);
			}
			else if (flag == "IMMUNE") {
				text_color = Color_t(130, 188, 255, 255);
			}

			draw_list->AddText(g_fonts->tab, 11, ImVec2(x + 6, y + padding + 1), ImColor(0, 0, 0), flag.c_str());
			draw_list->AddText(g_fonts->tab, 11, ImVec2(x + 5, y + padding), text_color.GetU32(), flag.c_str());

			padding += g_fonts->tab->CalcTextSizeA(7, FLT_MAX, FLT_MAX, flag.c_str()).y + 2;
		}

		if (settings.cfg().visuals.b_sound_esp)
		{
			if (!player_info.m_pawn)
				continue;

			auto scene_node = player_info.m_pawn->m_game_scene_node();
			if (!scene_node)
				continue;

			float sound = player_info.m_pawn->m_emit_sound_time();

			static float static_sound = sound;

			if (static_sound != sound) {
				sound_info.push_back(sound_info_t{ scene_node->m_vec_abs_origin(), true });
				static_sound = sound; // updt sound
			}

			for (int i = 0; i < sound_info.size(); i++)
			{
				g_render->circle_filled_3d(sound_info[i].player_origin, 20.f, ImColor(255, 255, 255), 1.f);
				if (i >= 3)
				{
					sound_info.erase(sound_info.begin(), sound_info.begin() + 1);
				}
			}
		}
	}
}
