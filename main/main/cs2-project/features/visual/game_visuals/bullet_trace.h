#pragma once
#include <mutex>
#include "../../../sdk/game_class/strong_handle.h"
#include "../../../sdk/game_class/color.h"
#include "../../../sdk/math/vector.h"

class particle_effect
{
public:
	const char* name{};
	char pad[0x30]{};
};

class particle_information
{
public:
	float time{};
	float width{};
	float unk2{};
};

class particle_data
{
public:
	vec3_t* positions{};
	char pad[0x74]{};
	float* times{};
	char pad2[0x28];
	float* times2{};
	char pad3[0x98];
};

struct particle_color
{
	float r;
	float g;
	float b;
};

class c_particle_snapshot {
public:
	void draw(int count, void* data);
};

class c_game_particle_manager_system
{
public:
	void create_effect_index(unsigned int* effect_index, particle_effect* effect_data);
	void create_effect2(unsigned int* pEffectIndex, const char* szName);
	void SetEffect(unsigned int effect_index, int unk, void* clr, int unk2);
	void fnInitEffect(int effect_index, unsigned int unk, const c_strong_handle<c_particle_snapshot>* particle_snapshot);
};

class c_particle_manager
{
public:
	void create_snapshot(c_strong_handle<c_particle_snapshot>* out_particle_snapshot);
	void draw(c_strong_handle<c_particle_snapshot>* particle_snapshot, int count, void* data);
};
class tracer_info
{
public:
	unsigned int effect_index = -1;
	vec3_t* positions = nullptr;
	float* times = nullptr;
	c_strong_handle<c_particle_snapshot> handle_snapshot_particle{};
	particle_data particle_data_;
};

class cbullet_tracer
{
public:
	void spawn_particle_at_position(const vec3_t& position, int ip);
	void spawn_particle_hit_at_position(const vec3_t& position, int ip);
	void spawn_particle_map_at_position(int ip, Color_t clr_);
	void add_bullet_trace(vec3_t start, vec3_t end, Color_t clr_);
private:
	class cbullet_tracer_info
	{
	public:
		cbullet_tracer_info(vec3_t src, vec3_t dst, float time, Color_t color)
		{
			this->src = src;
			this->dst = dst;
			this->time = time;
			this->color = color;
		}

		vec3_t src, dst;
		float time;
		Color_t color;
	};
};
inline std::unique_ptr<cbullet_tracer> g_bullet_trace = std::make_unique<cbullet_tracer>();

