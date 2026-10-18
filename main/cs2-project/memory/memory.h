#pragma once
#include <memory>
#include <unordered_map>
#include <string_view>
#include <WTypesbase.h>
#include <iostream>
#include "../dependencies/fnv1a.h"

#define CLIENT_DLL (L"client.dll")
#define ENGINE2_DLL (L"engine2.dll")
#define SCHEMASYSTEM_DLL (L"schemasystem.dll")
#define INPUTSYSTEM_DLL (L"inputsystem.dll")
#define SDL3_DLL (L"SDL3.dll")
#define TIER0_DLL (L"tier0.dll")
#define NAVSYSTEM_DLL (L"navsystem.dll")
#define RENDERSYSTEM_DLL (L"rendersystemdx11.dll")
#define LOCALIZE_DLL (L"localize.dll")
#define DBGHELP_DLL (L"dbghelp.dll")
#define GAMEOVERLAYRENDERER_DLL (L"GameOverlayRenderer64.dll")
#define RESOURCE_HANDLE_UTILS ("ResourceHandleUtils001")
#define PARTICLES_DLL (L"particles.dll")
#define SCENESYSTEM_DLL (L"scenesystem.dll")
#define MATERIAL_SYSTEM2_DLL (L"materialsystem2.dll")
#define MATCHMAKING_DLL (L"matchmaking.dll")
#define RESOURCESYSTEM_DLL (L"resourcesystem.dll")
#define SOUNDSYSTEM_DLL (L"soundsystem.dll")
#define filesystem_stdio_dll (L"filesystem_stdio.dll")

#pragma region memory_definitions
#pragma warning(push)
#pragma warning(disable: 6255) // '_alloca' indicates failure by raising a stack overflow exception. consider using '_malloca' instead
#define MEM_STACKALLOC(SIZE) _malloca(SIZE)
#pragma warning(pop)
#define MEM_STACKFREE(MEMORY) static_cast<void>(0)
#define _CS_INTERNAL_CONCATENATE(LEFT, RIGHT) LEFT##RIGHT
#define CS_CONCATENATE(LEFT, RIGHT) _CS_INTERNAL_CONCATENATE(LEFT, RIGHT)

#define MEM_PAD(SIZE)										\
private:												\
	char CS_CONCATENATE(pad_0, __COUNTER__)[SIZE]; \
public:
#pragma endregion

class CUtlBuffer;
class CGCClientSystem;
class CCSInventoryManager;
class CInterfaceRegister;

class c_memory
{
public:
	std::unordered_map< std::size_t, void* > m_originals{ };

	bool hook_function(const std::string_view name, const LPVOID custom_function, const uintptr_t target);
	bool init();
	void sent_message_to_local_chat(const char* msg);
	void* GetExportAddress(const void* hModuleBase, const char* szProcedureName);
	IMAGE_NT_HEADERS* get_nt_headers(uintptr_t mod_base);
	uintptr_t get_proc_address(uintptr_t mod, const char* name);
	std::uint8_t* ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset);
	void* GetModuleBaseHandle(const wchar_t* wszModuleName);
	const CInterfaceRegister* get_register_list(const wchar_t* wszModuleName);
	uintptr_t find_pattern(const wchar_t* module_name, const char* szSignature);
	[[nodiscard]] static uintptr_t get_virtual(void* instance, const std::size_t index)
	{
		return (*static_cast<uintptr_t**>(instance))[index];
	}

	template < typename T > T getOrig(const std::string_view name) { return (T)m_originals[fnv1a::hash(name)]; }
	
	//template <typename T, std::size_t nIndex, class CBaseClass, typename... Args_t>
	//static T CallVFunc(CBaseClass* thisptr, Args_t... argList)
	//{
	//	using VirtualFn_t = T(__thiscall*)(const void*, decltype(argList)...);
	//	return (*reinterpret_cast<VirtualFn_t* const*>(reinterpret_cast<std::uintptr_t>(thisptr)))[nIndex](thisptr, argList...);
	//}
	
	template <typename T = void*>
	inline T get_v_method(void* class_, unsigned int index) {
		try {
			if (!class_)
				return T{};

			void** table = *static_cast<void***>(class_);
			if (!table)
				return T{};

			return reinterpret_cast<T>(table[index]);
		}
		catch (...)
		{
			std::cout << "error CallVFunc" << std::endl;
			return T{};
		}
	}

	template <typename T, typename... Args>
	inline T CallVFunc(void* class_, unsigned int index, Args... args) {
		auto func = get_v_method<T(__thiscall*)(void*, Args...)>(class_, index);
		if (!func) {
			if constexpr (std::is_pointer_v<T>)
				return nullptr;
			else if constexpr (std::is_integral_v<T>) 
				return static_cast<T>(0);
			else
				return T();
		}

		return func(class_, args...);
	}


	template <typename T = void*>
	[[nodiscard]] inline T get_v_func(const void* thisptr, std::size_t nIndex)
	{
		return (*static_cast<T* const*>(thisptr))[nIndex];
	}

	template <typename T = std::uint8_t>
	T* GetAbsoluteAddress(T* pRelativeAddress, int nPreOffset = 0x0, int nPostOffset = 0x0)
	{
		if (!pRelativeAddress) return nullptr;
        pRelativeAddress += nPreOffset;		
		pRelativeAddress += sizeof(std::int32_t) + *reinterpret_cast<std::int32_t*>(pRelativeAddress);	
		pRelativeAddress += nPostOffset;
		return pRelativeAddress;
	}
	
	inline static void(__fastcall* fnUtlBufferInit)(CUtlBuffer*, int, int, int);
	inline static void(__fastcall* fnUtlBufferPutString)(CUtlBuffer*, const char*);
	inline static void(__fastcall* fnUtlBufferEnsureCapacity)(CUtlBuffer*, int);
	inline static std::int64_t(__fastcall* fnCreateMaterial)(void*, void*, const char*, void*, unsigned int, unsigned int);
	inline static void* (__fastcall* fnSetDynamicAttributeValueUint)(void*, void*,void*);
	inline static CGCClientSystem* fnGetClientSystem;
	inline static CCSInventoryManager* (*fnGetInventoryManager)();
};

inline auto g_memory = std::make_unique<c_memory>();
