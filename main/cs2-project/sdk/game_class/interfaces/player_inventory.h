#pragma once
#include "../../../memory/native_rtti.h"
#include <utility>
#include <cstdint>
#include "../../../memory/memory.h"
#include "../utl.h"

class CGCClientSharedObjectCache;
class CEconItem;
class CGCClientSharedObjectTypeCache;
class SOID_t;

class playerinv
{
private:
	using CreateSharedObjectSubclassEconItemA = CEconItem * (__cdecl*)();
	using CGCClientSharedObjectTypeCacheA = CGCClientSharedObjectTypeCache * (__fastcall*)(void*, int);
public:
	inline static CreateSharedObjectSubclassEconItemA fnCreateSharedObjectSubclassEconItemA;
	inline static CGCClientSharedObjectTypeCacheA fnCGCClientSharedObjectTypeCachefunc;
	static void setup();
};

enum EEconItemQuality
{
	IQ_UNDEFINED = -1,
	IQ_NORMAL,
	IQ_GENUINE,
	IQ_VINTAGE,
	IQ_UNUSUAL,
	IQ_UNIQUE,
	IQ_COMMUNITY,
	IQ_DEVELOPER,
	IQ_SELFMADE,
	IQ_CUSTOMIZED,
	IQ_STRANGE,
	IQ_COMPLETED,
	IQ_HAUNTED,
	IQ_TOURNAMENT,
	IQ_FAVORED
};

enum EEconItemRarity
{
	IR_DEFAULT,
	IR_COMMON,
	IR_UNCOMMON,
	IR_RARE,
	IR_MYTHICAL,
	IR_LEGENDARY,
	IR_ANCIENT,
	IR_IMMORTAL
};

enum EEconTypeID
{
	k_EEconTypeItem = 1,
	k_EEconTypePersonaDataPublic = 2,
	k_EEconTypeGameAccountClient = 7,
	k_EEconTypeGameAccount = 8,
	k_EEconTypeEquipInstance = 31,
	k_EEconTypeDefaultEquippedDefinitionInstance = 42,
	k_EEconTypeDefaultEquippedDefinitionInstanceClient = 43,
	k_EEconTypeCoupon = 45,
	k_EEconTypeQuest = 46,
};
class CCSPlayerInventory;
class CCSInventoryManager
{
public:
	static CCSInventoryManager* GetInstance();

	auto EquipItemInLoadout(int iTeam, int iSlot, uint64_t iItemID)
	{
		using fn_t=void(__fastcall*)(void*,uint16_t,uint16_t,uint64_t,uint16_t,bool);
        static auto fn=reinterpret_cast<fn_t>(g_memory->find_pattern(CLIENT_DLL,"48 8B C4 56 57 41 56 41 57 48 83 EC 48 0F B7 F2 4D 8B F9 41 0F B7 F8 4C 8B F1 83 FE 04 0F 83 ? ? ? ? 83 FF 3A 0F 83 ? ? ? ? 48 89 58 08 44 8B"));
        auto* inventory=GetLocalInventory();if(!inventory || !fn || iTeam<0 || iTeam>=4 || iSlot<0 || iSlot>=58) return false;
        fn(inventory,static_cast<uint16_t>(iTeam),static_cast<uint16_t>(iSlot),iItemID,0,true);return true;
	}

	CCSPlayerInventory* GetLocalInventory()
	{
		static auto offset=NativeRtti::InventoryOffsetFromUseTool(reinterpret_cast<const uint8_t*>(g_memory->find_pattern(CLIENT_DLL,"48 85 D2 0F 84 ? ? ? ? 53 41 57 48 83 EC 28 49 8B D8 4C 8B F9 48 85 DB 0F 84")));
        return offset ? *reinterpret_cast<CCSPlayerInventory**>(reinterpret_cast<uintptr_t>(this)+offset) : nullptr;
	}
};


class CEconItem
{
	void SetDynamicAttributeValue(int index, void* value);
	//void SetDynamicAttributeValueString(int index, const char* value);

public:
	static CEconItem* CreateInstance();
	//c_econ_item_definition* get_attribute_definition_by_name(const char* name);
	//void set_sticker(int index, int id);

	void Destruct()
	{
		return g_memory->CallVFunc<void>(this, 1U, true);
	}

	void SetPaintKit(float kit)
	{
		SetDynamicAttributeValue(6, &kit);
	}

	void SetPaintSeed(float seed)
	{
		SetDynamicAttributeValue(7, &seed);
	}

	void SetPaintWear(float wear)
	{
		SetDynamicAttributeValue(8, &wear);
	}

