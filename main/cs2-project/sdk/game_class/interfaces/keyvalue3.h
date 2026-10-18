#pragma once
#include "../../../memory/memory.h"
// used: MEM_PAD, FindPattern

struct KV3ID_t
{
	const char* szName;
	std::uint64_t unk0;
	std::uint64_t unk1;
};

class CKeyValues3
{
public:
	
	std::uint64_t uKey;
	void* pValue;
	

	bool LoadFromBuffer(const char* szString);
	bool LoadKV3(CUtlBuffer* buffer);

	static CKeyValues3* CreateMaterialResource()
	{
		using fnSetTypeKV3 = CKeyValues3*(__fastcall*)(CKeyValues3*, unsigned int, unsigned int);
		static const fnSetTypeKV3 oSetTypeKV3 = reinterpret_cast<fnSetTypeKV3>(g_memory->find_pattern(TIER0_DLL, "40 53 48 83 EC 30 80 FA 06 0F B6 C2 41 B9 16"));

#ifdef CS_PARANOID
		CS_ASSERT(oSetTypeKV3 != nullptr);
#endif

		CKeyValues3* pKeyValue = new CKeyValues3{};
		if (!oSetTypeKV3) { delete pKeyValue; return nullptr; }
        oSetTypeKV3(pKeyValue, 6U, 1U);
        return pKeyValue;
	}
};
