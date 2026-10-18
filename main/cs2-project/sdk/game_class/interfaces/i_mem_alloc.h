#pragma once
#include "../../memory/memory.h"
// used: mem::CallVFunc
#pragma warning(push)
#pragma warning(disable : 4191)

class i_mem_alloc
{
public:
	void* alloc(std::size_t nSize)
	{
		return g_memory->CallVFunc<void*>(this, 1, nSize);
	}

	void* realloc(const void* pMemory, std::size_t nSize)
	{
		return g_memory->CallVFunc<void*>(this, 2, pMemory, nSize);
	}

	void free(const void* pMemory)
	{
		return g_memory->CallVFunc<void>(this, 3, pMemory);
	}

	std::size_t get_size(const void* pMemory)
	{
		return g_memory->CallVFunc<std::size_t>(this, 21, pMemory);
	}
};

#pragma warning(pop)