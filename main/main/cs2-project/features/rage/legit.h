#pragma once
#include "ragebot.h"
#include "autowall.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../sdk/game_class/interfaces/csgoinput.h"
#include "../../sdk/entity_listener/entity_list.h"

namespace LEGIT {
inline void aim(csgoinput* input, c_user_cmd* cmd) {
    const auto& cfg = settings.cfg().legitbot;
    auto* local = sdk::m_local_pawn;
    if (settings.cfg().ragebot.b_ragebot || (!cfg.b_aim_fov && !cfg.b_trigger) ||
        !input || !cmd || !cmd->pb.has_base() || !local || !local->m_is_alive()) return;
    auto* weapon = local->ActiveWeapon();
    auto* data = weapon ? weapon->m_weapon_data() : nullptr;
    if (!data || data->m_weapon_type() == WEAPONTYPE_KNIFE || data->m_weapon_type() == WEAPONTYPE_GRENADE) return;
    auto* base = cmd->pb.mutable_base();
    vec3_t view{base->viewangles().x(), base->viewangles().y(), 0.f};
    const auto eye = local->get_eye_pos();
    float best_fov = cfg.aim_Fov;
    vec3_t best_angle{};
    bool found = false;
    if (cfg.b_aim_fov && (cmd->m_button_state.nValue & IN_ATTACK)) {
        for (auto* entity : g_entlistener->get("CCSPlayerController")) {
            auto* controller = reinterpret_cast<c_player_controller*>(entity);
            auto* pawn = controller ? controller->get_player_pawn() : nullptr;
            auto* node = pawn ? pawn->m_game_scene_node() : nullptr;
            if (!node || node->m_dormant() || pawn == local || !pawn->m_is_alive() ||
                pawn->m_team_num() == local->m_team_num() || pawn->m_gun_game_immunity()) continue;
            auto* skeleton = node->get_skeleton_instance();
            auto* bones = skeleton ? skeleton->get_bone_cache() : nullptr;
            if (!bones || skeleton->get_bone_count() <= bone_head) continue;
            const vec3_t target = reinterpret_cast<CBoneData*>(bones)[bone_head].position;
            if (cfg.legit_visible) {
                Ray_t ray{}; game_trace_t trace{};
                TraceFilter_t filter(0x1C3003, local, nullptr, 4);
                g_interface->m_game_trace->trace_shape(&ray, eye, target, &filter, &trace);
                if (trace.HitEntity != pawn && !trace.IsVisible()) continue;
            }
            const auto desired = math::CalcAngel(eye, target);
            const float pitch = std::remainder(desired.x - view.x, 360.f);
            const float yaw = std::remainder(desired.y - view.y, 360.f);
            const float fov = std::hypot(pitch, yaw);
            if (!std::isfinite(fov) || fov >= best_fov) continue;
            best_fov = fov; best_angle = {pitch, yaw, 0.f}; found = true;
        }
        if (found) {
            const float smooth = std::clamp(cfg.legit_smooth, 0.1f, 1.f);
            view.x = std::clamp(view.x + best_angle.x * smooth, -89.f, 89.f);
            view.y = std::remainder(view.y + best_angle.y * smooth, 360.f);
            base->mutable_viewangles()->set_x(view.x);
            base->mutable_viewangles()->set_y(view.y);
            cmd->SetSubTickAngle({view.x, view.y, 0.f});
            input->SetViewAngles(view);
        }
    }
    if (!cfg.b_trigger || !g_ragebot->can_shoot(weapon)) return;
    vec3_t forward{};
    math::angle_vectors(view, forward);
    Ray_t ray{}; game_trace_t trace{};
    TraceFilter_t filter(0x1C3003, local, nullptr, 4);
    g_interface->m_game_trace->trace_shape(&ray, eye, eye + forward * data->m_range(), &filter, &trace);
    auto* target = trace.HitEntity;
    if (!target || !target->is_player_pawn() || !target->m_is_alive() || target->m_gun_game_immunity() ||
        target->m_team_num() == local->m_team_num()) return;
    const float damage = g_autowall->penetration_damage(eye, trace.m_end_pos, local, local, target, target,
        false, target->m_team_num(), data);
    const float required = cfg.trigger_min_dmg > 100 ? target->m_health() : cfg.trigger_min_dmg;
    if (damage < required || g_ragebot->hitchance(view, weapon, target) < cfg.trigger_hitchance) return;
    cmd->m_button_state.nValue |= IN_ATTACK;
    cmd->m_button_state.nValueChanged |= IN_ATTACK;
    if (cmd->pb.input_history_size() > 0) cmd->pb.set_attack1_start_history_index(cmd->pb.input_history_size() - 1);
}
}
