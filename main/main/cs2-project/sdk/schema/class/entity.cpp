#include "entity.h"
#include "../../game_class/interfaces/igameentitysystem.h"
#include <unordered_map>
#include <unordered_set>
namespace {
struct PendingGraph { ULONGLONG due{}; unsigned tries{}; uint32_t subclass{}; bool applied{}; };
std::unordered_map<uint32_t,PendingGraph> pending;
uint32_t schema_get_offset(const char* cls,const char* field) { return g_schema->schema_get_offset(cls,field); }
using rebuild_graph_fn = void(__fastcall*)(void*,std::uint8_t);
bool invoke_graph_rebuild(void* hud, rebuild_graph_fn rebuild,
    std::uint32_t body_offset, std::uint32_t controller_offset, std::uint32_t graph_offset) {
    __try {
        auto* body = *reinterpret_cast<std::byte**>(static_cast<std::byte*>(hud) + body_offset);
        if (!body) return false;
        auto* controller = body + controller_offset;
        if (!*reinterpret_cast<void**>(controller + graph_offset)) return false;
        rebuild(controller,2);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool rebuild_hud_anim_graph(void* hud) {
    static auto rebuild = reinterpret_cast<rebuild_graph_fn>(g_memory->find_pattern(CLIENT_DLL,"40 55 56 48 83 EC 28 4C 89 74 24 58 48 8B F1 80 FA FF 75 04 0F B6 51 18"));
    static const auto body_offset = schema_get_offset("C_BaseEntity","m_CBodyComponent");
    static const auto controller_offset = schema_get_offset(
        "CBodyComponentBaseAnimGraph","m_animationController");
    static const auto graph_offset = schema_get_offset(
        "CBaseAnimGraphController","m_pGraphInstanceAG2");
    if (!hud || !rebuild || !body_offset || !controller_offset || !graph_offset) return false;
    return invoke_graph_rebuild(hud,rebuild,body_offset,controller_offset,graph_offset);
}

void precache(const char* model) {
    using insert_t=const char*(__fastcall*)(void*,int,const char*,int,bool);
    using purge_t=void(__fastcall*)(void*,int);
    using precache_t=void*(__fastcall*)(void*,void*,const char*);
    static auto insert=reinterpret_cast<insert_t>(GetProcAddress(GetModuleHandleW(TIER0_DLL),"?Insert@CBufferString@@QEAAPEBDHPEBDH_N@Z"));
    static auto purge=reinterpret_cast<purge_t>(GetProcAddress(GetModuleHandleW(TIER0_DLL),"?Purge@CBufferString@@QEAAXH@Z"));
    static auto fn=reinterpret_cast<precache_t>(g_memory->find_pattern(RESOURCESYSTEM_DLL,"40 53 55 57 48 81 EC ? ? ? ? 48 8B 01 49 8B E8 48 8B FA"));
    struct Buffer {int length;uint32_t flags;uint64_t data;};Buffer buffer{0,0xc0000008,0};
    if(!model || !*model || !insert || !purge || !fn || !g_interface->m_resource_system) return;
    static std::unordered_set<std::string> cached;
    if (!cached.insert(model).second) return;
    insert(&buffer,0,model,-1,false);fn(g_interface->m_resource_system,&buffer,"");purge(&buffer,0);
}
}
void c_base_entity::UpdateSubClass() {
    if(!c_entity::get_update) return;
    const auto handle=get_ref_ehandle();if(!handle.is_valid()) return;
    auto& state=pending[static_cast<uint32_t>(handle.to_int())];
    if(state.applied && state.subclass==m_nSubclassID()) return;
    c_entity::get_update(this);
    state.subclass=m_nSubclassID();state.applied=true;state.due=GetTickCount64()+250;state.tries=0;
}
void FrlsQueueKnifeAnimations(c_base_entity* weapon) {
    if (!weapon) return;
    auto& state = pending[static_cast<uint32_t>(weapon->get_ref_ehandle().to_int())];
    state.due = GetTickCount64() + 250;
    state.tries = 0;
}
void FrlsRefreshKnifeAnimations() {
    auto* system=g_interface->m_game_resource_service ? g_interface->m_game_resource_service->get_game_entity_system() : nullptr;
    if(!system) {pending.clear();return;}
    for(auto it=pending.begin();it!=pending.end();) {
        c_base_handle handle(it->first);auto* weapon=handle.get();
        if(!weapon) {it=pending.erase(it);continue;}
        auto& state=it->second;
        if(state.due && GetTickCount64()>=state.due) {
            bool rebuilt=false;
            for(int i=1,highest=system->get_highest_entity_index();i<=highest;++i) {
                auto* hud=system->get<c_base_entity>(i);
                if(hud && hud->find_class(fnv1a::hash("C_CS2HudModelWeapon")) && hud->get_owner_handle()==handle) {rebuilt=rebuild_hud_anim_graph(hud);break;}
            }
            if(rebuilt || ++state.tries>=8) state.due=0;
            else state.due=GetTickCount64()+250;
        }
        ++it;
    }
}

void c_base_entity::set_model(const char* model_name) {
    if (!c_entity::SetModel || !model_name || !*model_name) return;
    precache(model_name);
    c_entity::SetModel(this, model_name);
}

void FrlsRefreshGloveAnimations() {
    auto* system = g_interface->m_game_resource_service ? g_interface->m_game_resource_service->get_game_entity_system() : nullptr;
    if (!system) return;
    for (int i = 1, highest = system->get_highest_entity_index(); i <= highest; ++i) {
        auto* entity = system->get<c_base_entity>(i);
        if (entity && entity->find_class(fnv1a::hash("C_CS2HudModelArms"))) rebuild_hud_anim_graph(entity);
    }
}
