#include "movement.h"
#include "../../dependencies/config/Include/config.h"
#include "../../sdk/math/math.h"
#include <algorithm>
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../sdk/game_class/interfaces/csgoinput.h"
#include "../../sdk/game_class/cconvar.h"
#include "../../sdk/math/math.h"
#include "../../sdk/protobuffs/protobufs.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../sdk/input_system/inputsystem.h"
#include "../prediction/eng_pred.h"

void c_movement::angle_qangles(const qangle_t& angles, qangle_t* forward, qangle_t* right, qangle_t* up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	// Convert angles from degrees to radians
	angle = angles.y * (math::_PI / 180.0);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * (math::_PI / 180.0);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * (math::_PI / 180.0);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}	

void c_movement::auto_pick(c_user_cmd* pCmd) {
	if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive())
		return;
	auto m_data = &m_autopeek;
	if (settings.cfg().movement.b_auto_pick && g_inputsystem->IsKeyDown(settings.cfg().misc.i_key_autopeek)) {
		if (!m_data->m_autopeeking) {
			m_data->m_should_retrack = false;
			m_data->m_old_origin = sdk::m_local_pawn->m_game_scene_node()->m_vec_abs_origin();

			game_trace_t trace = game_trace_t();
			TraceFilter_t filter = TraceFilter_t(0x3001, nullptr, nullptr, 3);
			filter.collision2 = 0x10000;

			Ray_t ray{};
			g_interface->m_game_trace->trace_shape(&ray, sdk::m_local_pawn->get_eye_pos(), sdk::m_local_pawn->get_eye_pos() - vec3_t(0, 0, 1000.f), &filter, &trace);

			if (trace.Fraction < 1.0f)
				m_data->m_old_origin = trace.m_end_pos + vec3_t(0.0f, 0.0f, 2.0f);

		}
		if (m_data->m_should_retrack) {
			auto distance = sdk::m_local_pawn->m_game_scene_node()->m_vec_abs_origin().dist_to(m_data->m_old_origin);
			if (distance <= 16.0) {
				m_data->m_autopeeking = false;
				m_data->m_should_retrack = false;
			}
			else {
				vec3_t ang_to_pos;
				vec3_t local_angle = g_interface->m_csgoinput->GetViewAngles();
				auto difference = m_data->m_old_origin - sdk::m_local_pawn->m_game_scene_node()->m_vec_abs_origin();
				math::vector_angles4(difference, ang_to_pos);
				auto v45 = cos(ang_to_pos.x * 0.017453292);

				pCmd->pb.mutable_base()->set_forwardmove(0.0);
				pCmd->pb.mutable_base()->set_leftmove(0.0);

				auto v48 = math::deg2rad((local_angle.y - ang_to_pos.y));
				auto v66 = math::deg2rad((local_angle.y - ang_to_pos.y));
				auto cos_v48 = cos(v48);
				auto sin_v48 = sin(v66);

				auto v50 = cos_v48 * v45;

				pCmd->pb.mutable_base()->set_forwardmove(fmaxf(fminf(1.0, v50), -1.0));
				pCmd->pb.mutable_base()->set_leftmove(-fmaxf(fminf(1.0, (float)(sin_v48 * v45)), -1.0));
			}
		}
		if (m_data->did_shoot) {
			m_data->m_should_retrack = true;
		}
		m_data->m_autopeeking = true;
	}
	else {
		m_data->m_autopeeking = false;
		m_data->m_should_retrack = false;
	}
}

