#pragma once
#include <memory>
#include <d3d11.h>
#include <iostream>
#include <dxgi.h>
#include "../../sdk/game_class/interfaces/CViewSetup.h"
#include "../../sdk/game_class/interfaces/swapchaindx11.h"



enum e_model_type : int
{
    MODEL_SUN,
    MODEL_EFFECTS,
    MODEL_OTHER,
};

enum EClientFrameStage : int
{
    FRAME_UNDEFINED = -1,
    FRAME_START = 0,
    FRAME_SIMULATE_START = 1,
    FRAME_NET_UPDATE_START = 2,
    FRAME_NET_UPDATE_PREPROCESS = 3,
    FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE = 4,
    FRAME_NET_UPDATE_POSTDATAUPDATE_START = 5,
    FRAME_NET_UPDATE_POSTDATAUPDATE_END = 6,
    FRAME_NET_UPDATE_END = 7,
    FRAME_SIMULATE_END = 10
};

class c_user_cmd;
class c_entity_instance;
class csgoinput;
class c_envsky;
class c_postproccesing;
class CMeshData;
class c_base_scene_data;
class i_resource_system;
class c_weapon_base;
class C_GlowProperty;
class v_matrix;
class c_base_handle;
class c_base_entity;
class c_base_cs_grenade;
class c_player_pawn;
class c_cs_input_message;
class c_csgo_input_history_entry_pb;
class c_player_camera_services;

class c_hook
{
private: 
    using removed_aimfunction_t = qangle_t* (__fastcall*)(void*, qangle_t*, bool);
    using get_hash_seed = unsigned int(__fastcall*)(c_player_pawn*, qangle_t, int);

public:
    inline static ID3D11Device* m_device;
    inline static ID3D11DeviceContext* m_context;
    inline static ID3D11RenderTargetView* m_render_view;
    inline static ISwapChainDx11* m_swap_chain = nullptr;
    inline static WNDPROC g_wnd_proc;
    inline static HWND g_window;
    inline static removed_aimfunction_t fnUpdateAimPunch_fn;
    inline static get_hash_seed fnget_hash_seed;
    inline static const char* last_hook;
    inline static const char* last_func;



    static bool init();
    static void* __fastcall particle_draw_array(int64_t, int64_t, __m128*, uint64_t*, void*);
    static void __fastcall viewmodel_transform(c_base_entity*, vec3_t*, qangle_t*);
    static void* __fastcall first_person_legs(void*, void*, void*, void*, void*);
    static void __fastcall update_walls(void*, void*, void*);
    static float viewmodel_fov();
    static void remove_smoke(__int64 a1, __int64* a2, int a3, int a4, __int64 a5, __int64 a6);
    static void remove_flash(__int64 a1, int a2, __int64* a3, __int64 a4, __m128* a5);
    static void* __fastcall draw_view_punch(c_player_camera_services* pCameraServices, float* a2, float* a3, float* a4);
    static bool __fastcall should_render(c_player_pawn* pawn, int splitscreen);
    static int* create_dlight(int* a1, int a2);
    static void* hook_input(c_cs_input_message* a1, c_csgo_input_history_entry_pb* a2, bool mouseuse, bool idk, void* unk, c_player_pawn* a6);
    static void init_grenade(c_base_cs_grenade* grenade);
    static void mark_interp_latch_flags_dirty(void* a1, unsigned int a2);
    static __int64 __fastcall skybox_material(std::uintptr_t* a1, std::uintptr_t* a2, std::uintptr_t* a3, int bDrawSkybox, int a5, std::uintptr_t* a6, std::uintptr_t* a7, std::uintptr_t* a8);
    static void __fastcall equip_loadout_hook(void* rcx, uint16_t iTeam, uint16_t iSlot, uint64_t itemId, uint16_t unknown, bool update);
    static bool __fastcall draw_crosshair(c_weapon_base* pWeaponBaseGun);
    static bool __fastcall is_loadout_allow(void* a1);
    static void __fastcall set_model(void* rcx, const char* model);
    static bool __fastcall isGlowing(C_GlowProperty* glowprop);
    static void __fastcall GlowObject(C_GlowProperty* glowProperty, float* color);
    static long __fastcall hk_wnd_proc(HWND wnd, std::uint32_t msg, WPARAM wparam, LPARAM lparam);
    static void* __fastcall hk_should_update_sequences(void* a1, void* a2, void* a3);
    static __int64 __fastcall PopupAccptMatchFound(void* unk, const char* event_name, void* unk1, float unk2);
    static void __fastcall hk_frame_stage_notify(void* source_to_client, int stage);
    static void* __fastcall level_init(void* a1, const char* map_name);
    static void* __fastcall level_shutdown(void* a1);
    static void* __fastcall hk_draw_aggregate_scene_object(void* a1, void* a2, c_base_scene_data* a3, int a4, void* a5, void* a6, void* a7, void* a8);
    static void* __fastcall IsRelativeMouseMode(void* pThisptr, bool bActive);
    static void* __fastcall draw_object(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
    static bool __fastcall mouse_input_enabled(void* pThisptr);
    static void* __fastcall UpdateSkyBox(c_envsky* sky);
    static void* __fastcall UpdatePostProccesing(c_postproccesing* post_processing, int a2);
    static void* __fastcall on_add_entity(void* rcx, c_entity_instance* inst, c_base_handle handle);
    static void* __fastcall on_remove_entity(void* rcx, c_entity_instance* inst, c_base_handle handle);
    static bool __fastcall create_move(csgoinput* pInput, int nSlot, bool bActive);
    static void __fastcall get_matrix_for_view(void* render_game_system, void* view_render, v_matrix* out_world_to_view, v_matrix* out_view_to_projection, v_matrix* out_world_to_projection, v_matrix* out_world_to_pixels);
    static HRESULT __stdcall hk_present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags);
    static HRESULT __stdcall hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
    static HRESULT __stdcall hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain);
    static float __fastcall FovChnger(uintptr_t a1);
    static void __fastcall ViewModelChange(float* unk1, float* viewmodelOffsets, float* viewmodelFov);
    static void __fastcall RemoveScope(void* a1, void* a2);
    static void __fastcall AllowCameraAngleChange(csgoinput* input, int a2);
    static void __fastcall OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup);
    static void __fastcall RenderBatchList(void* a1);
    static void set_function_vector(int* a1, unsigned int hash_function, vec4_t* value);
};
inline auto g_hook = std::make_unique<c_hook>();
