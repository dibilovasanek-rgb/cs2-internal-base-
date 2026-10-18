#include "memory.h"
#include "../dependencies/fnv1a.h"
#include "pe64.h"
#include "../crt.h"
#include "../sdk/math/math.h"
#include "../dependencies/minhook/minhook.h"
#include "../core/interfaces/interface.h"

bool c_memory::hook_function(const std::string_view name, const LPVOID custom_function, const uintptr_t target)
{
    if (!target || !custom_function) { std::cout << "missing hook target: " << name << '\n'; return false; }
    void* original = nullptr;
    auto status = MH_CreateHook(reinterpret_cast<void*>(target), custom_function, &original);
    if (status != MH_OK) { std::cout << "hook create failed: " << name << " " << MH_StatusToString(status) << '\n'; return false; }
    m_originals[fnv1a::hash(name)] = original;
    status = MH_QueueEnableHook(reinterpret_cast<void*>(target));
    if (status != MH_OK) {
        MH_RemoveHook(reinterpret_cast<void*>(target)); m_originals.erase(fnv1a::hash(name));
        std::cout << "hook queue failed: " << name << " " << MH_StatusToString(status) << '\n'; return false;
    }
    std::cout << "prepared hook -> " << name << '\n'; return true;
}
bool c_memory::init()
{
    const void* hTier0 = GetModuleBaseHandle(TIER0_DLL);
    if (!hTier0)
        return false;

    math::fnRandomSeed = reinterpret_cast<decltype(math::fnRandomSeed)>(GetExportAddress(hTier0, "RandomSeed"));
    math::fnRandomFloat = reinterpret_cast<decltype(math::fnRandomFloat)>(GetExportAddress(hTier0, "RandomFloat"));
    math::fnRandomFloatExp = reinterpret_cast<decltype(math::fnRandomFloatExp)>(GetExportAddress(hTier0, "RandomFloatExp"));
    math::fnRandomInt = reinterpret_cast<decltype(math::fnRandomInt)>(GetExportAddress(hTier0, "RandomInt"));
    math::fnRandomGaussianFloat = reinterpret_cast<decltype(math::fnRandomGaussianFloat)>(GetExportAddress(hTier0, "RandomGaussianFloat"));

    fnUtlBufferInit = reinterpret_cast<decltype(fnUtlBufferInit)>(GetExportAddress(hTier0, "??0CUtlBuffer@@QEAA@HHW4BufferFlags_t@0@@Z"));
    fnUtlBufferPutString = reinterpret_cast<decltype(fnUtlBufferPutString)>(GetExportAddress(hTier0, "?PutString@CUtlBuffer@@QEAAXPEBD@Z"));
    fnUtlBufferEnsureCapacity = reinterpret_cast<decltype(fnUtlBufferEnsureCapacity)>(GetExportAddress(hTier0, "?EnsureCapacity@CUtlBuffer@@QEAAXH@Z"));
    fnCreateMaterial = reinterpret_cast<decltype(fnCreateMaterial)>(find_pattern(MATERIAL_SYSTEM2_DLL, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 8B F2"));

    fnGetInventoryManager = reinterpret_cast<decltype(fnGetInventoryManager)>(find_pattern(CLIENT_DLL, "48 8d 05 ?? ?? ?? ?? c3 cc cc cc cc cc cc cc cc 8b 91 ?? ?? ?? ?? b8"));

    return fnUtlBufferInit && fnUtlBufferPutString && fnUtlBufferEnsureCapacity;
}

void* FindHudElement(const char* element) {
    using fn_t = void* (*)(const char* element);
    // x-ref: #STR: "[%d] Could not find Hud Element: %s\n"
    // x-ref to relative: #STR: "CCSGO_HudChat", "#GameUI_VoiceAbusePenaltyNag" -> second function called, only function with "CCSGO_HudChat" as an arg
    using fnFindHud = std::uintptr_t* (__fastcall*)(const char* str);
    static fnFindHud FindHud = reinterpret_cast<fnFindHud>(g_memory->find_pattern(CLIENT_DLL, "40 55 48 83 EC ? 48 83 3D"));
    return FindHud(element);
}

void c_memory::sent_message_to_local_chat(const char* msg) {
    using fn_t = void(*)(void* chat, int unk, const char* msg);
    // x-ref to relative: #STR: "CCSGO_HudChat", "#GameUI_VoiceAbusePenaltyNag" -> first call after GameUI_VoiceAbusePenaltyNag
    static fn_t fn = (fn_t)find_pattern(CLIENT_DLL, "4C 89 44 24 ? 4C 89 4C 24 ? 53 B8");
    void* chat = FindHudElement("HudChatDelegate");
    fn(chat, -1, msg);
}

void* c_memory::GetExportAddress(const void* hModuleBase, const char* szProcedureName)
{
    const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);

    if (!hModuleBase) return nullptr;
    const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
    if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
        return nullptr;

    const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS64*>(pBaseAddress + pIDH->e_lfanew);
    if (pINH->Signature != IMAGE_NT_SIGNATURE)
        return nullptr;

    const IMAGE_OPTIONAL_HEADER64* pIOH = &pINH->OptionalHeader;
    const std::uintptr_t nExportDirectorySize = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    const std::uintptr_t uExportDirectoryAddress = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (nExportDirectorySize == 0U || uExportDirectoryAddress == 0U)
    {
        std::cout << ("module has no exports: \"") << std::endl;
        return nullptr;
    }

    const auto pIED = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pBaseAddress + uExportDirectoryAddress);
    const auto pNamesRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfNames);
    const auto pNameOrdinalsRVA = reinterpret_cast<const std::uint16_t*>(pBaseAddress + pIED->AddressOfNameOrdinals);
    const auto pFunctionsRVA = reinterpret_cast<const std::uint32_t*>(pBaseAddress + pIED->AddressOfFunctions);

    // Perform binary search to find the export by name
    std::size_t nRight = pIED->NumberOfNames, nLeft = 0U;
    while (nRight != nLeft)
    {
        // Avoid INT_MAX/2 overflow
        const std::size_t uMiddle = nLeft + ((nRight - nLeft) >> 1U);
        const int iResult = CRT::StringCompare(szProcedureName, reinterpret_cast<const char*>(pBaseAddress + pNamesRVA[uMiddle]));

        if (iResult == 0)
        {
            const std::uint32_t uFunctionRVA = pFunctionsRVA[pNameOrdinalsRVA[uMiddle]];

            // Check if it's a forwarded export
            if (uFunctionRVA >= uExportDirectoryAddress && uFunctionRVA - uExportDirectoryAddress < nExportDirectorySize)
            {
                // Forwarded exports are not supported
                break;
            }

            return const_cast<std::uint8_t*>(pBaseAddress) + uFunctionRVA;
        }

        if (iResult > 0)
            nLeft = uMiddle + 1;
        else
            nRight = uMiddle;
    }

    std::cout << ("export not found: ") << szProcedureName << std::endl;

    // Export not found
    return nullptr;
}

