#include "additionally.h"
#include <cmath>
#include "../../sdk/sdk.h"
#include "../../sdk/schema/class/entity.h"
#include "../../sdk/schema/class/player.h"
#include "../../sdk/entity_listener/entity_list.h"
#include "../../dependencies/config/Include/config.h"
#include "../../sdk/game_class/interfaces/engine_client.h"
#include "../../render/render.h"
#include "../../core/hooks/hooks.h"



void ADD::drawBinds(ImDrawList* drawList)
{
	g_hook->last_func = "drawBinds";
	if (settings.cfg().visuals.b_render_binds) {
		if (bMainWindowOpened) {
			ImGui::Begin("binds");
			ImGui::End();
		}
		else {
			if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
				return;
			if (!sdk::m_local_pawn)
				return;
			if (!sdk::m_local_pawn->m_is_alive())
				return;
			if (manual_left || manual_right || on_thirdperson) {
				ImGui::Begin("binds", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
				if (manual_left)
					ImGui::Text("<- Manual");
				if (manual_right)
					ImGui::Text("Manual ->");
				if (on_thirdperson)
					ImGui::Text("ThirdPerson [ON]");
				ImGui::End();
			}
		}
	}
}