void c_movement::bunnyhop(c_user_cmd* pCmd, c_player_pawn* pLocalPawn)
{
	if (!pCmd || !pLocalPawn || !pCmd->pb.has_base()) return;
	if (!pCmd || !pLocalPawn || !pCmd->pb.has_base()) return;
	auto* auto_bhop = g_interface->cvar->Find(fnv1a::hash("sv_autobunnyhopping"));
	if (!settings.cfg().movement.b_bunnyhope || (auto_bhop && auto_bhop->value.i1))
		return;

	if (!sdk::m_local_controller || !sdk::m_local_pawn)
		return;

	if (!sdk::m_local_pawn->m_is_alive() || sdk::m_local_pawn->get_water_level() >= 2 || sdk::m_local_pawn->m_move_type() == 7 || sdk::m_local_pawn->m_move_type() == 9)
		return;

	//if (!(pCmd->m_button_state.nValue & IN_JUMP || pCmd->m_button_state.nValueChanged & IN_JUMP))
	//	return;

	if (!(pLocalPawn->m_fflags() & FL_ONGROUND)) {
		pCmd->m_button_state.nValue &= ~IN_JUMP;
        pCmd->m_button_state.nValueChanged &= ~IN_JUMP;
        pCmd->m_button_state.nValueScroll &= ~IN_JUMP;
        auto* base = pCmd->pb.mutable_base();
        for (int i=0;i<base->subtick_moves_size();++i) {
            auto* step=base->mutable_subtick_moves(i);
            if (step && (step->button() & IN_JUMP)) step->set_pressed(false);
        }
	}
	// subtick sucks
	return;

	//if (!(pLocalPawn->m_fflags() & FL_ONGROUND)) {
	//	return;
	//}

        pCmd->m_button_state.nValueChanged &= ~IN_JUMP;
        pCmd->m_button_state.nValueScroll &= ~IN_JUMP;
	//sdk::m_cmd->m_button_state.nValueChanged &= ~IN_JUMP;
	//sdk::m_cmd->m_button_state.nValueScroll &= ~IN_JUMP;

	//bool move_left = sdk::m_cmd->m_button_state.nValue & IN_MOVELEFT, move_right = sdk::m_cmd->m_button_state.nValue & IN_MOVERIGHT;

	//if (pLocalPawn->m_fflags() & FL_ONGROUND) {
	//	m_scrolls = 1;
	//	if (move_left || move_right)
	//		m_max_scrolls = 5;
	//	else
	//		m_max_scrolls = 3;
	//}
	//const int max_ticks = 12; //experiment

	//// Distribute the subticks across the frame
	//for (int i = 0; i < max_ticks; ++i)
	//{
	//	auto subtick = g_protobuf->add_subtick_move_step(pCmd);

	//	// Set the timing for the subtick move (even distribution)
	//	subtick->set_when(static_cast<float>(i) / static_cast<float>(max_ticks));

	//	if (i == 0) // First subtick: Press the jump button
	//	{
	//		subtick->set_button(IN_JUMP);
	//		subtick->set_pressed(true);
	//	}
	//	else if (i == max_ticks - 1) // Last subtick: Release the jump button
	//	{
	//		subtick->set_button(IN_JUMP);
	//		subtick->set_pressed(false);
	//	}
	//}

	//if (m_scrolls && m_scrolls < m_max_scrolls)
	//{
	//	sdk::m_cmd->m_button_state.nValueScroll |= IN_JUMP;
	//	auto subtick = g_protobuf->add_subtick_move_step(pCmd);
	//	pCmd->pb.base();
	//	subtick->set_button(IN_JUMP);
	//	subtick->set_when(11.f/);
	//	subtick->set_pressed(true);
	//	subtick->clear_button();
	//	subtick->set_button(IN_JUMP);
	//	subtick->set_when(1);
	//	subtick->set_pressed(false);

	//	m_scrolls++;
	//}
}

