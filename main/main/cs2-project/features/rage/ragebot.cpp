#include "ragebot.h"
#include "../../sdk/schema/class/weapon.h"
#include "../../sdk/schema/class/player.h"
#include "autowall.h"
#include "../movement/movement.h"
#include "../../sdk/entity_listener/entity_list.h"
#include "../visual/additionally.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../render/render.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../sdk/game_class/interfaces/i_global_vars.h"
#include "../../sdk/game_class/cconvar.h"
#include "../../sdk/game_class/game_enums.h"
#include <future>
#include <mutex>
#include "../../sdk/input_system/inputsystem.h"
#include "../../core/hooks/hooks.h"
#include "../../sdk/game_class/interfaces/csgoinput.h"

vec3_t ragebot::calculate_spread_internal(int seed, float accuracy, float spread, float recoil_index, int item_def_idx)
{
    if (!math::fnRandomSeed || !math::fnRandomFloat) return {};
    static std::mutex sampler_mutex;
    const std::lock_guard lock(sampler_mutex);
    math::fnRandomSeed(seed + 1);
    float r1 = math::fnRandomFloat(0.f, 1.f), a1 = math::fnRandomFloat(0.f, 6.28318530718f);
    float r2 = math::fnRandomFloat(0.f, 1.f), a2 = math::fnRandomFloat(0.f, 6.28318530718f);
    if (item_def_idx == WEAPON_NEGEV && recoil_index < 3.f) {
        for (int i = 3; i > recoil_index; --i) { r1 *= r1; r2 *= r2; }
        r1 = 1.f - r1; r2 = 1.f - r2;
    }
    return {std::cos(a1) * r1 * accuracy + std::cos(a2) * r2 * spread,
            std::sin(a1) * r1 * accuracy + std::sin(a2) * r2 * spread, 0.f};
}

bool ragebot::can_shoot(c_base_player_weapon* active_weapon)
{
	return (active_weapon->m_iclip1() > 0) && (active_weapon->m_next_primary_attack_tick() <= sdk::m_local_controller->m_tick_base() + 2);
}


int ragebot::no_spread(vec3_t& pos, c_weapon_base* weapon, c_player_pawn* Target)
{
	if (!weapon)
		return false;

	auto weapon_data = weapon->m_weapon_data();

	if (!weapon_data)
		return false;

	auto controller = sdk::m_local_controller;
	if (!controller)
		return 0;

	auto pawn = sdk::m_local_pawn;
	if (!pawn)
		return 0;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return 0;

	vec3_t dir{};
	vec3_t end{};
	vec3_t fwd{};
	vec3_t right{};
	vec3_t up{};

	int hits{};
	int aw_hits{};
	float range = weapon_data->m_range();

	weapon->update_accuracy_penality();
	const float spread = local_data->m_spread;
	const float inaccuracy = local_data->m_inaccuracy;

	math::angle_vectors3(pos, &fwd, &right, &up);
	//if (g_RageBot->weapon_is_at_max_accuracy(weapon_data, inaccuracy))
	//	return true;
	float damage = 0;
	TraceFilter_t filter(0x1C300B, sdk::m_local_pawn, nullptr, 3);
	vec3_t eye = local_data->m_eye_pos;
	//for (int seed = 0; seed < 570; seed++) 
	{

		auto spread_calc = ragebot::calculate_spread_internal((int)g_hook->fnget_hash_seed(sdk::m_local_pawn, { pos.x,pos.y,pos.z }, +1), inaccuracy, spread, weapon->m_recoil_index(), weapon->m_attribute_manager()->m_item()->m_item_definition_index());

		// get spread direction.
		dir = vec3_t(fwd - (right * spread_calc.x) + (up * spread_calc.y));
		end = eye + dir * range;

		Ray_t ray{};
		game_trace_t trace{};
		//g_interface->m_game_trace->init_trace(filter, pawn, MASK_SHOT, 3, 7);
		//g_interface->m_game_trace->trace_shape(&ray, eye, end, &filter, &trace);
		g_interface->m_game_trace->clip_ray_entity(&ray, eye, end, Target, &filter, &trace);
		//clip_trace_to_players(eye, end, &filter, &trace, 0.F, 60.F, (1.F / (eye - end).length()) * (trace.m_end_pos - eye).length()); //
		if (trace.HitEntity /*&& trace.HitEntity->is_player_pawn()*/ /*&& trace.HitEntity->get_ref_ehandle().get_entry_index() == pawn->get_ref_ehandle().get_entry_index()*/)
			return 100;/*hits++;*/
	}
	return static_cast<int>((static_cast<float>(hits / 570.f)) * 100.f);
}

int ragebot::hitchance(vec3_t& pos, c_weapon_base* weapon, c_player_pawn* Target)
{
	if (!weapon)
		return false;

	auto weapon_data = weapon->m_weapon_data();

	if (!weapon_data)
		return false;

	auto controller = sdk::m_local_controller;
	if (!controller)
		return 0;

	auto pawn = sdk::m_local_pawn;
	if (!pawn)
		return 0;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return 0;

	vec3_t dir{};
	vec3_t end{};
	vec3_t fwd{};
	vec3_t right{};
	vec3_t up{};

	int hits{};
	int aw_hits{};
	float range = weapon_data->m_range();

	const float spread = local_data->m_spread;
	const float inaccuracy = local_data->m_inaccuracy;

	math::angle_vectors3(pos, &fwd, &right, &up);
	//if (g_RageBot->weapon_is_at_max_accuracy(weapon_data, inaccuracy))
	//	return true;
	float damage = 0;
	TraceFilter_t filter(0x1C300B, sdk::m_local_pawn, nullptr, 3);
	vec3_t eye = local_data->m_eye_pos;
	for (int seed = 0; seed < 570; seed++) {

		auto spread_calc = ragebot::calculate_spread_internal(seed, inaccuracy, spread, weapon->m_recoil_index(), weapon->m_attribute_manager()->m_item()->m_item_definition_index());

		// get spread direction.
		dir = vec3_t(fwd - (right * spread_calc.x) + (up * spread_calc.y));
		end = eye + dir * range;

		Ray_t ray{};
		game_trace_t trace{};
		//	g_interface->m_game_trace->init_trace(filter, pawn, MASK_SHOT, 3, 7);
			//g_interface->m_game_trace->trace_shape(&ray, eye, end, &filter, &trace);
		g_interface->m_game_trace->clip_ray_entity(&ray, eye, end, Target, &filter, &trace);
		//clip_trace_to_players(eye, end, &filter, &trace, 0.F, 60.F, (1.F / (eye - end).length()) * (trace.m_end_pos - eye).length()); //
		if (trace.HitEntity /*&& trace.HitEntity->is_player_pawn()*/ /*&& trace.HitEntity->get_ref_ehandle().get_entry_index() == pawn->get_ref_ehandle().get_entry_index()*/)
			hits++;
	}
	return static_cast<int>((static_cast<float>(hits / 570.f)) * 100.f);
}


