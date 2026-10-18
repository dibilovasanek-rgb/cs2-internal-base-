#include "protobufs.h"
#include "../../memory/native_rtti.h"

c_subtick_move_step* c_protobuf::add_subtick_move_step(c_user_cmd* user_cmd) {
	if (!user_cmd || !user_cmd->pb.has_base()) return nullptr;
	auto rept_field_move_steps = reinterpret_cast<google::protobuf::repeated_ptr_field_t<c_subtick_move_step>*>((PBYTE)user_cmd->pb.mutable_base() + 0x18);

	if (rept_field_move_steps->m_rep && rept_field_move_steps->m_current_size < rept_field_move_steps->m_rep->m_allocated_size)
		subtick_move_step = rept_field_move_steps->m_rep->m_elements[rept_field_move_steps->m_current_size++];
	else
		subtick_move_step = this->create_new_subtick_move_step(rept_field_move_steps, rept_field_move_steps->m_arena);

	return subtick_move_step;
}

c_csgo_input_history_entry_pb* c_protobuf::add_input_history(c_user_cmd* user_cmd) {
	if (!user_cmd) return nullptr;
	auto rept_field_input_history = reinterpret_cast<google::protobuf::repeated_ptr_field_t<c_csgo_input_history_entry_pb>*>(reinterpret_cast<PBYTE>(&user_cmd->pb) + 0x18);

	if (rept_field_input_history->m_rep && rept_field_input_history->m_current_size < rept_field_input_history->m_rep->m_allocated_size)
		input_history_entry = rept_field_input_history->m_rep->m_elements[rept_field_input_history->m_current_size++];
	else
		input_history_entry = this->create_new_input_history(rept_field_input_history, rept_field_input_history->m_arena);

	return input_history_entry;
}

CSGOInterpolationInfoPB* c_protobuf::create_new_interps(CSGOInterpolationInfoPB* message) {
    using create_t = CSGOInterpolationInfoPB*(__fastcall*)(void*, void*);
    static auto** table = NativeRtti::FindVTable(GetModuleHandleW(CLIENT_DLL), ".?AVCSGOInterpolationInfoPB@@");
    return message && table ? reinterpret_cast<create_t>(table[2])(nullptr, message->GetArena()) : nullptr;
}
c_csgo_input_history_entry_pb* c_protobuf::create_new_input_history(google::protobuf::repeated_ptr_field_t<c_csgo_input_history_entry_pb>* field, void* arena) {
    using create_t = c_csgo_input_history_entry_pb*(__fastcall*)(void*, void*);
    using append_t = c_csgo_input_history_entry_pb*(__fastcall*)(void*, void*);
    static auto** table = NativeRtti::FindVTable(GetModuleHandleW(CLIENT_DLL), ".?AVCSGOInputHistoryEntryPB@@");
    static auto append = reinterpret_cast<append_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B FA 48 8B 49 10 48 85 C9 74 ? 8B 01 3B 43 0C 75 ? BA 01 00 00 00 48 8B CB E8 43 01"));
    if (!table || !append || !field || field->m_current_size < 0 || field->m_current_size >= 4096) return nullptr;
    auto* message = reinterpret_cast<create_t>(table[2])(nullptr, arena);
    return message ? append(field, message) : nullptr;
}

c_subtick_move_step* c_protobuf::create_new_subtick_move_step(google::protobuf::repeated_ptr_field_t<c_subtick_move_step>* rept_ptr, void* arena) {
    using create_t=c_subtick_move_step*(__fastcall*)(void*,void*);
    static auto** table=NativeRtti::FindVTable(GetModuleHandleW(CLIENT_DLL),".?AVCSubtickMoveStep@@");
    using append_t=c_subtick_move_step*(__fastcall*)(google::protobuf::repeated_ptr_field_t<c_subtick_move_step>*,void*);
    static auto append=reinterpret_cast<append_t>(g_memory->find_pattern(CLIENT_DLL,"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B FA 48 8B 49 10 48 85 C9 74 ? 8B 01 3B 43 0C 75 ? BA 01 00 00 00 48 8B CB E8 43 01"));
    if (!table || !append || !rept_ptr || rept_ptr->m_current_size < 0 || rept_ptr->m_current_size >= 4096) return nullptr;
    auto* step=reinterpret_cast<create_t>(table[2])(nullptr,arena);
    return step ? append(rept_ptr,step) : nullptr;
}