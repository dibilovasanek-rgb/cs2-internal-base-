#pragma once
#include "usercmd.pb.h"
#include "cs_usercmd.pb.h"
#include "../game_class/usercmd.h"
class c_user_cmd;
class c_protobuf {
	c_subtick_move_step* subtick_move_step = nullptr;
	c_csgo_input_history_entry_pb* input_history_entry = nullptr;

	c_subtick_move_step* create_new_subtick_move_step(google::protobuf::repeated_ptr_field_t<c_subtick_move_step>* rept_ptr, void* arena);
	c_csgo_input_history_entry_pb* create_new_input_history(google::protobuf::repeated_ptr_field_t<c_csgo_input_history_entry_pb>* rept_ptr, void* arena);
public:
	c_subtick_move_step* add_subtick_move_step(c_user_cmd* user_cmd);
	c_csgo_input_history_entry_pb* add_input_history(c_user_cmd* user_cmd);
	CSGOInterpolationInfoPB* create_new_interps(CSGOInterpolationInfoPB* mutable_interpolation_info);
};

inline const auto g_protobuf = std::make_unique<c_protobuf>();