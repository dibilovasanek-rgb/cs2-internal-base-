#include <Windows.h>//LW pshalko
#include <cstdio>
#include <iostream>	
#include "render/render.h"
#include "sdk/schema/schema.h"
#include "sdk/entity_listener/entity_list.h"
#include <chrono>
#include <thread>
#include "dependencies/config/Include/config.h"
#include "sdk/game_class/interfaces/i_resource_system.h"
#include "features/skinchanger/skinchanger.h"
#include "sdk/schema/class/weapon.h"
#include "dependencies/discord-rpc/discord_rpc_init.h"
#include "sdk/game_class/interfaces/event.h"
#include "sdk/game_class/interfaces/c_game_trace.h"
#include "features/visual/game_visuals/chams.h"
#include "core/hooks/hooks.h"
#include "dependencies/minhook/minhook.h"
#include "sdk/input_system/inputsystem.h"
#include "features/visual/esp.h"
#include <winternl.h>
#include "entry.h"
#include "features/visual/game_visuals/custom_sky.h"
#include "features/rage/ragebot.h"
#include "sdk/protobuffs/protobufs.h"
#include "features/visual/game_visuals/bullet_trace.h"
#include "memory/stack_trace.h"
#include "sdk/game_class/interfaces/player_inventory.h"
#include <format>
#include "features/visual/game_visuals/model_changer.h"
#pragma comment(lib, "d3d11.lib")

namespace fs = std::filesystem;

FILE* fStream{};
HMODULE ghModule;


typedef NTSTATUS(WINAPI* tNtQueryInformationProcess)(
	HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG
	);
std::string to_hex(uintptr_t value) {
	char buf[32];
	sprintf(buf, "%08X", value);
	return buf;
}
LONG __stdcall exception_handler(EXCEPTION_POINTERS* ex) {
	if (ex->ExceptionRecord->ExceptionCode <= 0x80000000)
		return EXCEPTION_CONTINUE_SEARCH;

	HMODULE mod = nullptr;
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(const char*)ex->ExceptionRecord->ExceptionAddress,
		&mod)) {
		MEMORY_BASIC_INFORMATION mem{}, cheat{};
		if (VirtualQuery(ex->ExceptionRecord->ExceptionAddress, &mem, sizeof(mem)) &&
			VirtualQuery((void*)exception_handler, &cheat, sizeof(cheat))) {
			if (mem.AllocationBase == cheat.AllocationBase)
				mod = (HMODULE)ghModule;
		}
	}

	std::string msg;
	if (!mod) {
		msg = "Exception 0x" + to_hex(ex->ExceptionRecord->ExceptionCode) +
			" at 0x" + to_hex((uintptr_t)ex->ExceptionRecord->ExceptionAddress) +
			", no module info!";
		MessageBoxA(nullptr, msg.c_str(), "CRASH!", MB_TOPMOST | MB_ICONERROR | MB_OK);
		exit(ex->ExceptionRecord->ExceptionCode);
	}

	std::string bytes;
	uint8_t* base_addr = (uint8_t*)mod;
	uint8_t* crash_addr = (uint8_t*)ex->ExceptionRecord->ExceptionAddress;

	for (int i = -10; i < 10; i++) {
		if (crash_addr + i < base_addr)
			continue;
		bytes += to_hex(*(crash_addr + i)) + " ";
	}

	char module_name[256]{};
	GetModuleFileNameA(mod, module_name, sizeof(module_name));

	const char* system_modules[] = { "ntdll.dll", "tier0.dll", "KERNELBASE.dll" };
	for (const char* sys_mod : system_modules) {
		if (strstr(module_name, sys_mod))
			return EXCEPTION_CONTINUE_SEARCH;
	}

	msg =
		"Exception 0x" + to_hex(ex->ExceptionRecord->ExceptionCode) + " at 0x" +
		to_hex((uintptr_t)ex->ExceptionRecord->ExceptionAddress) + "\n" +
		"--------------\n" +
		"Module: " + (mod == (HMODULE)ghModule ? "STRWM" : module_name) + "\n\n" +
		"Nearby bytes: " + bytes + "\n" +
		"last hook: " + g_hook->last_hook + "\n" +
		"last func: " + g_hook->last_func + "\n" +
		"--------------\n\n" +
		"Press CTRL+C and send info to forum.";

	MessageBoxA(nullptr, msg.c_str(), "CRASH!", MB_TOPMOST | MB_ICONERROR | MB_OK);
	exit(ex->ExceptionRecord->ExceptionCode);

	return EXCEPTION_CONTINUE_EXECUTION;
}




