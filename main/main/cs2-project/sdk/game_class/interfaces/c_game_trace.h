#pragma once
#include "../../math/vector.h"
#include "../../../memory/memory.h"
#include <array>

struct Ray_t
{
public:
	vec3_t m_vecStart;
	vec3_t m_vecEnd;
	vec3_t m_vecMins;
	vec3_t m_vecMaxs;
	MEM_PAD(0x4);
	std::uint8_t UnkType;
};
static_assert(sizeof(Ray_t) == 0x38);

class c_player_pawn;
struct TraceFilter_t
{
public:
	void* table{};
	std::int64_t m_uTraceMask;//0x8
	std::array<std::int64_t, 2> m_v1;//
	std::array<std::int32_t, 4> m_arrSkipHandles;//
	std::array<std::int16_t, 2> m_arrCollisions;// 0x20
	char pad34[3]{};
    std::uint8_t m_v2;//0x24
	std::uint8_t m_v3;//
	std::uint8_t m_v4;//
char pad3a[6]{};
    char collision1{};
    uint16_t collision2{};

	TraceFilter_t() = default;
	TraceFilter_t(std::uint64_t uMask, c_player_pawn* pSkip1, c_player_pawn* pSkip2, int nLayer);
};
static_assert(sizeof(TraceFilter_t) == 0x48);

struct TraceHitboxData_t
{
public:
	MEM_PAD(0x38);
	int m_nHitGroup;
	MEM_PAD(0x4);
	char shapeData[8];
    uint16_t m_nHitboxId;
    char tail[0x26];
};
static_assert(sizeof(TraceHitboxData_t) == 0x70);

class game_trace_t
{
public:
	void* Surface;
	c_player_pawn* HitEntity;
	TraceHitboxData_t* HitboxData;
	MEM_PAD(0x38);
	std::uint32_t Contents;
	MEM_PAD(0x24);
	vec3_t m_start_pos, m_end_pos, m_normal, m_pos;
	MEM_PAD(0x4);
	float Fraction;
	MEM_PAD(0x6);
	bool m_all_solid;
	MEM_PAD(0x4D);
	bool IsVisible() const;
	//bool DidHitWorld() const;
};

struct trace_arr_element_t { std::byte data[0x38]{}; };
struct update_value_t {
    float previousLenght{},currentLenght{},damage{};
    uint32_t penetrationCount{},handleIdx{},secondHandle{};
};
struct trace_data_t {
    int32_t uk1{};float uk2{52.f};void* arr_pointer{};
    int32_t uk3{128},uk4{static_cast<int32_t>(0x80000000)};
    std::array<trace_arr_element_t,128> arr{};
    void* reserved{};int32_t num_update{},surfaceReserved{};
    update_value_t* pointer_update_value{};
    int32_t hitCount{8};uint32_t hitFlags{0x80000000};
    std::byte hitElements[0xc0]{};
    vec3_t start{},end{};float fraction{1.f};bool finished{};std::byte tail[0x4b]{};
};
static_assert(sizeof(trace_data_t)==0x1d60);
static_assert(offsetof(trace_data_t,num_update)==0x1c20);

class c_game_trace
{
private:
	using TraceShapes = bool(__fastcall*)(c_game_trace*, Ray_t*, vec3_t*, vec3_t*, TraceFilter_t*, game_trace_t*);
	inline static TraceShapes fnTraceShape;
	using function_t = void(__fastcall*)(vec3_t&, vec3_t&, TraceFilter_t*, game_trace_t*, float, int, float);
	inline static function_t fnClip;
	using fn_handle_bullet_penetration = bool(__fastcall*)(trace_data_t*,void*,update_value_t*,int,int64_t);
	inline static fn_handle_bullet_penetration fnhandle_bullet;
	using fn_init_trace_info = void(__fastcall*) (game_trace_t*);
	inline static fn_init_trace_info fnini_trace_info;
	using fn_get_trace_info = void(__fastcall*)(trace_data_t*,game_trace_t*,void*);
	inline static fn_get_trace_info fnGet_trace_info;
	using fn_create_trace_t = void(__fastcall*)(trace_data_t*,const vec3_t&,const vec3_t&,const TraceFilter_t&,int,bool);
	inline static fn_create_trace_t fnCreate_trace;
public:
	bool hookTrace()
	{
		fnTraceShape = reinterpret_cast<TraceShapes>(g_memory->find_pattern(CLIENT_DLL, "48 89 54 24 10 48 89 4C 24 08 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? B8 10 25 00 00"));
		fnhandle_bullet = reinterpret_cast<fn_handle_bullet_penetration>(g_memory->find_pattern(CLIENT_DLL, "48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48"));
		fnini_trace_info = reinterpret_cast<fn_init_trace_info>(g_memory->find_pattern(CLIENT_DLL, "40 55 41 55 41 57 48 83 EC"));
		fnGet_trace_info = reinterpret_cast<fn_get_trace_info>(g_memory->find_pattern(CLIENT_DLL, "41 0F B7 40 ? F3 41 0F 10 10"));
		fnCreate_trace = reinterpret_cast<fn_create_trace_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 F2 0F 10 02 4D 8D 71 08"));

		return 1;
	}

