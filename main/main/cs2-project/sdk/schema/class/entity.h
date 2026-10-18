#pragma once
#include <cstdint>
#include "../schema.h" 
#include "../../game_class/basehandle.h"
#include "../../game_class/interfaces/ischemasystem.h"
#include "../../../memory/memory.h"
#include "../../../core/interfaces/interface.h"
#include "../../../sdk/math/vector.h"
#include "../../../sdk/game_class/color.h"
#include "../../game_class/c_model.h"
#include "../../game_class/class_info.h"

#define GET_CLASS_INFO(var_name, dll_name, class_name)                                                                \
    static c_schema_system_type_scope* type_scope = g_interface->schema_system->find_type_scope_for_module(dll_name);     \
    if (!type_scope) {                                                                                                \
        printf("Failed to get type scope: %s", dll_name);                                                             \
                                                                                                                      \
        return false;                                                                                                 \
    }                                                                                                                 \
                                                                                                                      \
    static c_schema_class_info_data* var_name = type_scope->FindDeclaredClass(class_name);                               \
    if (!class_info) {                                                                                                \
        printf("Failed to get class info: %s", class_name);                                                           \
                                                                                                                      \
        return false;                                                                                                 \
    }

class CSkeletonInstance;    
class c_entity
{
private:
	using fnSetModel = void* (__fastcall*)(void*, const char*);
	using fnSetMeshGroup = void(__thiscall*)(void*, uint64_t);
    using fnGetupdate = void(__fastcall*)(void*);
    using fncalc_world_sppace = void(__fastcall*)(CSkeletonInstance*, uint32_t);

public:
    inline static fnSetModel SetModel;
    inline static fnSetMeshGroup setmeshgroup;
    inline static fnGetupdate get_update;
    inline static fncalc_world_sppace calc_world_space;

	static void setup()
	{
        SetModel = reinterpret_cast<fnSetModel>(g_memory->find_pattern(CLIENT_DLL, "40 53 48 83 EC ?? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24"));

        get_update = reinterpret_cast<fnGetupdate>(g_memory->find_pattern(CLIENT_DLL, ("4C 8B DC 53 48 81 EC ? ? ? ? 48 8B 41")));
        calc_world_space = reinterpret_cast<fncalc_world_sppace>(g_memory->find_pattern(CLIENT_DLL, "48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81"));
	}
};


class c_entity_identity
{
public:
    OFFSET(std::uint32_t, m_get_index, 0x10);

    [[nodiscard]] bool IsValid()
    {
        return m_get_index() != INVALID_EHANDLE_INDEX;
    }

    [[nodiscard]] int GetEntryIndex()
    {
        if (!IsValid())
            return ENT_ENTRY_MASK;

        return m_get_index() & ENT_ENTRY_MASK;
    }

    [[nodiscard]] int GetSerialNumber()
    {
        return m_get_index() >> NUM_SERIAL_NUM_SHIFT_BITS;
    }
    SCHEMA(m_designer_name, const char*, ("CEntityIdentity"), ("m_designerName"));
    SCHEMA(m_flags, std::uint32_t, ("CEntityIdentity"), ("m_flags"));
};

class c_entity_instance
{
public:
    SCHEMA(m_entity, c_entity_identity*, ("CEntityInstance"), ("m_pEntity"));

    //vremenno bydet tak
    
    c_schema_class_info_data* GetSchemaClassInfo()
    {
        auto* identity=reinterpret_cast<std::byte*>(m_entity());
        auto* binding=identity ? *reinterpret_cast<std::byte**>(identity+8) : nullptr;
        return binding ? *reinterpret_cast<c_schema_class_info_data**>(binding+0x58) : nullptr;
    }   

        
	const char* get_entity_class_name()
    {
        c_schema_class_info_data* class_info = this->GetSchemaClassInfo();

        return class_info && class_info->name ? class_info->name : "";
    }

