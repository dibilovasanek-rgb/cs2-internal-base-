#pragma once
#include "../../memory/memory.h"
#include "caggregatescene.h"

struct CSceneSkyboxObj
{
public:
	MEM_PAD(0x110); // 0x0
	CMaterial2** pSkyMaterial; // 0x110
	MEM_PAD(0x10); // 0x118 
	ByteColor colSky; // 0x128
};
