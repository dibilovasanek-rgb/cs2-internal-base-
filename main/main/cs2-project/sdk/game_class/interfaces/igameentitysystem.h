#pragma once
#include "../basehandle.h"
#include "../../../memory/memory.h"

class c_player_pawn;
class c_player_controller;

class c_game_entity_system
{
public:
	void* get_entity_by_index(int entity_index)
	{
        if (entity_index < 0 || entity_index >= 0x7fff) return nullptr;
        auto* chunk = *reinterpret_cast<std::byte**>(reinterpret_cast<uintptr_t>(this) + 0x10 + 8 * (entity_index >> 9));
        if (!chunk) return nullptr;
        auto* identity = chunk + 0x70 * (entity_index & 0x1ff);
        return (*reinterpret_cast<uint32_t*>(identity+0x10) & 0x7fff) == static_cast<uint32_t>(entity_index) ? *reinterpret_cast<void**>(identity) : nullptr;
	}

	template <typename T = c_base_entity>
	T* get(const c_base_handle handle)
	{
		if (!handle.is_valid())
			return nullptr;

		const auto index = handle.get_entry_index();
        if (index >= 0x7fff) return nullptr;
        auto* chunk = *reinterpret_cast<std::byte**>(reinterpret_cast<uintptr_t>(this) + 0x10 + 8 * (index >> 9));
        if (!chunk) return nullptr;
        auto* identity = chunk + 0x70 * (index & 0x1ff);
        return *reinterpret_cast<uint32_t*>(identity + 0x10) == static_cast<uint32_t>(handle.to_int()) ? *reinterpret_cast<T**>(identity) : nullptr;
	}

	template <typename T = c_base_entity>
	T* get(int index)
	{
		return reinterpret_cast<T*>(this->get_entity_by_index(index));
	}


	int get_highest_entity_index()
	{
		for (int chunkIndex=63;chunkIndex>=0;--chunkIndex) {
            auto* chunk=*reinterpret_cast<std::byte**>(reinterpret_cast<uintptr_t>(this)+0x10+chunkIndex*8);
            if (!chunk) continue;
            for(int slot=511;slot>=0;--slot) {
                const int index=chunkIndex*512+slot;auto* identity=chunk+slot*0x70;
                if(index<0x7fff && *reinterpret_cast<void**>(identity) && (*reinterpret_cast<uint32_t*>(identity+0x10)&0x7fff)==index) return index;
            }
        }
        return 0;
	}
};

class i_game_resource_service
{
public:
	c_game_entity_system* get_game_entity_system() {
        static auto* slot=reinterpret_cast<c_game_entity_system**>(g_memory->GetAbsoluteAddress(reinterpret_cast<uint8_t*>(g_memory->find_pattern(CLIENT_DLL,"48 8B 0D ? ? ? ? 48 8B D3 E8 ? ? ? ? FF C5")),3));
        return slot ? *slot : nullptr;
    }
};
