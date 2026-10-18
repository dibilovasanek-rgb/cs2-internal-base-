#pragma once
#include "../../../dependencies/imgui/imgui.h"
#include "../../../sdk/game_class/color.h"
#include <chrono>

class c_postproccesing;
class c_player_pawn;
class C_SceneLightObject;
class c_envsky;

class c_world
{
private:
	inline static std::chrono::steady_clock::time_point last_hit_time;
	inline static bool hitmarker_visible;

public:

    void update_walls(void* aggregate);
    void refresh_walls();
    void reset_walls();
	void skybox();
	void skybox(c_envsky* env_sky);
	void lighting(C_SceneLightObject* light_object);
	void exposure(c_player_pawn* pawn);
	void exposure(c_postproccesing* post_processing);
	void run_onshot();
	void DrawScopeOverlay(ImDrawList* draw);
	void DrawBomb(ImDrawList* draw);
	void trigger_hitmarker();
	void hitmarker(Color_t clr_);
	void penitrations(ImDrawList* draw);
	void active_grande(ImDrawList* draw);
	void test(ImDrawList* draw);
	void weapon_esp();

	static c_world& get_instance() {
		static c_world instance;
		return instance;
	}
};
inline c_world* g_world = &c_world::get_instance();