void ragebot::autostop(int mode) {
	c_weapon_base* weapon = sdk::m_local_pawn->ActiveWeapon();

	if (!can_shoot(weapon))
		return;

	if (!(sdk::m_local_pawn->m_fflags() & FL_ONGROUND))
		return;

	if (mode == 1)
	{
		if (sdk::m_local_pawn->m_vec_velocity().length2d() == 0)
			return;

		//sdk::m_cmd->pb.mutable_base()->set_forwardmove(1.f);
		//sdk::m_cmd->pb.mutable_base()->set_leftmove(0.f);
		//float moveforw = sdk::m_cmd->pb.mutable_base()->forwardmove();

		//vec3_t* move = (vec3_t*)&moveforw;
		//vec3_t move_backup = *move;

		//vec3_t abs_origin = sdk::m_local_pawn->m_game_scene_node()->m_vec_abs_origin();
		//qangle_t angle = math::CalcAngel(abs_origin, math::ExtrapolatePosition(abs_origin, sdk::m_local_pawn->m_vec_abs_velocity() * -5.f, 1.f));

		//const float delta = remainderf(angle.y - sdk::angleStor.y, 360.f);
		//const float yaw = DirectX::XMConvertToRadians(delta);

		//move->x = move_backup.x * std::cos(yaw) - move_backup.y * std::sin(yaw);
		//move->y = move_backup.x * std::sin(yaw) + move_backup.y * std::cos(yaw);

		//float mul = 1.f;
		//if (std::fabsf(move->x) > 1.f)
		//	mul = 1.f / std::fabsf(move->x);
		//else if (std::fabsf(move->y) > 1.f)
		//	mul = 1.f / std::fabsf(move->y);

		//move->x *= mul;
		//move->y *= mul;
		//move->z = 0.f;

		//sdk::m_cmd->pb.mutable_base()->set_forwardmove(move->x * mul);
		//sdk::m_cmd->pb.mutable_base()->set_leftmove(move->y * mul);
		g_movement->auto_stop(sdk::m_cmd, sdk::m_local_pawn, weapon, 0);
	}
	else {
		//MOVE::LimitSpeed(sdk::m_local_pawn, sdk::m_cmd, 70);
		//weapon->update_accuracy();// dont do this.
		auto pred_data = g_prediction->get_local_data();
		float inaccuracy = pred_data->m_inaccuracy/*->weapon->get_inaccuracy()*/;
		float spread = pred_data->m_spread;//weapon->get_spread();// g_PredictionSystem->GetSpread();// weapon->get_spread();

		float max_allowed_speed = [&]() {
			const auto weapon_type = weapon->m_weapon_data()->m_weapon_type();

			if (inaccuracy > 0.05f) {
				return 7.5f;
			}

			switch (weapon_type) {
			case WEAPONTYPE_SNIPER_RIFLE:
				return 10.f;
			case WEAPONTYPE_PISTOL:
				return 10.f;
			case WEAPONTYPE_SUBMACHINEGUN:
				return 10.f;
			default:
				return 10.f;
			}
			}();

		vec3_t velocity = sdk::m_local_pawn->m_vec_abs_velocity();
		float current_speed = velocity.length2d();

		if (current_speed > max_allowed_speed) {
			float speed_ratio = max_allowed_speed / (current_speed + 1.f);
			float as = sdk::m_cmd->pb.base().forwardmove();
			as *= speed_ratio;
			sdk::m_cmd->pb.mutable_base()->set_forwardmove(as);

			float as2 = sdk::m_cmd->pb.base().leftmove();
			as2 *= speed_ratio;
			sdk::m_cmd->pb.mutable_base()->set_leftmove(as2);
		}
	}
}

void ragebot::process_backtrack(lag_record_t* record) {
	if (!record)
		return;

	const int tick_shoot = TIME_TO_TICKS(record->m_simulation_time) + 2;

	for (int i = 0; i < sdk::m_cmd->pb.input_history_size(); i++) {

		if (sdk::m_cmd->pb.attack1_start_history_index() == -1)
			continue;

		auto input_history = sdk::m_cmd->pb.mutable_input_history(i);
		if (!input_history)
			continue;

		//if (!input_history->has_cl_interp())
		//	input_history->set_allocated_cl_interp(input_history->create_cl_interp_message());

		if (input_history->has_cl_interp()) {
			auto interp_info = input_history->mutable_cl_interp();
			interp_info->set_frac(0.f);
		}

		if (input_history->has_sv_interp0()) {
			auto interp_info = input_history->mutable_sv_interp0();
			interp_info->set_src_tick(tick_shoot);
			interp_info->set_dst_tick(tick_shoot);
			interp_info->set_frac(0.f);
		}

		if (input_history->has_sv_interp1()) {
			auto interp_info = input_history->mutable_sv_interp1();
			interp_info->set_src_tick(tick_shoot);
			interp_info->set_dst_tick(tick_shoot);
			interp_info->set_frac(0.f);
		}

		input_history->set_render_tick_count(tick_shoot);

		sdk::m_cmd->pb.mutable_base()->set_client_tick(tick_shoot);

		input_history->set_player_tick_count(sdk::m_local_controller->m_tick_base());
		//input_history->set_player_tick_fraction(g_prediction->get_local_data()->m_player_tick_fraction);
	}
}

