#pragma once
#include <cstdint>
#include <Windows.h>
namespace CurrentBuild {
inline constexpr std::uintptr_t particle_manager=0x22271D8;
inline constexpr unsigned timestamp = 0x6AD01314;
inline constexpr unsigned image_size = 0x299D000;
inline constexpr std::uintptr_t local_player_controller = 0x253CB78;
inline constexpr std::uintptr_t view_matrix = 0x256B020;
inline constexpr std::uintptr_t input = 0x257B640;
inline constexpr std::uintptr_t entity_system = 0x271A908;
inline constexpr std::uintptr_t game_event_manager = 0x254BC08;
inline constexpr std::uintptr_t global_vars = 0x2230090;
}

namespace CurrentBuild {
inline void* Address(std::uintptr_t rva) {
    auto* base=reinterpret_cast<const unsigned char*>(GetModuleHandleW(L"client.dll"));
    if(!base) return nullptr;
    auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE) return nullptr;
    auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    if(nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.TimeDateStamp!=timestamp || nt->OptionalHeader.SizeOfImage!=image_size || rva>=image_size) return nullptr;
    return const_cast<unsigned char*>(base)+rva;
}
}

namespace CurrentBuild {
inline void* EngineVectorSetter() {
    auto* base=reinterpret_cast<unsigned char*>(GetModuleHandleW(L"engine2.dll"));
    if (!base) return nullptr;
    auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic!=IMAGE_DOS_SIGNATURE) return nullptr;
    auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    if (nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.TimeDateStamp!=0x6ACFCD53 || nt->OptionalHeader.SizeOfImage!=0x971000) return nullptr;
    return base+0x246CC0;
}
}
