#include "event.h"
#include "../../schema/class/player.h"
#include "../../../features/skinchanger/skinchanger.h"
#include "../../../dependencies/config/Include/config.h"
#include "../interfaces/igameentitysystem.h"
#include "../../sdk.h"
#include "../../game_class/interfaces/i_resource_system.h"	
#include "../../../entry.h"
#include "../../../features/rage/ragebot.h"
#include "../../game_class/interfaces/i_global_vars.h"	
#include "../../../features/visual/game_visuals/bullet_trace.h"
#include "../../../render/render.h"
#include "../../../features/visual/game_visuals/world.h"
#include "../../../features/visual/game_visuals/model_changer.h"

std::int64_t c_game_event::get_int(const std::string_view event_name) noexcept
{
return get_int2(event_name.data(),false);
}

int c_game_event_helper::get_damage()
{
	if (!Event)
		return -1;

	return Event->get_int2("dmg_health", false);
}

int c_game_event::get_int2(const char* Name, bool Unk)
{
    Buffer key(Name); return g_memory->CallVFunc<int>(this, 7U, &key, 0);
}

bool c_event::Intilization()
{
    if (!g_interface->m_event) return false;
	g_interface->m_event->AddListener(this, "player_hurt", false);
	if (!g_interface->m_event->FindListener(this, "player_hurt"))
		return false;

	g_interface->m_event->AddListener(this, "player_death", false);
	if (!g_interface->m_event->FindListener(this, "player_death"))
		return false;

	//I::GameEventManager->AddListener(this, "add_bullet_hit_marker", false);
	//if (!I::GameEventManager->FindListener(this, "add_bullet_hit_marker"))
	//	return false;

	g_interface->m_event->AddListener(this, "bullet_impact", false);
	if (!g_interface->m_event->FindListener(this, "bullet_impact"))
		return false;

	g_interface->m_event->AddListener(this, "round_start", false);
	if (!g_interface->m_event->FindListener(this, "round_start"))
		return false;

	//I::GameEventManager->AddListener(this, "weapon_fire", false);
	//if (!I::GameEventManager->FindListener(this, "weapon_fire"))
	//	return false;

	g_interface->m_event->AddListener(this, "item_equip", false);
	if (!g_interface->m_event->FindListener(this, "item_equip"))
		return false;

	return true;
}

void c_event::FireGameEvent(c_game_event* event)
{
	std::string name = event->GetName();

	if (name.find("bullet_impact") != std::string::npos)
		bullet_impact(event);

	if (name.find("round_start") != std::string::npos)
	{
		g_skins->force_update = true;
		sdk::on_round_start = true;
		hit_capibara.clear();

	//	g_interface->m_engine->client_cmd_unrestricted("buy ssg08");
	}
	//cheat->mShouldClearNotice = true;

	if (name.find("player_death") != std::string::npos)
	{
		g_skins->force_update = true;
		player_death(event);
	}

	//	PlayerDeath(event);

	if (name.find("player_hurt") != std::string::npos)
		player_hurt(event);

	if (name.find("item_equip") != std::string::npos)
		g_modelchanger->SetWeaponModel(sdk::m_local_pawn);
}

namespace {
vec3_t ParticlePosition(c_player_pawn* pawn, int type) {
    if (!pawn) return {};
    if (type != 1) return pawn->get_scene_origin();
    auto* node = pawn->m_game_scene_node();
    auto* skeleton = node ? node->get_skeleton_instance() : nullptr;
    if (skeleton && skeleton->get_bone_count() > bone_head && skeleton->m_model_state().bones) {
        const auto head = skeleton->m_model_state().bones[bone_head].position;
        if (std::isfinite(head.x) && std::isfinite(head.y) && std::isfinite(head.z)) return head;
    }
    return pawn->get_eye_pos();
}
}

void c_event::player_death(c_game_event* event)
{
	if (!sdk::m_local_pawn)
		return;

	auto player = event->get_event_helper().get_player_controller();
	if (!player)
		return;

	if (!player->is_controller())
		return;

	if (player->m_is_local_player_controller())
		return;

	c_player_pawn* hurt_pawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(player->m_pawn());
	if (!hurt_pawn || !hurt_pawn->m_game_scene_node())
		return;

	if (!settings.cfg().visuals.m_killeffect)
		return;

	g_bullet_trace->spawn_particle_at_position(ParticlePosition(hurt_pawn, settings.cfg().visuals.i_killeffectpart), settings.cfg().visuals.i_killeffectpart);
}

void c_event::bullet_impact(c_game_event* event)
{
	if (!settings.cfg().worldModulation.b_bullet_trace)
		return;

	if (!sdk::m_local_pawn)
		return;

	auto player = event->get_event_helper().get_player_controller();
	if (!player)
		return;

	std::string_view token_name_x = ("x");
	CUtlStringToken1 tokenx(token_name_x.data());

	std::string_view token_name_y = ("y");
	CUtlStringToken1 tokeny(token_name_y.data());	

	std::string_view token_name_z = ("z");
	CUtlStringToken1 tokenz(token_name_z.data());
	auto position = vec3_t(event->GetFloatNew(tokenx), event->GetFloatNew(tokeny), event->GetFloatNew(tokenz));

	if (!player->m_is_local_player_controller())
	{
		if (player->get_player_pawn() && /*рср мсфем вей &&*/ player->get_player_pawn()->m_weapon_services())
			g_bullet_trace->add_bullet_trace(player->get_player_pawn()->get_eye_pos(), position, Color_t(255,0,0));
		return;
	}

	g_bullet_trace->add_bullet_trace(sdk::m_local_pawn->get_eye_pos(), position, settings.cfg().worldModulation.bullet_color);
}