void ragebot::process_attacka(c_user_cmd* user_cmd) {

	for (int i = 0; i < user_cmd->pb.input_history_size(); i++) {
		auto container = user_cmd->pb.mutable_input_history(i);
		if (container) {
			container->set_player_tick_count(g_prediction->get_local_data()->m_shoot_tick);
		}
	}
	//std::string message =
	//	CHAT_COLOR_WHITE "[" CHAT_COLOR_RED "Fearless" CHAT_COLOR_WHITE "] "
	//	CHAT_COLOR_RED  "damage: "  CHAT_COLOR_LIGHT_GREEN + std::to_string(best_damage) + " "
	//	CHAT_COLOR_BLUE " hitboxe: " + hitboxes[get_hitbox_from_menu(m_best_target->m_best_point->m_hitbox)];

	//g_memory->sent_message_to_local_chat(message.c_str());

	
	user_cmd->pb.set_attack1_start_history_index(0);
	if (!settings.cfg().legitbot.b_trigger)
		process_backtrack(m_best_target->m_lag_record);
	user_cmd->m_button_state.nValue |= IN_ATTACK;
}

vec3_t ragebot::calculate_spread_angles(vec3_t angle, int random_seed, float weapon_inaccuarcy, float weapon_spread)
{
	float r1, r2, r3, r4, s1, c1, s2, c2;

	math::fnRandomSeed(random_seed + 1);

	r1 = math::fnRandomFloat(0.f, 1.f);
	r2 = math::fnRandomFloat(0.f, math::_pi2);
	r3 = math::fnRandomFloat(0.f, 1.f);
	r4 = math::fnRandomFloat(0.f, math::_pi2);

	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	vec3_t spread = {
		(c1 * (r1 * weapon_inaccuarcy)) + (c2 * (r3 * weapon_spread)),
		(s1 * (r1 * weapon_inaccuarcy)) + (s2 * (r3 * weapon_spread))
	};

	vec3_t vec_forward, vec_right, vec_up;
	math::angle_vectors2(angle, vec_forward, vec_right, vec_up);

	return (vec_forward + (vec_right * spread.x) + (vec_up * spread.y)).normalized();
}

void ragebot::store_records(int stage) {
	if (!g_interface->m_engine->is_in_game()) {
		m_lag_records.clear();
		return;
	}

	if (stage != FRAME_NET_UPDATE_END || !sdk::m_local_pawn)
		return;

	auto* maxunlag = g_interface->cvar->Find(fnv1a::hash("sv_maxunlag"));
    const auto sv_maxunlag = maxunlag ? maxunlag->value.fl : 0.2f;
	const int max_ticks = std::clamp(TIME_TO_TICKS(sv_maxunlag), 1, 64);

	const auto& entity_list = g_entlistener->get("CCSPlayerController");
	for (int i = 0; i < entity_list.size(); ++i) {

		auto entity = entity_list[i];

		auto player_controller = reinterpret_cast<c_player_controller*>(entity);
		if (!player_controller || player_controller == sdk::m_local_controller)
			continue;

		auto handle = player_controller->get_ref_ehandle().to_int();

		if (!player_controller->m_pawn_is_alive()) {
			auto player_iterator = m_lag_records.find(handle);
			if (player_iterator != m_lag_records.end())
				m_lag_records.erase(player_iterator);

			continue;
		}

		auto player_pawn = reinterpret_cast<c_player_pawn*>(g_interface->m_game_resource_service->get_game_entity_system()->get(player_controller->m_pawn().get_entry_index()));
		if (!player_pawn) {
			auto player_iterator = m_lag_records.find(handle);
			if (player_iterator != m_lag_records.end())
				m_lag_records.erase(player_iterator);

			continue;
		}

		if (player_pawn->m_team_num() == sdk::m_local_pawn->m_team_num())
			continue;

		if (player_pawn == sdk::m_local_pawn)
			continue;

		if (m_lag_records.find(handle) == m_lag_records.end()) {
			m_lag_records.insert_or_assign(handle, std::deque<lag_record_t>{});

			continue;
		}

		auto& records = m_lag_records[handle];

        lag_record_t candidate(player_pawn);
        if (!candidate.m_pawn) continue;
        if (!records.empty() && records.front().m_simulation_time == candidate.m_simulation_time) continue;
        records.emplace_front(candidate);
        while (records.size() > static_cast<size_t>(max_ticks)) records.pop_back();
	}
}

lag_record_t* ragebot::select_record(int handle) {
	if (!sdk::m_local_pawn->m_is_alive())
		return nullptr;

	auto& records = m_lag_records[handle];

	if (records.empty())
		return nullptr;

	lag_record_t* best_record = nullptr;
	lag_record_t* last_record = &records.front();

	if (records.size() == 1u)
		return last_record->m_pawn && last_record->is_valid() ? last_record : nullptr;

	for (auto i = records.begin(); i != records.end(); i = std::next(i)) {
		auto record = &*i;

		if (!record->m_pawn || !record->is_valid())
			continue;

		if (!record || !record->is_valid())
			continue;

		if (!best_record) {
			best_record = record;

			continue;
		}

		if (record->m_throwing) {
			best_record = record;

			continue;
		}

		if ((record->m_pawn->m_fflags() & FL_ONGROUND) != (record->m_pawn->m_fflags() & FL_ONGROUND)) {
			if ((record->m_pawn->m_fflags() & FL_ONGROUND))
				best_record = record;
		}
	}

	return best_record;
}

