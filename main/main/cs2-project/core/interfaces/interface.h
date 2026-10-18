#pragma once
#include <memory>
#include "../../memory/memory.h"

using InstantiateInterfaceFn_t = void* (*)();
class CInterfaceRegister
{
public:
	InstantiateInterfaceFn_t fnCreate;
	const char* szName;
	CInterfaceRegister* pNext;
};

struct IInputSystem;
class i_mem_alloc;
class i_schema_system;
class i_resource_system;
class c_game_particle_manager_system;
class c_particle_manager;
class i_client;
class c_game_trace;
class i_game_resource_service;
class i_resource_handle_utils;
class i_engine_cvar;
class i_global_vars;
class i_engine_client;
class c_game_event_manager;
class c_prediction;
class i_network_client_service;
class ISteamClient;
class ISteamFriends;
class ISteamUtils;
class csgoinput;
class c_base_file_system;

class i_localize {
public:
	const char* find_key(const char* key) {
		return g_memory->CallVFunc<const char*>(this, 17, key);
	}
};

class c_interface
{
public:
	bool init();
    bool refresh();

	using tt = uint64_t(__cdecl*)(void);
	using fn_steam_client = ISteamClient * (__cdecl*)(void);

	IInputSystem* InputSystem = nullptr;
	i_schema_system* schema_system = nullptr;
	i_game_resource_service* m_game_resource_service = nullptr;
	csgoinput* m_csgoinput = nullptr;
	i_engine_client* m_engine = nullptr;
	i_global_vars* m_globalVars = nullptr;
	c_game_trace* m_game_trace = nullptr;
	i_mem_alloc* mem_alloc = nullptr;
	i_engine_cvar* cvar = nullptr;
	i_resource_system* m_resource_system = nullptr;
	i_resource_handle_utils* m_resource_handle_utils = nullptr;
	i_client* m_client = nullptr;
	i_localize* m_localize = nullptr;
	c_game_event_manager* m_event = nullptr;
	i_network_client_service* m_network_client_serv = nullptr;
	c_game_particle_manager_system* game_particle_manager_system = nullptr;
	c_particle_manager* particle_manager = nullptr;
	c_prediction* m_prediction = nullptr;
	ISteamFriends* m_steam_friends = nullptr;
	ISteamUtils* m_steam_utils = nullptr;
	c_base_file_system* m_file_system = nullptr;

	template<typename T>
	T* capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName);
};

inline auto g_interface = std::make_unique<c_interface>();