void c_event::player_hurt(c_game_event* event)
{
	const auto attacker_controller = event->get_event_helper().get_attacker_controller();
	if (attacker_controller && attacker_controller == sdk::m_local_controller)
	{
		g_world->trigger_hitmarker();

		c_game_event_helper eventHelper = event->get_event_helper();

		auto pVictimController = eventHelper.get_player_controller();
		auto pAttackerController = eventHelper.get_attacker_controller();
		if (!pVictimController || !pAttackerController || !sdk::m_local_controller)
			return;

		c_player_pawn* pVictimPawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(pVictimController->handle_player_pawn().get_entry_index());
		if (!pVictimPawn)
			return;

		scene_hitbox_data hitbox_data_{};
		scene_hitbox_position hitbox_position_{};
        auto* node = pVictimPawn->m_game_scene_node();
        auto* skeleton = node ? node->get_skeleton_instance() : nullptr;
        auto* bones = skeleton ? skeleton->get_bone_cache() : nullptr;
		hitbox_data_.hitbox_count = hitbox_position_.hitbox_count = 19;

		for (int i{}; skeleton && bones && skeleton->m_model_state().modelHandle && i < 19; i++)
		{
			auto hitbox_data = pVictimPawn->m_game_scene_node()->get_skeleton_instance()->m_model_state().modelHandle->get_hitbox(i);
			if (!hitbox_data)
				continue;

			pVictimPawn->m_game_scene_node()->get_skeleton_instance()->calculate_world_space_bones(0x100);

			auto bone_index = pVictimPawn->get_bone_index(hitbox_data->m_bone_name);

			if (bone_index < 0 || bone_index >= skeleton->get_bone_count()) continue;
            auto bone_data = skeleton->m_model_state().bones[bone_index];

			hitbox_data_.hitbox_array[i].hitbox = hitbox_data;
			hitbox_position_.hitbox_array[i].position = bone_data.position;
			hitbox_position_.hitbox_array[i].scale = bone_data.scale;
			hitbox_position_.hitbox_array[i].rotate = bone_data.rotation;
		}

		hit_t hit_temp;
		hit_temp.time = g_interface->m_globalVars->flCurrentTime + 4.f;
		hit_temp.hitbox_data = hitbox_data_;
		hit_temp.hitbox_position = hitbox_position_;
		hit_temp.clr = Color_t{ 255, 0, 255, 255 };
		hit_capibara.push_back(hit_temp);

		vec3_t explosionPos = ParticlePosition(pVictimPawn, settings.cfg().visuals.i_hiteffectpart);

		if (settings.cfg().visuals.m_hiteffect)		{
			g_bullet_trace->spawn_particle_hit_at_position(explosionPos, settings.cfg().visuals.i_hiteffectpart);
		}

		int damage = eventHelper.get_damage();

		vec3_t pos;

		this->damage = damage;
		pos = pVictimPawn->get_scene_origin();

		const auto victim_controller = event->get_event_helper().get_attacker_controller();
		if (victim_controller && victim_controller != sdk::m_local_controller)
		{
			flHurtTime = g_interface->m_globalVars->flCurrentTime;

			c_player_pawn* victim_pawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(victim_controller->handle_player_pawn().get_entry_index());
			if (victim_pawn)
			{

				int damage = eventHelper.get_damage();

				auto scenenode = victim_pawn->m_game_scene_node();
				if (!scenenode)
					return;

				if (damage > 0)
				{

					vec3_t explosionPos = ParticlePosition(pVictimPawn, settings.cfg().visuals.i_hiteffectpart);

					if (!settings.cfg().visuals.m_hiteffect)
						return;

					g_bullet_trace->spawn_particle_hit_at_position(explosionPos, settings.cfg().visuals.i_hiteffectpart);
				}

			}
		}

		if (auto pVictimPawn = pVictimController->get_player_pawn())
		{
			vec3_t explosionPos = ParticlePosition(pVictimPawn, settings.cfg().visuals.i_hiteffectpart);

			//if (!g_cfg->world_esp.m_killeffect)
			//	return;

			//f_playereffects->kill_effect(explosionPos);
		}

	}
}


c_player_controller* c_game_event_helper::get_player_controller() { return get_controller("userid"); }
c_player_controller* c_game_event_helper::get_attacker_controller() { return get_controller("attacker"); }
c_player_controller* c_game_event_helper::get_controller(const char* name) {
    if (!Event || !name) return nullptr;
    Buffer key(name);
    return g_memory->CallVFunc<c_player_controller*>(Event, 16U, &key);
}
c_player_controller* c_game_event_helper::get_controller(const char* name, bool) { return get_controller(name); }