void ragebot::find_targets() {
    m_aim_targets.clear();
	if (!sdk::m_local_pawn->m_is_alive())
		return;

	auto entities = g_entlistener->get("CCSPlayerController");

	for (auto entity : entities) {
		if (!entity)
			continue;

		auto player_controller = reinterpret_cast<c_player_controller*>(entity);
		if (!player_controller || player_controller == sdk::m_local_controller)
			continue;

		int handle = player_controller->get_ref_ehandle().to_int();
		lag_record_t* best_record = this->select_record(handle);

		if (!best_record)
			continue;

		m_aim_targets[handle] = std::make_shared<aim_target_t>(best_record);
	}
}

std::shared_ptr<aim_target_t> ragebot::get_nearest_target() {
	if (!sdk::m_local_pawn->m_is_alive())
		return nullptr;

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return nullptr;

	vec3_t shoot_pos = local_data->m_eye_pos;

	vec2_t screen_center(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);

	std::shared_ptr<aim_target_t> best_target = nullptr;
	float best_screen_distance = FLT_MAX;
	float best_distance = FLT_MAX;

	for (auto it = m_aim_targets.begin(); it != m_aim_targets.end(); it = std::next(it)) {
		auto target = &it->second;

		if (!target->get()->m_lag_record
			|| (uintptr_t)target->get()->m_lag_record->m_pawn == 0xdddddddddddddddd
			|| !target->get()->m_lag_record->m_pawn
			|| !target->get()->m_pawn->m_is_alive()
			|| !target->get()->m_lag_record->m_pawn->m_is_alive()
			|| !target->get()->m_lag_record->m_pawn->m_game_scene_node()
			) {
			continue;
		}
		vec3_t target_pos{};

		try
		{
			target_pos = target->get()->m_lag_record->m_pawn->m_game_scene_node()->m_vec_abs_origin();
		}
		catch (...)
		{
			std::cout << "error target_pos" << std::endl;
		}
		float distance{};

		vec2_t screen_pos;
		if (!g_render->world_to_screen(target_pos, screen_pos))
			distance = target_pos.dist_to(shoot_pos);


		float screen_distance = screen_pos.dist_to(screen_center);

		if (screen_distance < best_screen_distance || distance < best_distance) {
			best_screen_distance = screen_distance;
			best_distance = distance;
			if (target && *target) {
				best_target = *target;
			}
		}
	}

	return best_target;
}

vec3_t ragebot::transform_point(matrix3x4_t matrix, vec3_t point)
{
	vec3_t result{ };
	math::vector_transform(point, matrix, result);
	return result;
}

int /*CounterStrikeGlobalOffensive::Client::*/HitboxToHitgroup(int hitbox)
{
	if (hitbox == hitbox_head)
		return e_hitgroups::hitgroup_head;
	else if (hitbox == hitbox_stomach || hitbox == hitbox_pelvis)
		return e_hitgroups::hitgroup_stomach;
	else if (hitbox >= hitbox_chest && hitbox <= hitbox_upper_chest)
		return e_hitgroups::hitgroup_chest;
	else if (hitbox == hitbox_left_upper_arm || hitbox == hitbox_left_forearm || hitbox == hitbox_left_hand)
		return e_hitgroups::hitgroup_leftarm;
	else if (hitbox == hitbox_right_forearm || hitbox == hitbox_right_upper_arm || hitbox == hitbox_right_hand)
		return e_hitgroups::hitgroup_rightarm;
	else if (hitbox == hitbox_neck)
		return e_hitgroups::hitgroup_neck;
	else if (hitbox == hitbox_left_foot || hitbox == hitbox_left_calf || hitbox == hitbox_left_thigh)
		return e_hitgroups::hitgroup_leftleg;
	else if (hitbox == hitbox_right_foot || hitbox == hitbox_right_calf || hitbox == hitbox_right_thigh)
		return e_hitgroups::hitgroup_rightleg;
	else
		return e_hitgroups::hitgroup_generic;
}
	
enum BONEINDEX : DWORD
{
	Pelvis = 0,
	Spine_1 = 2,
	Spine_2 = 3,
	Spine_3 = 4,
	Neck_0 = 5,
	Head = 6,
	Arm_Upper_L = 8,
	Arm_Lower_L = 9,
	Hand_L = 10,
	Arm_Upper_R = 13,
	Arm_Lower_R = 14,
	Hand_R = 15,
	Leg_Upper_L = 22,
	Leg_Lower_L = 23,
	Ankle_L = 24,
	Leg_Upper_R = 25,
	Leg_Lower_R = 26,
	Ankle_R = 27,
};

DWORD ragebot::hitboxToBone(c_hitbox* hitbox) {
	size_t index = hitbox->m_hitgroup;
	if (index == 0)
		return BONEINDEX::Head;
	if (index == 1)
		return BONEINDEX::Neck_0;
	if (index == 2)
		return BONEINDEX::Pelvis;
	if (index == 3)
		return BONEINDEX::Neck_0;
	if (index == 4)
		return BONEINDEX::Spine_1;
	if (index == 5)
		return BONEINDEX::Spine_2;
	if (index == 6)
		return BONEINDEX::Spine_2; // fuckkkkkkkk
	if (index == 7)
		return BONEINDEX::Leg_Upper_L;
	if (index == 8)
		return BONEINDEX::Leg_Upper_R;
	if (index == 9)
		return BONEINDEX::Leg_Lower_L;
	if (index == 10)
		return BONEINDEX::Leg_Lower_R;
	if (index == 11)
		return BONEINDEX::Ankle_L;
	if (index == 12)
		return BONEINDEX::Ankle_R;
	if (index == 13)
		return BONEINDEX::Hand_L;
	if (index == 14)
		return BONEINDEX::Hand_R;
	if (index == 15)
		return BONEINDEX::Arm_Upper_L;
	if (index == 16)
		return BONEINDEX::Arm_Lower_L;
	if (index == 17)
		return BONEINDEX::Arm_Upper_R;
	if (index == 18)
		return BONEINDEX::Arm_Lower_R;
	return BONEINDEX::Pelvis;
}