inline IMAGE_NT_HEADERS* c_memory::get_nt_headers(uintptr_t mod_base)
{
    return reinterpret_cast<IMAGE_NT_HEADERS*>(mod_base + reinterpret_cast<IMAGE_DOS_HEADER*>(mod_base)->e_lfanew);
}

uintptr_t c_memory::get_proc_address(uintptr_t mod, const char* name)
{
    IMAGE_NT_HEADERS* nt = get_nt_headers(mod);

    IMAGE_EXPORT_DIRECTORY* export_dir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(mod + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    uint32_t* function_table = reinterpret_cast<uint32_t*>(mod + export_dir->AddressOfFunctions);
    uint32_t* names_table = reinterpret_cast<uint32_t*>(mod + export_dir->AddressOfNames);
    uint16_t* ordinals_table = reinterpret_cast<uint16_t*>(mod + export_dir->AddressOfNameOrdinals);

    for (uint32_t i = 0; i < export_dir->NumberOfNames; ++i)
        if (!_stricmp(reinterpret_cast<char*>(mod + names_table[i]), name))
            return mod + function_table[ordinals_table[i]];

    return 0;
}

std::uint8_t* c_memory::ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset)
{
    if (!nAddressBytes) return nullptr;
    std::int32_t nRVA = *reinterpret_cast<std::int32_t*>(nAddressBytes + nRVAOffset);
    std::uint64_t nRIP = reinterpret_cast<std::uint64_t>(nAddressBytes) + nRIPOffset;

    return reinterpret_cast<std::uint8_t*>(nRVA + nRIP);
}
void* c_memory::GetModuleBaseHandle(const wchar_t* wszModuleName)
{
    const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));

    if (wszModuleName == nullptr)
        return pPEB->ImageBaseAddress;

    void* pModuleBase = nullptr;
    for (LIST_ENTRY* pListEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink; pListEntry != &pPEB->Ldr->InMemoryOrderModuleList; pListEntry = pListEntry->Flink)
    {
        const _LDR_DATA_TABLE_ENTRY* pEntry = CONTAINING_RECORD(pListEntry, _LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (pEntry->FullDllName.Buffer != nullptr && CRT::StringCompare(wszModuleName, pEntry->BaseDllName.Buffer) == 0)
        {
            pModuleBase = pEntry->DllBase;
            break;
        }
    }

    if (pModuleBase == nullptr)
        std::cout << ("module base not found: \"") << std::endl;

    return pModuleBase;
}

