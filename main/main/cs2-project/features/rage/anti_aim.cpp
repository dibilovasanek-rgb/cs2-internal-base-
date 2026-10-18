#include "anti_aim.h"
#include "../../dependencies/config/Include/config.h"

void c_antiaim::runAA(c_user_cmd* pCmd, c_player_controller* pLocalController, c_player_pawn* pLocalPawn)
{
    if (!settings.cfg().ragebot.b_anti_aim)
        return;

    if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
        return;

    if (!pLocalPawn || pLocalPawn->m_move_type() == 7 || pLocalPawn->m_move_type() == 9)
        return;

    if (!pLocalPawn->m_is_alive())
        return;

    if (pCmd->m_button_state.nValue & (IN_USE | IN_ATTACK | IN_SECOND_ATTACK)) // Мануал на 'E' (неизменяемый)
        return;

    if (pLocalPawn->is_throwing())
        return;

    if (!pCmd->pb.mutable_base()->mutable_viewangles())
        return;

    float flRealPitch = 0;
    float flRealYaw = 0;

    switch (settings.cfg().ragebot.aa_style)
    {
    case BACKWARDS:
        flRealYaw += 179.f;
        flRealPitch = 89.f;
        break;
    case CUSTOM:
        flRealPitch = settings.cfg().ragebot.aa_pitch;
        flRealYaw = settings.cfg().ragebot.aa_yaw;
        break;
    case JITTER:
        auto nTickBase = pLocalController->m_tick_base();
        if (settings.cfg().ragebot.jitter_speed <= 0)
            break;

        if (nTickBase % settings.cfg().ragebot.jitter_speed)
            flRealYaw -= 89;
        else
            flRealYaw += 89;
        break;
    }

    bool bLeftManual = false;
    bool bRightManual = false;

    if (settings.cfg().ragebot.i_style_manual == 0) {
        bLeftManual = g_inputsystem->IsKeyDown(settings.cfg().ragebot.i_key_left_manual);
        bRightManual = g_inputsystem->IsKeyDown(settings.cfg().ragebot.i_key_right_manual);
    }
    else {
        static bool bLeftToggled = false;
        static bool bRightToggled = false;

        if (g_inputsystem->IsKeyReleased(settings.cfg().ragebot.i_key_left_manual))
            bLeftToggled = !bLeftToggled;
        if (g_inputsystem->IsKeyReleased(settings.cfg().ragebot.i_key_right_manual))
            bRightToggled = !bRightToggled;

        bLeftManual = bLeftToggled;
        bRightManual = bRightToggled;
    }

    if (bLeftManual) {
        flRealYaw += 89.f;
    }
    if (bRightManual) {
        flRealYaw -= 89.f;
    }

    // Применение углов
    pCmd->pb.mutable_base()->mutable_viewangles()->set_x(std::clamp(flRealPitch, -89.f, 89.f));
    pCmd->pb.mutable_base()->mutable_viewangles()->set_y(std::remainder(flRealYaw + pCmd->pb.base().viewangles().y(), 360.f));
}
