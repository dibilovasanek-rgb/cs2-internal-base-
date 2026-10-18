#pragma once
#include "../../memory/memory.h"

class c_net_channel_info {
public:
	//float get_latancy(Flow flow) {
	//	return MEM::CallVFunc<float, 10U>(this, flow);
	//}
};

class i_network_client {
public:
	MEM_PAD(0xE8); //0x0000
	c_net_channel_info* netchanell_info; // 0x00E8
	MEM_PAD(0x8); // 0x00F0
	bool should_predict; // 0x00F8
	MEM_PAD(0x7B); // 0x00F9
	int iSomePredictionTick; // 0x0174
	MEM_PAD(0x104); // 0x0178
	int delta_tick; // 0x027C


	bool IsConnected()
	{
		return g_memory->CallVFunc<bool>(this, 12U);
	}
	float maybe() {
		return g_memory->CallVFunc<float>(this, 62u);
	}
	// force game to clear cache and reset delta tick
	void FullUpdate()
	{
		// @ida: #STR: "Requesting full game update (%s)...\n"
		g_memory->CallVFunc<void>(this, 28U, ("unk"));
	}

	int GetDeltaTick()
	{
		// @ida: offset in FullUpdate();
		// (nDeltaTick = -1) == FullUpdate() called
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x25C);
	}

	int get_client_tick() {
		return g_memory->CallVFunc<int>(this, 5);
	}

	int get_server_tick() {
		return g_memory->CallVFunc<int>(this, 6);
	}

	const char* get_map_name() {
		return *reinterpret_cast<const char**>(reinterpret_cast<std::uintptr_t>(this) + 0x240);
	}

	bool is_connected() {
		return std::string_view{ get_map_name() }.find("maps/<empty>.vpk") == std::string::npos;
	}

	void set_prediction(bool value) {
		*(bool*)(std::uintptr_t(this) + 0xF8) = value;
	}

	void set_delta_tick(int tick) {
		*(int*)(std::uintptr_t(this) + 0x25C) = tick;
	}
};

class i_network_client_service
{
public:
	i_network_client* get_network_client() {
		return g_memory->CallVFunc<i_network_client*>(this, 23);
	}
};