void c_movement::AutoStrafe(c_user_cmd* pCmd, c_player_pawn* pLocalPawn)
{
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	if (!sdk::m_local_pawn || !sdk::m_local_controller->m_pawn_is_alive())
		return;

	if (pLocalPawn->get_water_level() >= 2)//WL_WAIST
		return;

	if (pLocalPawn->m_fflags() & FL_ONGROUND)
		return;
	// 1794 - noclip, 2313 - ladder
	if (!sdk::m_local_pawn->m_is_alive() || sdk::m_local_pawn->m_move_type() == 7 || sdk::m_local_pawn->m_move_type() == 9)
		return;

	static uint64_t last_pressed = 0;
	static uint64_t last_buttons = 0;

	if (!settings.cfg().movement.b_autostrafe)
		return;

	const auto current_buttons = pCmd->m_button_state.nValue;
	float yaw{};
	if (pCmd->pb.mutable_base()->mutable_viewangles())
		yaw = math::normalize_yaw(pCmd->pb.base().viewangles().y());

	const auto check_button = [&](const uint64_t button)
		{
			if (current_buttons & button && (!(last_buttons & button) || button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT) || button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT) || button & IN_FORWARD && !(last_pressed & IN_BACK) || button & IN_BACK && !(last_pressed & IN_FORWARD)))
			{
				if (button & IN_MOVELEFT)
					last_pressed &= ~IN_MOVERIGHT;
				else if (button & IN_MOVERIGHT)
					last_pressed &= ~IN_MOVELEFT;
				else if (button & IN_FORWARD)
					last_pressed &= ~IN_BACK;
				else if (button & IN_BACK)
					last_pressed &= ~IN_FORWARD;

				last_pressed |= button;
			}
			else if (!(current_buttons & button))
				last_pressed &= ~button;
		};

	check_button(IN_MOVELEFT);
	check_button(IN_MOVERIGHT);
	check_button(IN_FORWARD);
	check_button(IN_BACK);

	last_buttons = current_buttons;

	const auto velocity = pLocalPawn->m_vec_abs_velocity();

	auto offset = 0.f;
	if (last_pressed & IN_MOVELEFT)
		offset += 90.f;
	if (last_pressed & IN_MOVERIGHT)
		offset -= 90.f;
	if (last_pressed & IN_FORWARD)
		offset *= 0.5f;
	else if (last_pressed & IN_BACK)
		offset = -offset * 0.5f + 180.f;

	yaw += offset;

	auto velocity_angle = M_RAD2DEG(std::atan2f(velocity.y, velocity.x));
	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	const auto speed = velocity.length2d();
	auto ideal = 0.f;

	if (speed > 0.f)
		ideal = math::clamp(M_RAD2DEG(std::atan2(15.f, speed)), 0.0f, 90.0f);

	auto correct = (100.f - settings.cfg().movement.smooth_stafe) * 0.02f * (ideal + ideal);

	pCmd->pb.mutable_base()->set_forwardmove(0.f);
	const auto velocity_delta = math::normalize_yaw(yaw - velocity_angle);

	auto rotate_movement = [](c_user_cmd* pCmd, float target_yaw)
		{
			const float rot = M_DEG2RAD(pCmd->pb.base().viewangles().y() - target_yaw);

			const float new_forward = std::cos(rot) * pCmd->pb.base().forwardmove() - std::sin(rot) * pCmd->pb.base().leftmove();
			const float new_side = std::sin(rot) * pCmd->pb.base().forwardmove() + std::cos(rot) * pCmd->pb.base().leftmove();

			pCmd->m_button_state.nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);
			pCmd->m_button_state.nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

			pCmd->pb.mutable_base()->set_forwardmove(math::clamp(new_forward, -1.f, 1.f));
			pCmd->pb.mutable_base()->set_leftmove(math::clamp(new_side * -1.f, -1.f, 1.f));

			if (pCmd->pb.base().forwardmove() > 0.0f)
			{
				//Interfaces::csgo_input->add_button(IN_FORWARD);
				pCmd->m_button_state.nValue |= IN_FORWARD;
			}
			else if (pCmd->pb.base().forwardmove() < 0.0f)
			{
				//Interfaces::csgo_input->add_button(IN_BACK);
				pCmd->m_button_state.nValue |= IN_BACK;
			}
			if (pCmd->pb.base().leftmove() > 0.0f)
			{
				//Interfaces::csgo_input->add_button(IN_MOVELEFT);
				pCmd->m_button_state.nValue |= IN_MOVELEFT;
			}
			else if (pCmd->pb.base().leftmove() < 0.0f)
			{
				//Interfaces::csgo_input->add_button(IN_MOVERIGHT);
				//pUserCmd->pInButtonState->SetBits(IN_MOVERIGHT);

				pCmd->m_button_state.nValue |= IN_MOVERIGHT;
			}
		};

	if (fabsf(velocity_delta) > 170.0f && speed > 80.0f || velocity_delta > correct && speed > 80.0f)
	{
		yaw = correct + velocity_angle;
		pCmd->pb.mutable_base()->set_leftmove(-1.0f);
		rotate_movement(pCmd, math::normalize_yaw(yaw));
		return;
	}
	static bool m_switch_value;
	m_switch_value = !m_switch_value;

	if (-correct <= velocity_delta || speed <= 80.f)
	{
		if (m_switch_value)
		{		
			yaw = yaw - ideal;
			pCmd->pb.mutable_base()->set_leftmove(-1.0f);
		}
		else
		{
			yaw = ideal + yaw;
			pCmd->pb.mutable_base()->set_leftmove(1.0f);
		}
	}
	else
	{
		yaw = velocity_angle - correct;
		pCmd->pb.mutable_base()->set_leftmove(1.0f);
	}

	rotate_movement(pCmd, math::normalize_yaw(yaw));
	//if (!C_GET(bool, Vars.bSubtickStrafe))
	//	return;

	g_interface->m_csgoinput->flForwardMove = pCmd->pb.base().forwardmove();
	g_interface->m_csgoinput->flSideMove = pCmd->pb.base().leftmove();
	if (pCmd->pb.mutable_base()->subtick_moves().empty())
		return;

	for (auto i = 0; i < pCmd->pb.base().subtick_moves_size(); i++)
	{
		auto subtick = pCmd->pb.mutable_base()->mutable_subtick_moves(i);

		subtick->set_analog_forward_delta(pCmd->pb.base().forwardmove());
		subtick->set_analog_left_delta(pCmd->pb.base().leftmove());

		subtick->set_button(IN_SPRINT);
		subtick->set_when((1.f / 12.f) * (i));
	}

	auto subtick = g_protobuf->add_subtick_move_step(pCmd);
	subtick->set_analog_forward_delta(pCmd->pb.base().forwardmove());
	subtick->set_analog_left_delta(pCmd->pb.base().leftmove());

	subtick->set_button(pCmd->pb.base().buttons_pb().buttonstate1());
	subtick->set_when((1.f / 12.f) * pCmd->pb.base().subtick_moves_size());

}

