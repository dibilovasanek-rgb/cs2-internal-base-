#include "c_game_trace.h"
#include "../../schema/class/player.h"

TraceFilter_t::TraceFilter_t(std::uint64_t mask,c_player_pawn* first,c_player_pawn* second,int layer) {
    std::memset(this,0,sizeof(*this));
    using fn_t=void(__fastcall*)(TraceFilter_t*,void*,uint64_t,int);
    static auto fn=reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL,"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24"));
    if(fn) fn(this,first ? first : second,mask,layer);
    m_v2=7;
}
bool game_trace_t::IsVisible() const
{
	return (Fraction > 0.97f);
}