    c_base_handle get_ref_ehandle()
    {
        c_base_handle handle{};
        c_entity_identity* identity = m_entity();
        if (!identity)
            return handle;

        return c_base_handle(identity->GetEntryIndex(), identity->GetSerialNumber() - (identity->m_flags() & 1));

    }
};

class matrix2x4_t;

class CSkeletonInstance
{
public:
    matrix2x4_t* get_bone_cache() {
        const auto offset=g_schema->schema_get_offset("CSkeletonInstance","m_modelState");
        return offset ? *reinterpret_cast<matrix2x4_t**>(reinterpret_cast<std::byte*>(this)+offset+0x80) : nullptr;
    }
    int get_bone_count() {
        const auto offset=g_schema->schema_get_offset("CSkeletonInstance","m_modelState");
        auto* handle=offset ? *reinterpret_cast<std::byte**>(reinterpret_cast<std::byte*>(this)+offset+0xa0) : nullptr;
        auto* model=handle ? *reinterpret_cast<std::byte**>(handle) : nullptr;
        return model ? std::clamp(*reinterpret_cast<int*>(model+0x178),0,128) : 0;
    }
    SCHEMA(m_model_state, c_model_state, ("CSkeletonInstance"), ("m_modelState"));

    void calculate_world_space_bones(uint32_t flags)
    {
        if (!c_entity::calc_world_space)
        {
            std::cout << "fail calc_world_space" << std::endl;
            return;
        }
        c_entity::calc_world_space(this, flags);
    }
};

class c_game_scene_node
{
public:
    SCHEMA(m_vec_abs_origin, vec3_t, ("CGameSceneNode"), ("m_vecAbsOrigin"));
    SCHEMA(m_vec_origin, vec3_t, ("CGameSceneNode"), ("m_vecOrigin"));
    SCHEMA(m_ang_rotation, qangle_t, ("CGameSceneNode"), ("m_angRotation"));
    SCHEMA(m_ang_abs_rotation, qangle_t, ("CGameSceneNode"), ("m_angAbsRotation"));
    SCHEMA(m_dormant, bool, ("CGameSceneNode"), ("m_bDormant"));


    __forceinline CSkeletonInstance* get_skeleton_instance() {
        return reinterpret_cast<CSkeletonInstance*>(this);
    }

    void set_mesh_group_mask(uint64_t mask) {
        auto* skeleton = get_skeleton_instance();
        const auto offset = g_schema->schema_get_offset("CModelState", "m_MeshGroupMask");
        if (skeleton && offset) *reinterpret_cast<uint64_t*>(reinterpret_cast<std::byte*>(&skeleton->m_model_state()) + offset) = mask;
    }

};

class c_collision_property
{
public:
    SCHEMA(m_vec_mins, vec3_t, ("CCollisionProperty"), ("m_vecMins"));
    SCHEMA(m_vec_maxs, vec3_t, ("CCollisionProperty"), ("m_vecMaxs"));
    std::uint16_t CollisionMask()
    {
        return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
    }
    SCHEMA(GetSolidFlags, std::uint8_t, "CCollisionProperty","m_usSolidFlags");

};

class c_animation_graph_instance
{
public:
    void set_networked_variables(void* ptr) noexcept
    {
        *reinterpret_cast<void**>(std::uintptr_t(this) + 0x2E0) = ptr;
    }
};

class c_base_anim_graph_controller
{
public:
    c_animation_graph_instance* get_anim_graph()
    {
        return *(c_animation_graph_instance**)(std::uintptr_t(this) + 0x16F0);
    }
};

//class c_body_component : public c_entity_instance
//{
//public:
//    c_game_scene_node* m_game_scene_node;
//    unsigned char pad_10[0x40];
//
//    c_base_anim_graph_controller* get_base_anim_graph_controller()
//    {
//        return reinterpret_cast<c_base_anim_graph_controller*>(std::uintptr_t(this) + 0x460);
//    }
//};

static vec3_t vecEmpty = vec3_t(0, 0, 0);

