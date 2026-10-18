#include "keyvalue3.h"
#include "../../../crt.h"
#include "../../../sdk/game_class/utlbuffer.h"

bool CKeyValues3::LoadFromBuffer(const char* szString)
{
    if (!szString || !g_memory->fnUtlBufferInit || !g_memory->fnUtlBufferPutString || !g_memory->fnUtlBufferEnsureCapacity) return false;
	CUtlBuffer buffer(0, (CRT::StringLength(szString) + 10), 1);
	if (!buffer.PutString(szString)) return false;
	return LoadKV3(&buffer);
}

bool CKeyValues3::LoadKV3(CUtlBuffer* buffer) {
    using fn_t=bool(__fastcall*)(CKeyValues3*,void*,CUtlBuffer*,const KV3ID_t&,const char*,unsigned);
    static auto fn=reinterpret_cast<fn_t>(GetProcAddress(GetModuleHandleW(TIER0_DLL),"?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEAVCUtlBuffer@@AEBUKV3ID_t@@PEBDI@Z"));
    const KV3ID_t format{"generic",0x469806E97412167C,0xE73790B53EE6F2AF};
    return fn && buffer && fn(this,nullptr,buffer,format,"",0);
}