const CInterfaceRegister* c_memory::get_register_list(const wchar_t* module) {
    return reinterpret_cast<const CInterfaceRegister*>(GetModuleBaseHandle(module));
}

uintptr_t c_memory::find_pattern(const wchar_t* module_name, const char* szSignature) 
{
    auto pModuleBaseAddress = (uintptr_t)GetModuleBaseHandle(module_name);
    if (!pModuleBaseAddress || !szSignature) return 0;
    auto nSelectResultIndex = 0;
    auto PatternToBytes = [](const char* szpattern) {
        auto       m_iBytes = std::vector<int>{};
        const auto szStartAddr = const_cast<char*>(szpattern);
        const auto szEndAddr = const_cast<char*>(szpattern) + strlen(szpattern);

        for (auto szCurrentAddr = szStartAddr; szCurrentAddr < szEndAddr; ++szCurrentAddr) {
            if (*szCurrentAddr == '?') {
                ++szCurrentAddr;
                if (*szCurrentAddr == '?') ++szCurrentAddr;
                m_iBytes.push_back(-1);
            }
            else m_iBytes.push_back(strtoul(szCurrentAddr, &szCurrentAddr, 16));
        }
        return m_iBytes;
        };

    const auto pDosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
    const auto pNTHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + pDosHeader->e_lfanew);
    const auto dwSizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
    auto       m_iPatternBytes = PatternToBytes(szSignature);
    const auto pScanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);
    const auto m_iPatternBytesSize = m_iPatternBytes.size();
    const auto m_iPatternBytesData = m_iPatternBytes.data();
    size_t nFoundResults = 0;

    if (m_iPatternBytesSize == 0 || m_iPatternBytesSize > dwSizeOfImage) return 0;
    for (size_t i = 0; i <= dwSizeOfImage - m_iPatternBytesSize; ++i) {
        bool bFound = true;

        for (auto j = 0ul; j < m_iPatternBytesSize; ++j) {
            if (pScanBytes[i + j] != m_iPatternBytesData[j] && m_iPatternBytesData[j] != -1) {
                bFound = false;
                break;
            }
        }

        if (bFound) {
            if (nSelectResultIndex != 0) {
                if (nFoundResults < nSelectResultIndex) {
                    nFoundResults++;
                    bFound = false;
                }
                else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
            }
            else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
        }
    }
    return NULL;
}