int ragebot::get_hitbox_from_menu(int hitbox) {
	switch (hitbox) {
	case hitbox_head:
		return 0;
	case hitbox_chest:
	case hitbox_lower_chest:
	case hitbox_upper_chest:
		return 1;
	case hitbox_pelvis:
	case hitbox_stomach:
		return 2;
	case hitbox_right_hand:
	case hitbox_left_hand:
	case hitbox_right_upper_arm:
	case hitbox_right_forearm:
	case hitbox_left_upper_arm:
	case hitbox_left_forearm:
		return 3;
	case hitbox_right_thigh:
	case hitbox_left_thigh:
	case hitbox_right_calf:
	case hitbox_left_calf:
		return 4;
	case hitbox_right_foot:
	case hitbox_left_foot:
		return 5;
	}

	return -1;
}

void ragebot::double_tap()
{
    if (!settings.cfg().ragebot.b_ragebot || !settings.cfg().ragebot.m_doubletap || !sdk::m_cmd || !sdk::m_local_pawn || !sdk::m_local_pawn->ActiveWeapon()) return;
	static int shot_count{};
	static int old_shot_count{ };
	double offset_tick{ };
	auto temp = sdk::m_local_pawn->ActiveWeapon()->m_next_primary_attack_tick_ratio() + modf(sdk::m_local_pawn->ActiveWeapon()->m_iWatTickOffset(), &offset_tick);
	auto next_attack = sdk::m_local_pawn->ActiveWeapon()->m_next_primary_attack_tick();
	auto shoot_tick = (int)next_attack + offset_tick;
	if (temp >= 0.0) {
		if (temp >= 1.0) {
			++shoot_tick;
		}
	}
	else {
		--shoot_tick;
	}
	bool f = shot_count % 2;


	if (sdk::m_cmd->pb.attack1_start_history_index() > -1)
		shot_count++;
	for (int i = 0; i < sdk::m_cmd->pb.input_history_size(); i++) {
		auto input_history = sdk::m_cmd->pb.mutable_input_history(i);
		if (!input_history) {
			continue;
		}
		input_history->set_player_tick_count(f ? shoot_tick - 1 : 0);
		input_history->set_player_tick_fraction(0.f);
	}
	sdk::m_cmd->pb.set_attack1_start_history_index(-1);
	old_shot_count = shot_count;
}

std::vector<vec3_t> ragebot::calculate_sphere_points(float radius, int num_points)
{
	std::vector<vec3_t> points;
	points.reserve(num_points);

	const float phi = math::_PI * (3.0f - std::sqrt(5.0f));

	for (int i = 0; i < num_points; ++i)
	{
		float y = 1 - (i / float(num_points - 1)) * 2;
		float radius_at_y = std::sqrt(1 - y * y);

		float theta = phi * i;

		float x = std::cos(theta) * radius_at_y;
		float z = std::sin(theta) * radius_at_y;

		vec3_t vec_point{};
		points.push_back({ x * radius, y * radius, z * radius });
	}

	return points;
}

std::vector<vec3_t> ragebot::calculate_points(int num_points, float radius, float height, matrix3x4_t matrix, hitbox_data_t hitbox)
{
	std::vector<vec3_t> points;
	points.reserve(num_points);

	for (int i = 0; i < num_points; ++i)
	{
		float theta = 2.f * math::_PI * i / num_points;
		float y = radius * std::cos(theta);
		float z = radius * std::sin(theta);

		vec3_t in_point = { hitbox.m_mins.x + hitbox.m_radius * height, y, z };

		vec3_t vec_point{};
		math::vector_transform(in_point, matrix, vec_point);
		points.push_back(vec_point);
	}

	return points;
}

float ragebot::calc_point_scale(vec3_t& point, const float& hitbox_radius) {
	auto local_data = g_prediction->get_local_data();

	if (!local_data)
		return 0.f;

	float spread = local_data->m_spread + local_data->m_inaccuracy;
	float distance = point.dist_to(local_data->m_eye_pos);

	float new_distance = distance / std::sin(math::deg2rad(90.f - math::rad2deg(spread)));
	float scale = ((hitbox_radius - new_distance * spread) + 0.1f) * 100.f;

	return std::clamp(scale, 0.f, 95.f);
}

bool ragebot::multi_points(lag_record_t* record, int hitbox, std::vector<aim_point_t>& points) {
	if (!record)
		return false;

	c_player_pawn* pawn = record->m_pawn;

	if (!pawn || !pawn->m_is_alive() || !pawn->is_player_pawn())
		return false;

	hitbox_data_t hitbox_data = get_hitbox_data(pawn, hitbox);

	if (hitbox_data.m_invalid_data)
		return false;

	auto local_data = g_prediction->get_local_data();

	if (!local_data)
		return false;

	vec3_t center = (hitbox_data.m_mins + hitbox_data.m_maxs) * 0.5f;

	matrix3x4_t matrix = math::transform_to_matrix(record->m_bone_data[hitbox_data.m_num_bone]);

	vec3_t point = transform_point(matrix, center);

	int hitbox_from_menu = get_hitbox_from_menu(hitbox);

	if (hitbox_from_menu == -1)
		return false;

	points.emplace_back(point, hitbox, true);

	if (!settings.cfg().ragebot.m_multi_points[hitbox_from_menu])
		return true;

	float scale = settings.cfg().ragebot.m_multipoint_scale;
	float radius = hitbox_data.m_radius * (scale / 100.f);

	if (radius <= 0.f)
		return true;

	if (hitbox == hitbox_upper_chest)
		return true;

	if (hitbox > hitbox_upper_chest) {
		auto limb_points = calculate_sphere_points(radius, 3);

		if (limb_points.empty())
			return false;

		for (const auto& point : limb_points) {
			auto point_pos = transform_point(matrix, { hitbox_data.m_maxs.x + point.x, hitbox_data.m_maxs.y + point.y, hitbox_data.m_maxs.z + point.z });
			points.emplace_back(point_pos, hitbox);
		}

		return true;
	}

	if (hitbox == hitbox_head || hitbox == hitbox_neck) {
		auto sphere_points = calculate_sphere_points(radius, 15);

		if (sphere_points.empty())
			return false;

		for (const auto& point : sphere_points) {
			auto point_pos = transform_point(matrix, { hitbox_data.m_maxs.x + point.x, hitbox_data.m_maxs.y + point.y, hitbox_data.m_maxs.z + point.z });
			points.emplace_back(point_pos, hitbox);
		}

		return true;
	}

	auto capsule_points = calculate_points(4, radius, 0.5f, matrix, hitbox_data);

	if (capsule_points.empty())
		return false;

	for (const auto& point : capsule_points)
		points.emplace_back(point, hitbox);

	return true;
}

