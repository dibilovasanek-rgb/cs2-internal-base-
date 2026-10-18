#pragma once
#include "../../sdk/sdk.h"
#include "../../dependencies/config/Include/config.h"
#include "../../sdk/math/math.h"
#include <deque>
#include "../../sdk/math/matrix.h"
#include "../prediction/eng_pred.h"

class csgoinput;
class c_user_cmd;

struct lag_record_t {
	vec3_t m_origin{};
	c_player_pawn* m_pawn{};
	CSkeletonInstance* m_skeleton = nullptr;
	CBoneData m_bone_data[128];
	CBoneData m_bone_data_backup[128];
	float m_simulation_time{};
	vec3_t m_vec_mins{};
	vec3_t m_vec_maxs{};
	vec3_t m_vec_velocity{};

	bool m_throwing{};

	lag_record_t() {}

	inline lag_record_t(c_player_pawn* pawn) {
		this->store(pawn);
	}

	inline void store(c_player_pawn* pawn) {
		if (!pawn || !pawn->m_is_alive() || !pawn->is_player_pawn())
			return;

		auto scene_node = pawn->m_game_scene_node();
		if (!scene_node)
			return;

		auto skeleton_instance = pawn->m_game_scene_node()->get_skeleton_instance();
		if (!skeleton_instance || !skeleton_instance->get_bone_cache() || skeleton_instance->get_bone_count() <= 0)
			return;

		auto collision = pawn->m_p_collision();
		if (!collision)
			return;


		m_pawn = pawn;

		m_skeleton = skeleton_instance;
		m_origin = scene_node->m_vec_abs_origin();

		m_skeleton->calculate_world_space_bones(0x100);

		std::memcpy(m_bone_data, skeleton_instance->get_bone_cache(), sizeof(matrix2x4_t) * skeleton_instance->get_bone_count());

		m_simulation_time = pawn->m_simulation_time();
		m_vec_mins = collision->m_vec_mins();	
		m_vec_maxs = collision->m_vec_maxs();
		m_vec_velocity = pawn->m_vec_velocity();

		m_throwing = pawn->is_throwing();			
	}

	inline void apply(c_player_pawn* pawn) {	
		if (!pawn || !pawn->m_is_alive() || !pawn->is_player_pawn())
			return;

		auto scene_node = pawn->m_game_scene_node();
		if (!scene_node)
			return;
		CSkeletonInstance* skeleton_instance{};

		skeleton_instance = scene_node->get_skeleton_instance();
		if (!skeleton_instance || !skeleton_instance->get_bone_cache() || skeleton_instance->get_bone_count() <= 0)
			return;

		std::memcpy(m_bone_data_backup, skeleton_instance->get_bone_cache(), sizeof(matrix2x4_t) * skeleton_instance->get_bone_count());

		std::memcpy(skeleton_instance->get_bone_cache(), m_bone_data, sizeof(matrix2x4_t) * skeleton_instance->get_bone_count());
	}

	inline void reset(c_player_pawn* pawn) {
		if (!pawn || !pawn->m_is_alive() || !pawn->is_player_pawn())
			return;	

		auto scene_node = pawn->m_game_scene_node();
		if (!scene_node)	
			return;

		auto skeleton_instance = scene_node->get_skeleton_instance();
		if (!skeleton_instance || !skeleton_instance->get_bone_cache() || skeleton_instance->get_bone_count() <= 0)
			return;

		std::memcpy(skeleton_instance->get_bone_cache(), m_bone_data_backup, sizeof(matrix2x4_t) * skeleton_instance->get_bone_count());
	}
		
	bool is_valid() {
		float dead_time = (g_prediction->get_local_data()->m_tick_base * INTERVAL_PER_TICK) - m_simulation_time;

		return m_pawn && dead_time >= -INTERVAL_PER_TICK && dead_time < 0.2f;
	}
};	

struct aim_point_t {
	aim_point_t(vec3_t point, int hitbox, bool center = false) {
		this->m_point = point;
		this->m_hitbox = hitbox;
		this->m_center = center;
	}

	vec3_t m_point{};
	int m_hitbox{};
	bool m_center{};
};

struct aim_target_t {
	lag_record_t* m_lag_record{};
	std::shared_ptr<aim_point_t> m_best_point;
	c_player_pawn* m_pawn{ nullptr };

	aim_target_t() = default;

	inline aim_target_t(lag_record_t* record) {
		this->store(record);
	}
		
	inline void store(lag_record_t* record) {
		if (!record || !record->m_pawn || !record->m_pawn->is_player_pawn())
			return;

		m_lag_record = record;
		m_pawn = record->m_pawn;
	}

	inline void reset() {
		if (this == nullptr || (uintptr_t)this == 0xFFFFFFFFFFFFFFF7)
			return;
	
		m_lag_record = nullptr;
		m_pawn = nullptr;
		if (m_best_point)
		    m_best_point.reset();
	}
};

struct c_points
{
	vec3_t m_vecPosition;
	bool m_bCenter;
};


