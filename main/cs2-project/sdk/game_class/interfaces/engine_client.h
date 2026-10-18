#pragma once
#include "../../../memory/memory.h"
#include "i_client.h"

class INetChannelInfo
{
public:
	float get_network_latency()
	{
		return g_memory->CallVFunc<float>(this, 10U);
	}

	float get_engine_latency()
	{
		return g_memory->CallVFunc<float>(this, 11U);
	}
};

class i_engine_client
{
public:
	int get_max_clients()
	{
		return g_memory->CallVFunc<int>(this, 38U);
	}

	bool is_in_game()
	{
		return g_memory->CallVFunc<bool>(this, 39U);
	}

	bool is_connected()
	{
		return g_memory->CallVFunc<bool>(this, 40U);
	}

	// return CBaseHandle index
    int get_local_player();



[[nodiscard]] INetChannelInfo* GetNetChannelInfo(int nSplitScreenSlot = 0)
	{
		return g_memory->CallVFunc<INetChannelInfo*>(this, 41U, nSplitScreenSlot);
	}

	c_networked_client_info* get_networked_client_info() {
		static thread_local c_networked_client_info client_info{};

		g_memory->CallVFunc<void*>(this, 173, &client_info);
		return &client_info;
	}
};