struct aim_result_t {
	aim_point_t point;
	float damage;
};

aim_point_t ragebot::select_points(lag_record_t* record, float& damage) {
	std::vector<std::future<std::optional<aim_result_t>>> futures;
	std::atomic<bool> lethal_found{ false };
	std::atomic<float> best_damage{ 0.0f };
	std::mutex lethal_mutex;
	aim_result_t best_result{ {vec3_t(0.f, 0.f, 0.f), -1}, 0.f };

	if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive())
		return best_result.point;
	if (!record->m_pawn || !record->m_pawn->m_is_alive())
		return best_result.point;

	c_weapon_base* weapon = sdk::m_local_pawn->ActiveWeapon();
	c_cs_weapon_base_vdata* weapon_data = weapon->m_weapon_data();

	if (!weapon_data)
		return best_result.point;

	auto local_ent_base = g_interface->m_game_resource_service->get_game_entity_system()->get(sdk::m_local_pawn->get_ref_ehandle());
	c_player_pawn* pawn_ent = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(record->m_pawn->get_ref_ehandle());

	auto local_data = g_prediction->get_local_data();
	if (!local_data)
		return best_result.point;

	vec3_t shoot_pos = local_data->m_eye_pos;

	for (auto& hitbox : m_hitboxes) {
		futures.push_back(std::async(std::launch::deferred, [&, hitbox]() -> std::optional<aim_result_t> {
			if (lethal_found.load(std::memory_order_relaxed))
				return std::nullopt;
			std::vector<aim_point_t> points;
			//if (!multipoints(record->m_pawn, record->m_bone_data/*->m_game_scene_node()->get_skeleton_instance()->m_model_state().bones*/, hitbox, points, 0)) // 0 prediction == false
			//	return std::nullopt;
			if (!multi_points(record, hitbox, points))
				return std::nullopt;

			aim_result_t local_best{ {vec3_t(0.f, 0.f, 0.f), -1}, 0.f };
			for (const auto& point : points) {
				if (lethal_found.load(std::memory_order_relaxed))
					return std::nullopt;

				float point_damage = g_autowall->penetration_damage(shoot_pos, point.m_point, local_ent_base, sdk::m_local_pawn, record->m_pawn, pawn_ent, record->m_pawn->m_item_services()->m_has_heavy_armor(), record->m_pawn->m_team_num(), weapon_data);
				if (point_damage == 0)
					continue;

				if (point_damage > best_damage.load(std::memory_order_relaxed)) {
					local_best = { point, point_damage };
					best_damage.store(point_damage, std::memory_order_relaxed);
				}

				if (point_damage >= record->m_pawn->m_health()) {
					std::lock_guard<std::mutex> lock(lethal_mutex);
					if (!lethal_found.load(std::memory_order_relaxed)) {
						lethal_found.store(true, std::memory_order_release);
						return local_best;
					}
				}
			}
			return local_best.damage > 0 ? std::optional<aim_result_t>(local_best) : std::nullopt;
			}));
	}

	for (auto& future : futures) {
		if (auto result = future.get()) {
			if (result->damage > best_result.damage) {
				best_result = *result;
			}
		}
	}

	damage = best_result.damage;
	return best_result.point;
}

bool ragebot::multipoints(c_player_pawn* pawn, CBoneData* bones, int index, std::vector<aim_point_t>& points, bool predicted)
{

	int hitbox_from_menu = get_hitbox_from_menu(index);
	if (hitbox_from_menu == -1)
		return false;

	points.clear();

	auto hitbox_data = get_hitbox_data(pawn, index);

	if (hitbox_data.m_invalid_data)
		return false;

	auto bone_index = pawn->get_bone_index(hitbox_data.m_name_bone);
	bool is_chest = index == hitbox_chest || index == hitbox_upper_chest || index == hitbox_lower_chest;

	if (predicted) {
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if (index == hitbox_head && settings.cfg().ragebot.m_multi_points[0])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if (is_chest && settings.cfg().ragebot.m_multi_points[1])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if (index == hitbox_stomach && settings.cfg().ragebot.m_multi_points[2])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if ((HitboxToHitgroup(index) == e_hitgroups::hitgroup_leftarm || HitboxToHitgroup(index) == e_hitgroups::hitgroup_rightarm) && settings.cfg().ragebot.m_multi_points[3])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if ((HitboxToHitgroup(index) == e_hitgroups::hitgroup_leftleg || HitboxToHitgroup(index) == e_hitgroups::hitgroup_rightleg) && settings.cfg().ragebot.m_multi_points[4])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}

	if ((index == hitbox_left_foot || index == hitbox_right_foot) && settings.cfg().ragebot.m_multi_points[5])
	{
		add_point(bones[bone_index].position, bone_index, false, points);
		return true;
	}
	float scale = settings.cfg().ragebot.m_multipoint_scale;

	float radius = hitbox_data.m_radius * (scale / 100.f);

	vec3_t center{};
	vec3_t left = vec3_t(0.f, 0.f, radius);
	vec3_t right = vec3_t(0.f, 0.f, -radius);
	vec3_t back = vec3_t(0.f, -radius, 0.f);

	if (radius <= 0.f)
		return false;

	matrix3x4_t mat = math::transform_to_matrix(bones[bone_index]);

	if (hitbox_data.m_radius > 0.f)
	{
		switch (index)
		{
		case hitbox_head:
		{
			auto mini_peka = vec3_t(hitbox_data.m_maxs.x + radius, hitbox_data.m_maxs.y - radius, hitbox_data.m_maxs.z);
			left = vec3_t(hitbox_data.m_maxs.x, hitbox_data.m_maxs.y, hitbox_data.m_maxs.z - radius);
			right = vec3_t(hitbox_data.m_maxs.x, hitbox_data.m_maxs.y, hitbox_data.m_maxs.z + radius);
			add_point(left.transform(mat), bone_index, false, points);
			add_point(right.transform(mat), bone_index, false, points);
			add_point(mini_peka.transform(mat), bone_index, false, points);
		}
		break;
		case hitbox_chest:
		case hitbox_lower_chest:
		case hitbox_upper_chest:
		case hitbox_stomach:
			add_point(left.transform(mat), bone_index, false, points);
			add_point(right.transform(mat), bone_index, false, points);
			add_point(back.transform(mat), bone_index, false, points);
			break;
		default:
			add_point(bones[bone_index].position, bone_index, false, points);
			break;
		}
	}

	return true;
}