std::string GetMainModulePath() {
	char path[MAX_PATH] = { 0 };

	HMODULE hNtdll = (HMODULE)g_memory->GetModuleBaseHandle(L"ntdll.dll");
	if (!hNtdll) return "";

	tNtQueryInformationProcess NtQueryInformationProcess =
		(tNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");

	if (!NtQueryInformationProcess) return "";

	PROCESS_BASIC_INFORMATION pbi;
	ULONG returnLength;

	if (NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), &returnLength) == 0) {
		PEB* peb = pbi.PebBaseAddress;
		if (peb) {
			PEB_LDR_DATA* ldr = peb->Ldr;
			LIST_ENTRY* head = &ldr->InMemoryOrderModuleList;
			LIST_ENTRY* current = head->Flink;

			if (current != head) {
				LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(current, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
				if (entry->FullDllName.Buffer) {
					WideCharToMultiByte(CP_ACP, 0, entry->FullDllName.Buffer, -1, path, MAX_PATH, NULL, NULL);
					return std::string(path);
				}
			}
		}
	}
	return "";
}

std::vector<std::string> ParseVmdlCFiles(const std::string& directory, const std::string& folder, const std::string& extension) {
	std::vector<std::string> result;

	size_t pos = directory.find("game\\");
	if (pos == std::string::npos) {
		std::cerr << "error: 'game\\' not found path!" << std::endl;
		return {};
	}

	std::string rootPath = directory.substr(0, pos + 5);
	rootPath += "csgo\\";

	if (!fs::exists(rootPath)) {
		std::cerr << "error: directory not found!" << std::endl;
		return {};
	}

	fs::path targetPath = rootPath + folder;
	if (!fs::exists(targetPath)) {
		std::cerr << "error: directory " << folder << " not found!" << std::endl;
		return {};
	}

	for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
		if (fs::is_regular_file(entry)) {
			std::string fileStr = entry.path().string();

			if (fileStr.size() >= extension.size() + 2 &&
				fileStr.compare(fileStr.size() - extension.size() - 2, extension.size() + 2, extension + "_c") == 0) {

				size_t folderPos = fileStr.find(folder);
				if (folderPos != std::string::npos) {
					fileStr.erase(0, folderPos);
				}
				std::replace(fileStr.begin(), fileStr.end(), '\\', '/');
				if (fileStr.size() >= 2 && fileStr.substr(fileStr.size() - 2) == "_c") {
					fileStr.erase(fileStr.size() - 2);
				}
				std::cout << "[" << folder.c_str() << "]-> " << fileStr.c_str() << std::endl;
				result.push_back(fileStr);
			}
		}
	}
	return result;
}