class c_base_entity : public c_entity_instance
{
public:
    SCHEMA(m_team_num, uint8_t, ("C_BaseEntity"), ("m_iTeamNum"));
    SCHEMA(m_fflags, int, ("C_BaseEntity"), ("m_fFlags"));
    SCHEMA(m_health, int, ("C_BaseEntity"), ("m_iHealth"));
    SCHEMA(m_max_health, int, ("C_BaseEntity"), ("m_iMaxHealth"));
    SCHEMA(m_life_state, uint8_t, ("C_BaseEntity"), ("m_lifeState"));
    SCHEMA(m_move_type, uint8_t, ("C_BaseEntity"), ("m_MoveType"));
    SCHEMA(m_vec_velocity, vec3_t, ("C_BaseEntity"), ("m_vecVelocity"));
    SCHEMA(m_vec_abs_velocity, vec3_t, ("C_BaseEntity"), ("m_vecAbsVelocity"));
    SCHEMA(m_game_scene_node, c_game_scene_node*, ("C_BaseEntity"), ("m_pGameSceneNode"));
    SCHEMA(m_p_collision, c_collision_property*, ("C_BaseEntity"), ("m_pCollision"));
    SCHEMA(m_sub_class_id, uint32_t, ("C_BaseEntity"), ("m_nSubclassID"));
    SCHEMA(get_owner_handle, c_base_handle, "C_BaseEntity","m_hOwnerEntity");
    SCHEMA(get_water_level, float, "C_BaseEntity","m_flWaterLevel");
    SCHEMA(m_simulation_time, float, ("C_BaseEntity"), ("m_flSimulationTime"));

    bool find_class(uint64_t hash) {
        auto visit=[&](auto&& self,c_schema_class_info_data* info,int depth)->bool {
            if(!info || depth>16) return false;
            if(info->name && fnv1a::hash(info->name)==hash) return true;
            auto* raw=reinterpret_cast<std::byte*>(info);
            struct Base { int32_t offset; c_schema_class_info_data* info; };
            const int count=*reinterpret_cast<uint8_t*>(raw+0x2b);
            auto* bases=*reinterpret_cast<Base**>(raw+0x40);
            if (bases && count<=16) for(int i=0;i<count;++i) if(self(self,bases[i].info,depth+1)) return true;
            return false;
        };
        return visit(visit,GetSchemaClassInfo(),0);
    }

    c_unkown_information* get_class_information() {
        if ((uintptr_t)this == 0xdddddddddddddddd)
            return {};
        return *(c_unkown_information**)(std::uintptr_t(this) + 0x10);
    }

    bool is_player_pawn() {
        //GET_CLASS_INFO(class_info, "client.dll", "C_CSPlayerPawn");
        return find_class(fnv1a::hash("C_CSPlayerPawn"))/*GetSchemaClassInfo() == class_info*/;
    }

    bool is_controller() {
        return find_class(fnv1a::hash("CCSPlayerController"));
    }

    void UpdateSubClass();

    void UpdateVData()
    {
        g_memory->CallVFunc<void*>(this, 182);
    }

    bool is_weapon() {
        return g_memory->CallVFunc<bool>(this, 158);
    }

    SCHEMA(m_nSubclassID, std::uint32_t,"C_BaseEntity","m_nSubclassID");

    //SCHEMA(m_body_component, c_body_component*, ("C_BaseEntity"), ("m_CBodyComponent"));
    void* GetVData() {
        if (this)
           return *reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(this)+g_schema->schema_get_offset("C_BaseEntity","m_nSubclassID")+8); // subclass
    }

    void __fastcall set_model(const char* model_name);

	bool m_is_alive() {
        try {   
            if (this && (uintptr_t)this->m_life_state() != 0xFFFFFFFFFFFFFFFF && (uintptr_t)this->m_life_state() != 0xddddddddddddddd)
            {
                return this->m_life_state() != 2 && this->m_health() > 0; //2 =  ELifeState::LIFE_DEAD
            }
            else
                return false;
        }
        catch (...)
        {
            std::cout << "error is_alive" << std::endl;
        }
	}

    const vec3_t& get_scene_origin()
    {
        if (this->m_game_scene_node())
            return m_game_scene_node()->m_vec_abs_origin();

        return vecEmpty;
    }
};