float c_movement::get_max_player_speed() {
	if (!sdk::m_local_pawn->ActiveWeapon() || !sdk::m_local_pawn->ActiveWeapon()->m_weapon_data())
		return 260.f;

	return sdk::m_local_pawn->m_is_scoped() ? sdk::m_local_pawn->ActiveWeapon()->get_max_speed() : sdk::m_local_pawn->ActiveWeapon()->get_max_speed();
}

void c_movement::stop_movement(c_user_cmd* user_cmd, c_player_pawn* local_player) {
	c_player_movement_service* movement_services = local_player->m_movement_services();

	if (!movement_services)
		return;

	vec3_t velocity = g_prediction->get_local_data()->m_velocity;
	float speed = velocity.length2d();
	if (speed < 1.f) {
		user_cmd->pb.mutable_base()->set_forwardmove(0.f);
		user_cmd->pb.mutable_base()->set_leftmove(0.f);
		return;
	}

	auto* acceleration = g_interface->cvar->Find(fnv1a::hash("sv_accelerate"));
    auto* maxspeed = g_interface->cvar->Find(fnv1a::hash("sv_maxspeed"));
    if (!acceleration || !maxspeed || acceleration->value.fl <= 0.f) return;
    const auto max_accelspeed = acceleration->value.fl * maxspeed->value.fl * movement_services->m_surface_friction() * INTERVAL_PER_TICK;

	auto get_maximum_accelerate_speed = [&]
		{
			const auto speed_ratio = speed / (acceleration->value.fl * INTERVAL_PER_TICK);
			return speed - max_accelspeed <= -1.f ? max_accelspeed / speed_ratio : max_accelspeed;
		};

	vec3_t view_angles{};
	view_angles = g_interface->m_csgoinput->GetViewAngles();

	qangle_t velocity_angle{ };
	math::VectorAngles(velocity * -1.f, velocity_angle);
	velocity_angle.y = view_angles.y - velocity_angle.y;

	vec3_t forward{ };
	math::angle_vectors({ velocity_angle.x,velocity_angle.y,velocity_angle.z }, forward);

	const auto total_speed = get_maximum_accelerate_speed();
	const auto max_weapon_speed = get_max_player_speed();

	user_cmd->pb.mutable_base()->set_forwardmove((forward.x * total_speed) / max_weapon_speed);
	user_cmd->pb.mutable_base()->set_leftmove((forward.y * -total_speed) / max_weapon_speed);
}

