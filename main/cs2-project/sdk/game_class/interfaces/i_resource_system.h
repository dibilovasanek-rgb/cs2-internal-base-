#pragma once
#include "../../../memory/memory.h"
#include "../c_buffer_string.h"
#include "../utlbuffer.h"
#include "../../../sdk/game_class/strong_handle.h"

class i_resource_system
{
private:
	constexpr uint64_t string_to_uint64(std::string_view str) {
		uint64_t result = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			result = (result << 8) | static_cast<uint8_t>(str[i]);
		}
		return result;
	}
public:
	void load_resource(const char* models_path, uint64_t extension)
	{	
        PreCache1(models_path);
	}

	void* QueryInterface(const char* szInterfaceName)
	{
		return g_memory->CallVFunc<void*>(this, 2U, szInterfaceName);
	}

	void PreCache(const char* models_path, const char* extension)
	{	
        PreCache1(models_path);
	}

	void* PreCache1(const char* name)
	{
		CBufferString names = CBufferString(name);
		using fn_t = void*(__fastcall*)(void*, void*, const char*);
        static auto fn = reinterpret_cast<fn_t>(g_memory->find_pattern(RESOURCESYSTEM_DLL, "40 53 55 57 48 81 EC ? ? ? ? 48 8B 01 49 8B E8 48 8B FA"));
        return fn ? fn(this, &names, "") : nullptr;
	}
};

class i_resource_handle_utils
{
public:
	void DeleteResource(const ResourceBinding_t* pBinding)
	{
		g_memory->CallVFunc<void>(this, 2U, pBinding);
	}
};