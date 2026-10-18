#include "interface.h"
#include "../hooks/hooks.h"
#include "../../sdk/game_class/interfaces/event.h"
#include "../../sdk/game_class/interfaces/swapchaindx11.h"
#include "../../crt.h"
#include "../../sdk/game_class/interfaces/i_network_game_client.h"
#include "../../features/visual/game_visuals/bullet_trace.h"
#include "../../sdk/game_class/interfaces/csgoinput.h"
#include "../../sdk/game_class/interfaces/steam/isteamclient.h"
#include "../../sdk/game_class/interfaces/i_resource_system.h"
#include "../../sdk/game_class/interfaces/c_game_trace.h"
#include "../../sdk/game_class/interfaces/igameentitysystem.h"

template <typename T = void*>
T* c_interface::capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
{
    using factory_t=void*(__cdecl*)(const char*,int*);
    auto factory=pModuleRegister ? reinterpret_cast<factory_t>(GetProcAddress(reinterpret_cast<HMODULE>(const_cast<CInterfaceRegister*>(pModuleRegister)),"CreateInterface")) : nullptr;
    auto* result=factory ? static_cast<T*>(factory(szInterfaceName,nullptr)) : nullptr;
    if (!result) std::cout << "Missing interface: " << szInterfaceName << std::endl;
    return result;
}