	bool handle_bullet_penetration(trace_data_t* const trace, void* stats, update_value_t* const mod_value, const int draw_showimpacts)
	{
		if (!fnhandle_bullet)
			fnhandle_bullet = reinterpret_cast<fn_handle_bullet_penetration>(g_memory->find_pattern(CLIENT_DLL, "48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48"));

		return fnhandle_bullet && fnhandle_bullet(trace,stats,mod_value,draw_showimpacts,0);
	}

	void create_trace(trace_data_t* const trace, vec3_t start, vec3_t end, const TraceFilter_t& filter, int penetration_count) {
		if (!fnCreate_trace)
			fnCreate_trace = reinterpret_cast<fn_create_trace_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 F2 0F 10 02 4D 8D 71 08"));
		
		if (!fnCreate_trace || !filter.table) return;
        trace->arr_pointer=trace->arr.data();trace->pointer_update_value=reinterpret_cast<update_value_t*>(trace->hitElements);
        fnCreate_trace(trace,start,end,filter,penetration_count,true);
	}

	bool trace_shape(Ray_t* pRay, vec3_t vecStart, vec3_t vecEnd, TraceFilter_t* pFilter, game_trace_t* pGameTrace)
	{
		if (!fnTraceShape)
			fnTraceShape = reinterpret_cast<TraceShapes>(g_memory->find_pattern(CLIENT_DLL, "48 89 54 24 10 48 89 4C 24 08 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? B8 10 25 00 00"));

		return fnTraceShape && fnTraceShape(this, pRay, &vecStart, &vecEnd, pFilter, pGameTrace);
	}

	bool trace_shape2(Ray_t* ray, vec3_t start, vec3_t end, TraceFilter_t* filter, game_trace_t* game_trace)
	{
return trace_shape(ray,start,end,filter,game_trace);
	}


	bool clip_ray_entity(Ray_t* ray, vec3_t start, vec3_t end, c_player_pawn* pawn, TraceFilter_t* filter, game_trace_t* game_trace)
	{

		return pawn && trace_shape(ray,start,end,filter,game_trace) && game_trace->HitEntity==pawn;
	}

	void init_trace(TraceFilter_t& filter, c_player_pawn* pawn, uint64_t mask, uint8_t layer, uint16_t unknown) {
		using fn_init_trace_t = TraceFilter_t * (__fastcall*)(TraceFilter_t&, void*, uint64_t, uint8_t, uint16_t);
		static fn_init_trace_t fn = reinterpret_cast<fn_init_trace_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24"));
			
		filter=TraceFilter_t(mask,pawn,nullptr,layer);
	}

	void init_trace_info(game_trace_t* const hit) {
		if (fnini_trace_info == nullptr)
			fnini_trace_info = reinterpret_cast<fn_init_trace_info>(g_memory->find_pattern(CLIENT_DLL, "40 55 41 55 41 57 48 83 EC"));
		
		if(fnini_trace_info) fnini_trace_info(hit);
	}
	void get_trace_info(trace_data_t* const trace, game_trace_t* const hit, const float unknown_float, void* unknown) {
		if (!fnGet_trace_info)
			fnGet_trace_info = reinterpret_cast<fn_get_trace_info>(g_memory->find_pattern(CLIENT_DLL, "41 0F B7 40 ? F3 41 0F 10 10"));
		
	    if(fnGet_trace_info) fnGet_trace_info(trace,hit,unknown);
	}
	//void init_trace(TraceFilter_t& filter, c_player_pawn* pawn, uint64_t mask, uint8_t layer, uint16_t unknown) {
	//	using fn_init_trace_t = TraceFilter_t * (__fastcall*)(TraceFilter_t&, void*, uint64_t, uint8_t, uint16_t);
	//	static fn_init_trace_t fn = reinterpret_cast<fn_init_trace_t>(g_memory->find_pattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24"));

	//	filter=TraceFilter_t(mask,pawn,nullptr,layer);
	//}
};