class c_planted_c4 : public c_base_entity {
public:
    SCHEMA(get_bomb_site, int, ("C_PlantedC4"), ("m_nBombSite"));
    SCHEMA(is_c4_activated, bool, ("C_PlantedC4"), ("m_bC4Activated"));
    SCHEMA(if_bomb_defused, bool, ("C_PlantedC4"), ("m_bBombDefused"));
    SCHEMA(get_c4_pos, vec3_t, ("C_PlantedC4"), ("m_vecC4ExplodeSpectatePos"));
};

class c_envsky
{
public:
    SCHEMA(m_bStartDisabled, bool, ("C_EnvSky"), ("m_bStartDisabled"));
    SCHEMA(m_vTintColor, ByteColor, ("C_EnvSky"), ("m_vTintColor"));
    SCHEMA(m_vTintColorLightingOnly, Color_t, ("C_EnvSky"), ("m_vTintColorLightingOnly"));
    SCHEMA(m_flBrightnessScale, float, ("C_EnvSky"), ("m_flBrightnessScale"));
    SCHEMA(m_nFogType, int, ("C_EnvSky"), ("m_nFogType"));
    SCHEMA(m_flFogMinStart, float, ("C_EnvSky"), ("m_flFogMinStart"));
    SCHEMA(m_flFogMinEnd, float, ("C_EnvSky"), ("m_flFogMinEnd"));
    SCHEMA(m_flFogMaxStart, float, ("C_EnvSky"), ("m_flFogMaxStart"));
    SCHEMA(m_flFogMaxEnd, float, ("C_EnvSky"), ("m_flFogMaxEnd"));
    SCHEMA(m_bEnabled, bool, ("C_EnvSky"), ("m_bEnabled"));
};

class c_base_model_entity : public c_base_entity
{
public:
    SCHEMA(m_vec_view_offset, vec3_t, ("C_BaseModelEntity"), ("m_vecViewOffset"));
    SCHEMA(m_create_time, int, ("C_BaseModelEntity"), ("m_flCreateTime"));
    SCHEMA(m_creation_tick, int, ("C_BaseModelEntity"), ("m_nCreationTick"));
    SCHEMA(m_anim_time, float, ("C_BaseModelEntity"), ("m_flAnimTime"));

    //void set_model(const char* model_name)
    //{
    //    static const auto sig = g_modules->m_client_dll.get_address(_("m_set_model")).abs(1, 5).as< void* (__fastcall*)(void*, const char*) >();
    //    if (!sig)
    //        return;

    //    sig(this, model_name);
    //}
};

class c_postproccesing
{
public:
    SCHEMA(ExposureControl, bool, ("C_PostProcessingVolume"), ("m_bExposureControl"));
    SCHEMA(MinExposure, float, ("C_PostProcessingVolume"), ("m_flMinExposure"));
    SCHEMA(MaxExposure, float, ("C_PostProcessingVolume"), ("m_flMaxExposure"));
    SCHEMA(FadeSpeedUp, float, ("C_PostProcessingVolume"), ("m_flExposureFadeSpeedUp"));
    SCHEMA(FadeSpeedDown, float, ("C_PostProcessingVolume"), ("m_flExposureFadeSpeedDown"));
};

class C_SceneLightObject
{
public:
    char pad_0000[0xE4]; // 0x0
    float r; // 0xE4
    float g; // 0xE4
    float b; // 0xE4
};
void FrlsRefreshKnifeAnimations();
void FrlsQueueKnifeAnimations(c_base_entity* weapon);

void FrlsRefreshGloveAnimations();