bool Init()
{
	g_entry = std::make_unique<entry_point>();
	g_stack_trace = std::make_unique<c_stack_trace>();
	g_skins = std::make_unique<c_skins>();
	g_Discord = std::make_unique<c_Discord>();
	g_interface = std::make_unique<c_interface>();
	g_render = std::make_unique<c_render>();
	g_memory = std::make_unique<c_memory>();
	g_chams = std::make_unique<c_chams>();
	g_event = std::make_unique<c_event>();
	g_hook = std::make_unique<c_hook>();
	g_ragebot = std::make_unique<ragebot>();

	//g_stack_trace->setup_crash_handler();
	if (!g_interface->init() || !g_render->init() || !g_memory->init()) { std::cout << "initialization failed\n"; return false; }
	c_entity::setup();
	c_weapon_i::setup();
	playerinv::setup();
    g_interface->refresh();
	if (!g_hook->init()) { MH_DisableHook(MH_ALL_HOOKS); std::cout << "hook initialization failed\n"; return false; }

	g_chams->Initialize();
	g_event->Intilization();
	g_skins->setup();
    g_skins->dump_skins();
	//g_skins->create_image();

	g_entry->models_agents = ParseVmdlCFiles(GetMainModulePath(),"characters",".vmdl");
	g_entry->custom_sounds = ParseVmdlCFiles(GetMainModulePath(), "sounds", ".vsnd");
	g_entry->models_knife = ParseVmdlCFiles(GetMainModulePath(), "weapons", ".vmdl");

	g_modelchanger->UpdateWeaponModels();
	g_modelchanger->UpdatePlayerModels();

	g_customsky->UpdateSkyboxes();
	g_customsky->LoadDefaultSkyboxes();

	settings.load("Default");
	settings.init_all_cfg();
	g_Discord->Initialize();
	if (g_Discord)
		g_Discord->Update(nullptr);
	g_entry->init_cheat = true;
    const auto hook_status = MH_ApplyQueued();
    if (hook_status != MH_OK) {
        g_entry->init_cheat = false;
        MH_DisableHook(MH_ALL_HOOKS);
        std::cout << "hook activation failed: " << MH_StatusToString(hook_status) << '\n';
        return false;
    }

	return true;
}

#ifdef _DEBUG
void unload(HMODULE hModule)
{
	while (!(GetAsyncKeyState(VK_END) & 1)) { Sleep(50); }

	settings.load("Default");
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	g_skins->item_weapons.clear();
	g_skins->item_knife_default.clear();
	g_skins->item_agent.clear();
	g_skins->item_glove.clear();
	g_skins->item_stickers.clear();

	g_skins->Shutdown();
	g_skins.reset();
	//g_ragebot.reset();

	g_bullet_trace.reset();
	
	g_Discord->Shutdown();
	g_Discord.reset();

	g_interface->m_event->RemoveListener(g_event.get());
	//g_interface->m_event->Reset();

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	g_esp->avatar_cache.clear();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	g_entlistener->level_shutdown();

	g_inputsystem->Destroy();
	delete g_render->esp_drawlist;
	fclose(fStream);
	FreeConsole();
	PostMessageW(GetConsoleWindow(), WM_CLOSE, 0, 0);
	FreeLibraryAndExitThread(hModule, 0);
}
#endif // _DEBUG

DWORD WINAPI FrlsWorker(void* parameter) {
    auto module = static_cast<HMODULE>(parameter);
    while (!GetModuleHandleW(L"navsystem.dll")) Sleep(300);
#ifdef _DEBUG
    AllocConsole();
    freopen_s(&fStream, "CONOUT$", "wb", stdout);
    SetConsoleTitleA("STRWM");
#endif
    if (!Init()) return 1;
#ifdef _DEBUG
    unload(module);
#endif
    return 0;
}
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        ghModule = module;
        DisableThreadLibraryCalls(module);
        auto thread = CreateThread(nullptr, 0, FrlsWorker, module, 0, nullptr);
        if (thread) CloseHandle(thread);
    }
    return TRUE;
}
/*
+========++======++++====+=+++==+++++++===+++=======+=====+==================+==++==========+++++++
+++=+==========+++++++++=+==+++===++++++===+=+++++++=++==++++=++++==+==+++++==+==++=++====+=+++++=
+++=+++++++====++=++++++++=+=+++++++++++==+=++====++=+=+++++++++====+++++=+=+=+===+===+====++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++=+++=+++++++++++++=============++====+=++++++++++
+++++++++++++===+=++++++++++++++++++++++++++++++++++++++++++++++++++++++=+=++==++++=+++++++==+=+++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=++++++++
++++++++++++==++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++###############################+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@%@%%%@@@@@@@@@%@@@@@@%@@%%@@@%@@@%#++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++@%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++#%%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++%%%%%#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%+++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%%++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
github dibilovasanek-rgb aka strawman
*/