bool c_interface::init()
{

	bool bSuccess = true;

	const auto pInputSystemRegisterList = g_memory->get_register_list(INPUTSYSTEM_DLL);
	if (pInputSystemRegisterList == nullptr)
		return false;

	const auto schemaRegister = g_memory->get_register_list(SCHEMASYSTEM_DLL);
	if (schemaRegister == nullptr)
		return false;

	const auto engine2register = g_memory->get_register_list(ENGINE2_DLL);
	if (engine2register == nullptr)
		return false;

	const auto clientregister = g_memory->get_register_list(CLIENT_DLL);
	if (clientregister == nullptr)
		return false;

	const auto pTier0Handle = g_memory->GetModuleBaseHandle(TIER0_DLL);
	if (pTier0Handle == nullptr)
		return false;

	const auto resource_system_reg = g_memory->get_register_list(RESOURCESYSTEM_DLL);
	if (resource_system_reg == nullptr)
		return false;

	const auto tier0_reg = g_memory->get_register_list(TIER0_DLL);
	if (tier0_reg == nullptr)
		return false;

	const auto localize_reg = g_memory->get_register_list(LOCALIZE_DLL);
	if (localize_reg == nullptr)
		return false;

	static const auto pParticleRegister = g_memory->get_register_list(PARTICLES_DLL);
	if (pParticleRegister == nullptr)
		return false;

	static const auto filesystem_stdio_dll_reg = g_memory->get_register_list(filesystem_stdio_dll);
	if (filesystem_stdio_dll_reg == nullptr)
		return false;

	using get_input_t = csgoinput * (__fastcall*)();
	get_input_t get_input = reinterpret_cast<get_input_t>(g_memory->GetAbsoluteAddress((uint8_t*)g_memory->find_pattern(CLIENT_DLL, "E8 ? ? ? ? 48 8B 56 ? 48 8B C8 E8 ? ? ? ? 4C 89 7E"), 0x1));
	m_csgoinput = get_input ? get_input() : nullptr;
    if (!m_csgoinput) return false;

	InputSystem = capture<IInputSystem>(pInputSystemRegisterList, "InputSystemVersion001");
	schema_system = capture<i_schema_system>(schemaRegister, "SchemaSystem_001");
	m_game_resource_service = capture<i_game_resource_service>(engine2register, "GameResourceServiceClientV001");

    static ISwapChainDx11 bootstrap{};
    g_hook->m_swap_chain=&bootstrap;
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount=1;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=FindWindowA("SDL_app",nullptr);
    if(!desc.OutputWindow) desc.OutputWindow=GetForegroundWindow();
    desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    if(FAILED(D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&bootstrap.pDXGISwapChain,&g_hook->m_device,nullptr,&g_hook->m_context))) return false;

	auto** allocator = reinterpret_cast<i_mem_alloc**>(g_memory->GetExportAddress(pTier0Handle, "g_pMemAlloc"));
    mem_alloc = allocator ? *allocator : nullptr;
	m_engine = capture<i_engine_client>(engine2register, "Source2EngineToClient001");
	auto** globals = reinterpret_cast<i_global_vars**>(g_memory->ResolveRelativeAddress((uint8_t*)g_memory->find_pattern(CLIENT_DLL, "48 89 15 ? ? ? ? 48 89 42"), 0x3, 0x7));
    m_globalVars = globals ? *globals : nullptr;
	auto* traceSlot=reinterpret_cast<c_game_trace**>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL,"48 8B 0D ? ? ? ? 48 8B D0 C7 44 24 ? 04 00 00 00 48 C7 44 24 ? 01 30 1C 00")),3));
    m_game_trace=traceSlot ? *traceSlot : nullptr;
	cvar = capture<i_engine_cvar>(tier0_reg, "VEngineCvar007");
	m_resource_system = capture<i_resource_system>(resource_system_reg, "ResourceSystem013");
	m_client = capture<i_client>(clientregister, "Source2Client002");
	m_localize = capture<i_localize>(localize_reg, "Localize_001");
	auto* eventSlot=reinterpret_cast<c_game_event_manager**>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL,"48 39 05 ? ? ? ? 48 89 0D")),3));
    m_event=eventSlot ? *eventSlot : nullptr;
	m_network_client_serv = capture<i_network_client_service>(engine2register, "NetworkClientService_001");
	particle_manager = capture<c_particle_manager>(pParticleRegister, "ParticleSystemMgr003");
	m_prediction = capture<c_prediction>(clientregister, "Source2ClientPrediction001");
	m_file_system = capture<c_base_file_system>(filesystem_stdio_dll_reg, "VFileSystem017");

	if (m_resource_system != nullptr)
	{
		m_resource_handle_utils = reinterpret_cast<i_resource_handle_utils*>(g_interface->m_resource_system->QueryInterface(RESOURCE_HANDLE_UTILS));
		bSuccess &= (m_resource_handle_utils != nullptr);
	}

	auto steamApi = g_memory->GetModuleBaseHandle(L"steam_api64.dll");
	if (!steamApi)
		return false;

	ISteamClient* m_steam_client = reinterpret_cast<fn_steam_client>(g_memory->get_proc_address((uintptr_t)steamApi, ("SteamClient")))();

	std::uint64_t steam_user = reinterpret_cast<tt>(g_memory->get_proc_address((uintptr_t)steamApi, ("SteamAPI_GetHSteamUser")))();
	std::uint64_t steam_pipe = reinterpret_cast<tt>(g_memory->get_proc_address((uintptr_t)steamApi, ("SteamAPI_GetHSteamPipe")))();

	m_steam_friends = m_steam_client->GetISteamFriends((std::uint64_t)steam_user, (std::uint64_t)steam_pipe, "SteamFriends017");
	m_steam_utils = m_steam_client->GetISteamUtils((std::uint64_t)steam_pipe, "SteamUtils010");

	return bSuccess && InputSystem && schema_system && m_game_resource_service && m_csgoinput && mem_alloc && m_engine && cvar && m_resource_system && m_client && m_localize && m_file_system;
}

bool c_interface::refresh() {
    static auto** globals = reinterpret_cast<i_global_vars**>(g_memory->ResolveRelativeAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL, "48 89 15 ? ? ? ? 48 89 42")), 3, 7));
    static auto** trace = reinterpret_cast<c_game_trace**>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL, "48 8B 0D ? ? ? ? 48 8B D0 C7 44 24 ? 04 00 00 00 48 C7 44 24 ? 01 30 1C 00")), 3));
    m_globalVars = globals ? *globals : nullptr;
    m_game_trace = trace ? *trace : nullptr;
    static c_game_trace* initialized = nullptr;
    if (m_game_trace && m_game_trace != initialized) { m_game_trace->hookTrace(); initialized = m_game_trace; }
    return m_globalVars && m_game_trace && m_game_resource_service && m_game_resource_service->get_game_entity_system();
}