void c_movement::auto_stop(c_user_cmd* user_cmd, c_player_pawn* local_player, c_base_player_weapon* active_weapon, bool no_spread) {
	//if (!settings.cfg().movement)
	//	return;

	if (no_spread)
		return;

	if (!(local_player->m_fflags() & FL_ONGROUND))
		return;

	vec3_t velocity = g_prediction->get_local_data()->m_velocity;

	if (velocity.length2d() <= 1.f)
		return;

	float max_accurate_speed = get_max_player_speed() * 0.25f;

	if (velocity.length2d() <= max_accurate_speed)
		return limit_speed(user_cmd, local_player);
	
	stop_movement(user_cmd, local_player);
}

void c_movement::limit_speed(c_user_cmd* user_cmd, c_player_pawn* local_player) {
	c_player_movement_service* movement_services = local_player->m_movement_services();
	if (!movement_services)
		return;

	vec3_t velocity = g_prediction->get_local_data()->m_velocity;

	float max_speed = get_max_player_speed();
	max_speed *= 0.033999f;

	const float distance_to_minimal_speed = velocity.length2d() / max_speed;
	if (distance_to_minimal_speed <= 0.0f)
		return;

	const float forward_move = user_cmd->pb.mutable_base()->forwardmove() / distance_to_minimal_speed;

	user_cmd->pb.mutable_base()->set_forwardmove(forward_move);

	const float side_move = user_cmd->pb.mutable_base()->leftmove() / distance_to_minimal_speed;

	user_cmd->pb.mutable_base()->set_leftmove(side_move);
}

void c_movement::movement_fix(c_user_cmd* user_cmd, vec3_t angle)
{
	vec3_t wish_angle{ user_cmd->pb.mutable_base()->viewangles().x(), user_cmd->pb.mutable_base()->viewangles().y(), user_cmd->pb.mutable_base()->viewangles().z() };
	int sign = wish_angle.x > 89.f ? -1.f : 1.f;
	wish_angle.clamp();

	vec3_t forward, right, up, old_forward, old_right, old_up;
	vec3_t view_angles = angle;

	math::angle_vectors2(wish_angle, forward, right, up);

	forward.z = right.z = up.x = up.y = 0.f;

	forward.normalize_in_place();
	right.normalize_in_place();
	up.normalize_in_place();

	math::angle_vectors2(view_angles, old_forward, old_right, old_up);

	old_forward.z = old_right.z = old_up.x = old_up.y = 0.f;

	old_forward.normalize_in_place();
	old_right.normalize_in_place();
	old_up.normalize_in_place();

	forward *= user_cmd->pb.mutable_base()->forwardmove();
	right *= user_cmd->pb.mutable_base()->leftmove();
	up *= user_cmd->pb.mutable_base()->upmove();

	float fixed_forward_move = old_forward.dot(right) + old_forward.dot(forward) + old_forward.dot(up, true);

	float fixed_side_move = old_right.dot(right) + old_right.dot(forward) + old_right.dot(up, true);

	float fixed_up_move = old_up.dot(right, true) + old_up.dot(forward, true) + old_up.dot(up);

	user_cmd->pb.mutable_base()->set_forwardmove(fixed_forward_move);
	user_cmd->pb.mutable_base()->set_leftmove(fixed_side_move);
	user_cmd->pb.mutable_base()->set_upmove(fixed_up_move);

	fixed_forward_move = sign * (old_forward.dot(right) + old_forward.dot(forward));
	fixed_side_move = old_right.dot(right) + old_right.dot(forward);

	user_cmd->pb.mutable_base()->set_forwardmove(std::clamp(fixed_forward_move, -1.f, 1.f));
	user_cmd->pb.mutable_base()->set_leftmove(std::clamp(fixed_side_move, -1.f, 1.f));
}
