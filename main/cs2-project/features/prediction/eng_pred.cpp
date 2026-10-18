#include "eng_pred.h"
#include "../../sdk/sdk.h"
#include "../../sdk/game_class/interfaces/i_global_vars.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
void c_eng_pred::run() {
    m_initialized=false;
    if(!sdk::m_cmd || !sdk::m_local_pawn || !sdk::m_local_controller || !sdk::m_local_pawn->m_is_alive()) return;
    m_pred_data={};
    m_pred_data.m_tick_base=sdk::m_local_controller->m_tick_base();
    m_pred_data.m_shoot_tick=m_pred_data.m_tick_base;
    m_pred_data.m_eye_pos=sdk::m_local_pawn->get_eye_pos();
    m_pred_data.m_velocity=sdk::m_local_pawn->m_vec_velocity();
    if(auto* info=g_interface->m_engine->get_networked_client_info()) {
        m_pred_data.m_player_tick=info->m_player_tick_count;
        m_pred_data.m_player_tick_fraction=info->m_player_tick_fraction;
        m_pred_data.m_render_tick=info->m_render_tick;
        m_pred_data.m_render_tick_fraction=info->m_render_tick_fraction;
    }
    if(auto* weapon=sdk::m_local_pawn->ActiveWeapon()) {
        weapon->update_accuracy_penality();
        m_pred_data.m_spread=weapon->get_spread();m_pred_data.m_inaccuracy=weapon->get_inaccuracy();
    }
    m_initialized=true;
}
void c_eng_pred::predict() {run();}
void c_eng_pred::end() {m_initialized=false;}
