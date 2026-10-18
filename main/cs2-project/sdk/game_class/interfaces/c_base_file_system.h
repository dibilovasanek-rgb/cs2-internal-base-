#pragma once 
#include "../../../memory/memory.h"

class c_base_file_system
{
public:
    int Read(void* pOutput, int size, void* file)
    {
        using Fn = int(__thiscall*)(void*, void*, int, void*);
        return g_memory->CallVFunc<int>(this, 11, pOutput, size, file);// call_virtual<Fn>(this, 0)(this, pOutput, size, file);
    }

    // if pathID is NULL, all paths will be searched for the file
    void* Open(const char* pFileName, const char* pOptions, const char* pathID = nullptr)
    {
        using Fn = void* (__thiscall*)(void*, const char*, const char*, const char*);
        return g_memory->CallVFunc<void*>(this, 13, pFileName, pOptions, pathID); // call_virtual<Fn>(this, 2)(this, pFileName, pOptions, pathID);
    }

    void Close(void* file)
    {
        using Fn = void(__thiscall*)(void*, void*);
        return g_memory->CallVFunc<void>(this, 14, file);
    }

    unsigned int Size(void* file)
    {
        using Fn = unsigned int(__thiscall*)(void*, void*);
        return g_memory->CallVFunc<unsigned int>(this, 18, file);
    }

    const char* unk(const char* file, std::uintptr_t* unk)
    {
        return g_memory->CallVFunc<const char*>(this, 60, file, unk);
    }

    bool exists(const char* pFileName, const char* a3)
    {
        using Fn = bool(__thiscall*)(void*, const char*, const char* a3);
        return g_memory->CallVFunc<bool>(this, 21, pFileName, a3);
    }
};