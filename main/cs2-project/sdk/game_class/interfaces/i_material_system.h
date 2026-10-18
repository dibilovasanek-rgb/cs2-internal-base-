#pragma once
#include <cstdint>
#include "../../../memory/memory.h"
#include "../../math/vector.h"
#include "../basehandle.h"
#include "../color.h"
#include "../caggregatescene.h"

struct MaterialParamater {
	vec4_t vecValue; // 0x0
	void* textureValue; // 0x10
	char pad001[0x10]; // 0x18
	const char* parameterName; // 0x28
	const char* textValue; // 0x30
	int64_t intValue; // 0x38
};

// idk
struct MaterialKeyVar_t
{
	std::uint64_t uKey;
	const char* szName;

	MaterialKeyVar_t(std::uint64_t uKey, const char* szName) :
		uKey(uKey), szName(szName) {
	}

	MaterialKeyVar_t(const char* szName, bool bShouldFindKey = false) :
		szName(szName)
	{
		uKey = bShouldFindKey ? FindKey(szName) : 0x0;
	}

	std::uint64_t FindKey(const char* szName)
	{
		using fnFindKeyVar = std::uint64_t(__fastcall*)(const char*, unsigned int, int);
		static auto oFindKeyVar = reinterpret_cast<fnFindKeyVar>(g_memory->find_pattern(PARTICLES_DLL, "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA"));

#ifdef CS_PARANOID
		CS_ASSERT(oFindKeyVar != nullptr);
#endif

		// idk those enum flags, just saw it called like that soooo yea
		return oFindKeyVar(szName, 0x12, 0x31415926);
	}
};

class CObjectInfo
{
	MEM_PAD(0xB0);
	int nId;
};

class CSceneAnimatableObject
{
	MEM_PAD(0xC0);
	c_base_handle hOwner;
};

// the naming is incorrect but i dont care atm
class CMeshData
{
public:




	MEM_PAD(0x18); // 0x0
	CSceneAnimatableObject* pSceneAnimatableObject; // 0x18
	CMaterial2* pMaterial; // 0x20
	CMaterial2* pMaterialSecondary;
    MEM_PAD(0x20);
	Color_t colValue;
	MEM_PAD(0x1c);
};

static_assert(sizeof(CMeshData)==0x70);