void ragebot::debug(c_player_pawn* pawn) {
	if (!pawn || !pawn->is_player_pawn() || !pawn->m_is_alive())
		return;

	auto record = g_ragebot->m_best_target->m_lag_record;// get_last_record(pawn->get_ref_ehandle().to_int());

	if (!record)
		return;

	auto game_scene = pawn->m_game_scene_node();

	if (!game_scene)
		return;

	auto skeleton = game_scene->get_skeleton_instance();

	if (!skeleton)
		return;

	c_model_state& model_state = skeleton->m_model_state();
	auto& model = model_state.modelHandle;

	if (!model)
		return;

	std::vector<aim_point_t> points;

	for (int i = 0; i < 19; i++) {
		if (!multi_points(record, i, points))
			continue;

		for (auto& point : points) {
			vec2_t	 vec_3d;
			if (g_render->world_to_screen(point.m_point, vec_3d))
				g_render->m_draw_list->AddRect({ vec_3d.x - 1, vec_3d.y - 1 }, { vec_3d.x + 1, vec_3d.y + 1 }, Color_t(255, 0, 255).GetU32());
		}
	}
}

void ragebot::select_target() {
    m_best_target.reset();
    if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive()) return;
    auto* weapon = sdk::m_local_pawn->ActiveWeapon();
    auto* data = weapon ? weapon->m_weapon_data() : nullptr;
    if (!data) return;
    auto nearest = get_nearest_target();
    std::vector<std::shared_ptr<aim_target_t>> candidates;
    if (nearest) candidates.push_back(nearest);
    for (const auto& [handle, target] : m_aim_targets)
        if (target && target != nearest) candidates.push_back(target);
    for (const auto& target : candidates) {
        auto* pawn = target->m_pawn;
        auto* record = target->m_lag_record;
        if (!pawn || !record || !record->is_valid() || !pawn->m_is_alive() || pawn->m_gun_game_immunity()) continue;
        int requested = g_inputsystem->IsKeyDown(settings.cfg().ragebot.key_min_damage_over)
            ? settings.cfg().ragebot.r_min_damage_over : settings.cfg().ragebot.r_min_damage;
        const int minimum = data->m_weapon_type() == WEAPONTYPE_TASER ? pawn->m_health() : (std::min)(requested, pawn->m_health());
        float damage = 0.f;
        record->apply(pawn);
        aim_point_t point({0.f,0.f,0.f}, -1);
        try { point = select_points(record, damage); }
        catch (...) { record->reset(pawn); continue; }
        record->reset(pawn);
        if (point.m_hitbox == -1 || damage < minimum) continue;
        target->m_best_point = std::make_shared<aim_point_t>(point);
        best_damage = damage;
        m_best_target = target;
        return;
    }
}

void ragebot::revolver_cock(c_user_cmd* cmd) {
	if (!settings.cfg().ragebot.b_ragebot)
		return;

	if (!sdk::m_local_pawn)
		return;

	if (!sdk::m_local_pawn->ActiveWeapon())
		return;

	auto weapon = sdk::m_local_pawn->ActiveWeapon();
	if (!weapon)
		return;

	auto attribute = weapon->m_attribute_manager();
	if (!attribute)
		return;

	auto item = attribute->m_item();
	if (!item)
		return;

	if (item->m_item_definition_index() != 64)
		return;

	if (bool valid_revolver = !(cmd->m_button_state.nValue & ECommandButtons::IN_RELOAD) && !(cmd->m_button_state.nValue & ECommandButtons::IN_ATTACK) && weapon->m_iclip1(); !valid_revolver)
		return;

	const auto server_time = g_prediction->get_local_data()->m_tick_base;

	if (weapon->m_postpone_fire_ready_ticks() > server_time) {
		if (server_time > next_cock_time && sdk::m_local_pawn->ActiveWeapon()->m_next_primary_attack_tick() <= server_time) {
			cmd->m_button_state.nValue |= IN_ATTACK;
		}
	}
	else {
		next_cock_time = server_time + 0.234375f/*int(0.25 * 64)*/;
	}
}

