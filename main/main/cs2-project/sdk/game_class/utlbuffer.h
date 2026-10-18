#pragma once
#include "../../memory/memory.h"
class alignas(8) CUtlBuffer {
public:
    char data[0x80]{};
    CUtlBuffer(int grow_size, int size, int flags) {
        if (g_memory->fnUtlBufferInit) g_memory->fnUtlBufferInit(this, grow_size, size, flags);
    }
    bool IsValid() const {
        return g_memory->fnUtlBufferInit && g_memory->fnUtlBufferPutString && g_memory->fnUtlBufferEnsureCapacity;
    }
    bool PutString(const char* text) {
        if (!IsValid() || !text) return false;
        g_memory->fnUtlBufferPutString(this, text); return true;
    }
    bool EnsureCapacity(int size) {
        if (!IsValid()) return false;
        g_memory->fnUtlBufferEnsureCapacity(this, size); return true;
    }
};
static_assert(sizeof(CUtlBuffer) == 0x80);