	void SetStatTrak(int count)
	{
		SetDynamicAttributeValue(80, &count);
	}

	void SetStatTrakType(int type)
	{
		SetDynamicAttributeValue(81, &type);
	}

	//void SetCustomName(const char* pName)
	//{
	//	SetDynamicAttributeValueString(111, pName);
	//}
	void SetKeychains(int id) { SetDynamicAttributeValue(299, &id); }
	char pad0[0x10]; // 2 vtables
	uint64_t m_ulID;
	uint64_t m_ulOriginalID;
	void* m_pCustomDataOptimizedObject;
	uint32_t m_unAccountID;
	uint32_t m_unInventory;
	uint16_t m_unDefIndex;
	uint16_t m_unOrigin : 5;
	uint16_t m_nQuality : 4;
	uint16_t m_unLevel : 2;
	uint16_t m_nRarity : 4;
	uint16_t m_dirtybitInUse : 1;
	int16_t m_iItemSet;
	int m_bSOUpdateFrame;
	uint8_t m_unFlags;
};

struct SOID_t
{
	uint64_t m_id;
	uint32_t m_type;
	uint32_t m_padding;
};

class CSharedObject;
class CGCClientSharedObjectTypeCache
{
public:
	auto AddObject(CSharedObject* pObject)
	{
		return g_memory->CallVFunc<bool>(this, 1u, pObject);
	}

	auto RemoveObject(CSharedObject* soIndex)
	{
		return g_memory->CallVFunc<CSharedObject*>(this, 3u, soIndex);
	}

	template <typename T>
	auto& GetVecObjects()
	{
		return *reinterpret_cast<c_utl_vector<T>*>((uintptr_t)(this) + 0x8);
	}
};

class CGCClientSharedObjectCache
{
public:
	CGCClientSharedObjectTypeCache* CreateBaseTypeCache(int nClassID);
};


struct SOID_t;

class CGCClient
{
public:
};

class CGCClientSystem
{
public:
	CGCClientSystem* GetInstance();

	CGCClient* GetCGCClient()
	{
		return reinterpret_cast<CGCClient*>((uintptr_t)(this) + 0xB8);
	};
};

enum ESOCacheEvent
{
	/// Dummy sentinel value
	eSOCacheEvent_None = 0,

	/// We received a our first update from the GC and are subscribed
	eSOCacheEvent_Subscribed = 1,

	/// We lost connection to GC or GC notified us that we are no longer
	/// subscribed. Objects stay in the cache, but we no longer receive updates
	eSOCacheEvent_Unsubscribed = 2,

	/// We received a full update from the GC on a cache for which we were
	/// already subscribed. This can happen if connectivity is lost, and then
	/// restored before we realized it was lost.
	eSOCacheEvent_Resubscribed = 3,

	/// We received an incremental update from the GC about specific object(s)
	/// being added, updated, or removed from the cache
	eSOCacheEvent_Incremental = 4,

	/// A lister was added to the cache
	/// @see CGCClientSharedObjectCache::AddListener
	eSOCacheEvent_ListenerAdded = 5,

	/// A lister was removed from the cache
	/// @see CGCClientSharedObjectCache::RemoveListener
	eSOCacheEvent_ListenerRemoved = 6,
};
class c_econ_item_view;

class CCSPlayerInventory
{
public:
	static CCSPlayerInventory* GetInstance();

	auto SOCreated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return g_memory->CallVFunc<void>(this, 0u, owner, pObject, eEvent);
	}

	auto SOUpdated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return g_memory->CallVFunc<void>(this, 1u, owner, pObject, eEvent);
	}

	auto SODestroyed(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return g_memory->CallVFunc<void>(this, 2u, owner, pObject, eEvent);
	}

	auto GetItemInLoadout(int iClass, int iSlot)
	{
		return g_memory->CallVFunc<c_econ_item_view*>(this, 8u, iClass, iSlot);
	}

	bool AddEconItem(CEconItem* pItem);
	void RemoveEconItem(CEconItem* pItem);
	std::pair<uint64_t, uint32_t> GetHighestIDs();
	c_econ_item_view* GetItemViewForItem(uint64_t itemID);
	CEconItem* GetSOCDataForItem(uint64_t itemID);

	auto get_owner()
	{
		return *reinterpret_cast<SOID_t*>((uintptr_t)(this) + 0x10);
	}

	auto& GetItemVector()
	{
		return *reinterpret_cast<c_utl_vector<c_econ_item_view*>*>(
			(uintptr_t)(this) + 0x20);
	}
};
