#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>

namespace NativeRtti {
inline bool HasBase(const void* object, const char* name)
{
    if (!object || !name) return false;
    __try
    {
        const auto* table = *reinterpret_cast<const std::uintptr_t* const*>(object);
        if (!table) return false;
        const auto* locator = reinterpret_cast<const std::uint32_t*>(table[-1]);
        if (!locator || locator[0] != 1) return false;
        const auto base = reinterpret_cast<std::uintptr_t>(locator) - locator[5];
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
        const auto size = nt->OptionalHeader.SizeOfImage;
        if (size < 16 || locator[4] > size - 16) return false;
        const auto* hierarchy = reinterpret_cast<const std::uint32_t*>(base + locator[4]);
        const auto count = hierarchy[2];
        if (!count || count > 256 || hierarchy[3] > size - count * 4) return false;
        const auto* entries = reinterpret_cast<const std::uint32_t*>(base + hierarchy[3]);
        for (std::uint32_t i = 0; i < count; ++i)
        {
            if (entries[i] > size - 4) return false;
            const auto type = *reinterpret_cast<const std::uint32_t*>(base + entries[i]);
            if (type > size - 16) return false;
            const char* typeName = reinterpret_cast<const char*>(base + type + 16);
            const auto length = std::strlen(name);
            if (length + 1 > size - type - 16) return false;
            if (std::memcmp(typeName, name, length + 1) == 0) return true;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    return false;
}
using SetAttributeFn=void(__fastcall*)(void*,void*,const void*);
struct Section
    {
        std::uint8_t* start = nullptr;
        std::size_t size = 0;
    };
inline bool GetSection(HMODULE module, const char* name, Section& out)
    {
        if (!module)
            return false;

        auto* base = reinterpret_cast<std::uint8_t*>(module);
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);

        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        {
            if (std::strncmp(reinterpret_cast<const char*>(section->Name), name, IMAGE_SIZEOF_SHORT_NAME) == 0)
            {
                out.start = base + section->VirtualAddress;
                out.size = section->Misc.VirtualSize;
                return true;
            }
        }

        return false;
    }
inline bool InSection(const Section& section, const void* address)
    {
        const auto* p = static_cast<const std::uint8_t*>(address);
        return section.start && p >= section.start && p < section.start + section.size;
    }
inline bool RttiIs(const void* object, const char* name)
    {
        if (!object || !name)
            return false;

        __try
        {
            const auto* vtable = *reinterpret_cast<const std::uintptr_t* const*>(object);

            if (!vtable)
                return false;

            const auto* locator = reinterpret_cast<const std::uint32_t*>(vtable[-1]);

            if (!locator || locator[0] != 1)
                return false;

            const std::uintptr_t imageBase = reinterpret_cast<std::uintptr_t>(locator) - locator[5];
            const char* typeName = reinterpret_cast<const char*>(imageBase + locator[3] + 0x10);
            return std::strcmp(typeName, name) == 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }
inline void** FindVTable(HMODULE module, const char* name)
    {
        Section data{}, rdata{};

        if (!GetSection(module, ".data", data) || !GetSection(module, ".rdata", rdata))
            return nullptr;

        const std::size_t nameLength = std::strlen(name);
        const auto moduleBase = reinterpret_cast<std::uintptr_t>(module);

        std::uint8_t* typeDescriptor = nullptr;

        for (std::size_t i = 16; i + nameLength + 1 < data.size; ++i)
        {
            if (data.start[i] == '.' && std::memcmp(data.start + i, name, nameLength + 1) == 0)
            {
                typeDescriptor = data.start + i - 16;
                break;
            }
        }

        if (!typeDescriptor)
            return nullptr;

        const auto typeDescriptorRva = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(typeDescriptor) - moduleBase);

        std::uint8_t* locator = nullptr;

        for (std::size_t i = 0; i + 24 <= rdata.size; i += 4)
        {
            const auto* candidate = reinterpret_cast<const std::uint32_t*>(rdata.start + i);

            if (candidate[3] != typeDescriptorRva || candidate[0] != 1 || candidate[1] != 0)
                continue;

            if (candidate[5] != static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(candidate) - moduleBase))
                continue;

            locator = rdata.start + i;
            break;
        }

        if (!locator)
            return nullptr;

        const auto locatorAddress = reinterpret_cast<std::uintptr_t>(locator);

        for (std::size_t i = 0; i + 16 <= rdata.size; i += 8)
        {
            if (*reinterpret_cast<const std::uintptr_t*>(rdata.start + i) == locatorAddress)
                return reinterpret_cast<void**>(rdata.start + i + 8);
        }

        return nullptr;
    }
inline SetAttributeFn SetterFromAttributeType(HMODULE module, const char* typeName)
    {
        Section text{};

        if (!GetSection(module, ".text", text))
            return nullptr;

        void** vtable = FindVTable(module, typeName);

        if (!vtable)
            return nullptr;

        const auto* dispatcher = static_cast<const std::uint8_t*>(vtable[6]);

        if (!InSection(text, dispatcher))
            return nullptr;

        static constexpr std::uint8_t head[] = { 0x48, 0x89, 0x5C, 0x24 };
        if (std::memcmp(dispatcher, head, sizeof(head)) != 0 || dispatcher[5] != 0x57)
            return nullptr;

        for (int i = 6; i < 0x60; ++i)
        {
            if (dispatcher[i] != 0xE8)
                continue;

            const std::int32_t relative = *reinterpret_cast<const std::int32_t*>(dispatcher + i + 1);
            const std::uint8_t* target = dispatcher + i + 5 + relative;

            if (InSection(text, target) && target[0] == 0x48 && target[1] == 0x89 && target[2] == 0x6C && target[3] == 0x24)
                return reinterpret_cast<SetAttributeFn>(const_cast<std::uint8_t*>(target));

            return nullptr;
        }

        return nullptr;
    }
inline std::uint32_t InventoryOffsetFromUseTool(const std::uint8_t* useTool)
    {
        if (!useTool)
            return 0;

        for (int i = 0; i < 0x90; ++i)
        {
            if (useTool[i] == 0xE8 && useTool[i + 5] == 0x4C && useTool[i + 6] == 0x8B && useTool[i + 7] == 0xB0)
            {
                const std::uint32_t offset = *reinterpret_cast<const std::uint32_t*>(useTool + i + 8);
                return offset > 0 && offset < 0x200000 ? offset : 0;
            }
        }

        return 0;
    }
}