void ragebot::store_hitboxes() {
	if (settings.cfg().ragebot.m_hitboxes[0])
		m_hitboxes.emplace_back(hitbox_head);

	if (settings.cfg().ragebot.m_hitboxes[1]) {
		m_hitboxes.emplace_back(hitbox_chest);
		m_hitboxes.emplace_back(hitbox_lower_chest);
		m_hitboxes.emplace_back(hitbox_upper_chest);
	}

	if (settings.cfg().ragebot.m_hitboxes[2]) {
		m_hitboxes.emplace_back(hitbox_pelvis);
		m_hitboxes.emplace_back(hitbox_stomach);
	}

	if (settings.cfg().ragebot.m_hitboxes[3]) {
		m_hitboxes.emplace_back(hitbox_right_hand);
		m_hitboxes.emplace_back(hitbox_left_hand);
		m_hitboxes.emplace_back(hitbox_right_upper_arm);
		m_hitboxes.emplace_back(hitbox_right_forearm);
		m_hitboxes.emplace_back(hitbox_left_upper_arm);
		m_hitboxes.emplace_back(hitbox_left_forearm);
	}

	if (settings.cfg().ragebot.m_hitboxes[4]) {
		m_hitboxes.emplace_back(hitbox_right_thigh);
		m_hitboxes.emplace_back(hitbox_left_thigh);

		m_hitboxes.emplace_back(hitbox_right_calf);
		m_hitboxes.emplace_back(hitbox_left_calf);
	}

	if (settings.cfg().ragebot.m_hitboxes[5]) {
		m_hitboxes.emplace_back(hitbox_right_foot);
		m_hitboxes.emplace_back(hitbox_left_foot);
	}
}

bool ragebot::valid_interpolation_timing2()
{
	if (!g_interface->m_csgoinput->input_messages) return true;
	for (int i{}; i < std::clamp(g_interface->m_csgoinput->MessageSize, 0, 256); i++)
	{
		auto history = g_interface->m_csgoinput->input_messages[i];
		int player_tick_count = history.PlayerTickCount;
		float player_fraction = history.PlayerFraction;

		if (player_fraction >= float(1.0 - 0.0099999998))
			player_tick_count += 1;

		float time = player_tick_count;
		time += player_fraction;

		auto tick = sdk::m_local_pawn->get_lerp_tick(player_tick_count);

		// минус 1 тик ебливый предикт
		if (tick >= player_tick_count)
			return false;
	}

	return true;
}


void ragebot::run(csgoinput* pInput, c_user_cmd* pCmd)
{
	if (!settings.cfg().ragebot.b_ragebot)
		return;

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	c_user_cmd* user_cmd = sdk::m_cmd;
	if (!user_cmd)
		return;

	c_player_pawn* local_player = sdk::m_local_pawn;
	if (!local_player || !local_player->m_is_alive())
		return;

	g_movement->m_autopeek.did_shoot = false;

	if (!m_hitboxes.empty())
		m_hitboxes.clear();
	vec3_t qatovec{};

	c_weapon_base* active_weapon = local_player->ActiveWeapon();
	if (!active_weapon)
		return;

	c_cs_weapon_base_vdata* weapon_data = active_weapon->m_weapon_data();
	if (!weapon_data) return;
	bool is_taser = weapon_data->m_weapon_type() == WEAPONTYPE_TASER;

	try {
		if (m_best_target)
			m_best_target->reset();

		if (!weapon_data
			|| weapon_data->m_weapon_type() == WEAPONTYPE_KNIFE
			|| weapon_data->m_weapon_type() == WEAPONTYPE_GRENADE)
			return;

		store_hitboxes();

		if (m_hitboxes.empty())
			return;

		find_targets();

		select_target();

		if (!m_best_target || !m_best_target->m_best_point)
			return;

		auto local_data = g_prediction->get_local_data();

		if (!local_data)
			return;

		if (!ragebot::can_shoot(active_weapon))
			return;

		//if (!valid_interpolation_timing2())
		//	return;
		auto* no_spread_cvar = g_interface->cvar->Find(fnv1a::hash("weapon_accuracy_nospread"));
        bool b_no_spread = no_spread_cvar && no_spread_cvar->value.i1;

		if ((weapon_data->m_weapon_type() == WEAPONTYPE_SNIPER_RIFLE)
			&& !(user_cmd->m_button_state.nValue & IN_ZOOM)
			&& !local_player->m_is_scoped()
			&& (local_player->m_fflags() & FL_ONGROUND)
			&& !b_no_spread)
			user_cmd->m_button_state.nValue |= IN_SECOND_ATTACK;

		if ((uintptr_t)m_best_target->m_best_point.get() == 0xdddddddddddddddd)
			return;

		m_best_target->m_lag_record->apply(m_best_target->m_pawn);
		qangle_t angle = math::CalcAngel(local_data->m_eye_pos, m_best_target->m_best_point->m_point);

		qangle_t aim_punch = {};
		qatovec = { angle.x,angle.y,angle.z };
		g_movement->auto_stop(sdk::m_cmd, sdk::m_local_pawn, active_weapon, b_no_spread);

		if (g_hook->fnUpdateAimPunch_fn) g_hook->fnUpdateAimPunch_fn(sdk::m_local_pawn, &aim_punch, true);
		qatovec.x -= aim_punch.x;
		qatovec.y -= aim_punch.y;
		qatovec.z -= aim_punch.z;

	}
	catch (...)
	{
		std::cout << "[ERROR] ragebot" << std::endl;
	}

	int hit_chance = hitchance(qatovec, active_weapon, m_best_target->m_pawn);
	if (hit_chance >= (is_taser ? 70 : settings.cfg().ragebot.r_hitchance)) {
		for (int i = 0; i < user_cmd->pb.input_history_size(); i++) {
			auto tick = user_cmd->pb.mutable_input_history(i);
			if (tick) {
				tick->mutable_view_angles()->set_x(qatovec.x);
				tick->mutable_view_angles()->set_y(qatovec.y);
				tick->mutable_view_angles()->set_z(qatovec.z);
			}
		}
		//pInput->SetViewAngles(qatovec);
		//if(!settings.cfg().ragebot.b_silent)
		//	g_interface->m_csgoinput->SetViewAngles(m_best_target->m_best_point->m_point);
        user_cmd->pb.mutable_base()->mutable_viewangles()->set_x(qatovec.x);
        user_cmd->pb.mutable_base()->mutable_viewangles()->set_y(qatovec.y);
		process_attacka(user_cmd);
	}
	g_movement->m_autopeek.did_shoot = true;


	m_best_target->m_lag_record->reset(m_best_target->m_pawn);
}