struct hitbox_data_t
{
	hitbox_data_t() = default;

	int m_num_hitbox;
	int m_num_bone;
	const char* m_name_bone;
	int m_num_hitgroup;	
	c_hitbox* m_hitbox_data;
	float m_radius;
	matrix3x4_t m_matrix;
	vec3_t m_mins;
	vec3_t m_maxs;
	bool m_invalid_data{ true };
};

class ragebot {
private:
	typedef void(__fastcall* FnCalcSpread)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*);
	inline static FnCalcSpread fnCalcSpread;
	int next_cock_time = 0;
	float best_damage = 0.0f;

public:
	std::vector<int> m_hitboxes;
	std::unordered_map<int, std::deque<lag_record_t>> m_lag_records{};
	std::unordered_map<int, std::shared_ptr<aim_target_t>> m_aim_targets;
	std::shared_ptr<aim_target_t> m_best_target;

	void add_point(vec3_t pos,int bone, bool center, std::vector<aim_point_t>& point)
	{
		point.emplace_back(aim_point_t(pos, bone, center));
	}
	hitbox_data_t get_hitbox_data(c_player_pawn* player_pawn, const int hitbox_id) {
		if (!player_pawn)
			return hitbox_data_t();

		auto game_scene = player_pawn->m_game_scene_node();
		if (!game_scene)
			return hitbox_data_t();

		auto skeleton = game_scene->get_skeleton_instance();
		if (!skeleton)
			return hitbox_data_t();

		auto& model_state = skeleton->m_model_state();
		auto& model = model_state.modelHandle;
		if (!model)
			return hitbox_data_t();

		auto hitbox_data = model->get_hitbox(hitbox_id);
		if (!hitbox_data)
			return hitbox_data_t();

		const auto bone_index = player_pawn->get_bone_index(hitbox_data->m_bone_name);
		const auto bones = model_state.bones;
        if (!bones || bone_index < 0 || bone_index >= skeleton->get_bone_count()) return hitbox_data_t();

		hitbox_data_t hitbox;
		hitbox.m_num_hitbox = hitbox_id;
		hitbox.m_num_bone = bone_index;
		hitbox.m_hitbox_data = hitbox_data;
		hitbox.m_radius = hitbox_data->m_shape_radius;
		hitbox.m_num_hitgroup = hitbox_data->m_hitgroup;
		hitbox.m_matrix = math::transform_to_matrix(bones[bone_index]);
		hitbox.m_mins = hitbox_data->m_vec_min;
		hitbox.m_maxs = hitbox_data->m_vec_maxs;
		hitbox.m_name_bone = hitbox_data->m_name;
		hitbox.m_invalid_data = false;

		return hitbox;
	}

	inline lag_record_t* get_last_record(int handle) {
		if (m_lag_records.find(handle) == m_lag_records.end())
			return nullptr;

		auto& records = m_lag_records[handle];

		if (records.empty())
			return nullptr;

		auto record = &records.back();
		if (record->is_valid())
			return nullptr;

		return record;
	}

	vec3_t calculate_spread_internal(int seed, float accuracy, float spread, float recoil_index, int item_def_idx);
	bool can_shoot(c_base_player_weapon* active_weapon);
	int no_spread(vec3_t& pos, c_weapon_base* weapon, c_player_pawn* Target);
	int hitchance(vec3_t& pos, c_weapon_base* weapon, c_player_pawn* Target);
	void autostop(int mode);
	void process_backtrack(lag_record_t* record);
	void process_attacka(c_user_cmd* user_cmd);
	vec3_t calculate_spread_angles(vec3_t angle, int random_seed, float weapon_inaccuarcy, float weapon_spread);
	void store_records(int stage);
	lag_record_t* select_record(int handle);
	void find_targets();
	std::shared_ptr<aim_target_t> get_nearest_target();
	vec3_t transform_point(matrix3x4_t matrix, vec3_t point);
	DWORD hitboxToBone(c_hitbox* hitbox);
	int get_hitbox_from_menu(int hitbox);
	void double_tap();
	std::vector<vec3_t> calculate_sphere_points(float radius, int num_points);
	std::vector<vec3_t> calculate_points(int num_points, float radius, float height, matrix3x4_t matrix, hitbox_data_t hitbox);
	float calc_point_scale(vec3_t& point, const float& hitbox_radius);
	bool multi_points(lag_record_t* record, int hitbox, std::vector<aim_point_t>& points);
	aim_point_t select_points(lag_record_t* record, float& damage);
	bool multipoints(c_player_pawn* pawn, CBoneData* bones, int index, std::vector<aim_point_t>& points, bool predicted);
	void debug(c_player_pawn* pawn);
	void select_target();
	void revolver_cock(c_user_cmd* cmd);
	void store_hitboxes();
	bool valid_interpolation_timing2();
	void run(csgoinput* pInput, c_user_cmd* pCmd);
};
inline std::unique_ptr<ragebot> g_ragebot = std::make_unique<ragebot>();
																		