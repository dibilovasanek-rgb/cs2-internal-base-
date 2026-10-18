#include "player_inventory.h"
#include <assert.h>
#include "../../schema/class/weapon.h"
#include "../../schema/class/player.h"
#include "i_client.h"

void playerinv::setup()
{
	fnCreateSharedObjectSubclassEconItemA = reinterpret_cast<CreateSharedObjectSubclassEconItemA>(g_memory->find_pattern(CLIENT_DLL, ("48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85")));
	fnCGCClientSharedObjectTypeCachefunc = reinterpret_cast<CGCClientSharedObjectTypeCacheA>(g_memory->find_pattern(CLIENT_DLL,"40 ? 48 83 EC ? 4C 8B 49 ? 44 8B D2"));
}

static CGCClientSharedObjectTypeCache* CreateBaseTypeCache(
	CCSPlayerInventory* pInventory)
{
    auto* raw=reinterpret_cast<std::byte*>(pInventory);
    for(int offset=0;offset<0x300;offset+=8) {
        auto* cache=*reinterpret_cast<CGCClientSharedObjectCache**>(raw+offset);
        if(NativeRtti::RttiIs(cache,".?AVCGCClientSharedObjectCache@GCSDK@@")) return cache->CreateBaseTypeCache(k_EEconTypeItem);
    }
    return nullptr;
}

CGCClientSharedObjectTypeCache* CGCClientSharedObjectCache::CreateBaseTypeCache(int nClassID)
{
	assert(playerinv::fnCGCClientSharedObjectTypeCachefunc != nullptr);
	if (playerinv::fnCGCClientSharedObjectTypeCachefunc == nullptr)
	{
		std::cout << "INVALID SIGNATURE playerinv::fnCGCClientSharedObjectTypeCachefunc" << std::endl;
	}
	return playerinv::fnCGCClientSharedObjectTypeCachefunc ? playerinv::fnCGCClientSharedObjectTypeCachefunc(this,nClassID) : nullptr;
}




CCSPlayerInventory* CCSPlayerInventory::GetInstance()
{
	CCSInventoryManager* pInventoryManager = CCSInventoryManager::GetInstance();
	if (!pInventoryManager)
		return nullptr;

	return pInventoryManager->GetLocalInventory();
}

bool CCSPlayerInventory::AddEconItem(CEconItem* pItem)
{
	// Helper function to aid in adding items.
	if (!pItem)
		return false;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (!pSOTypeCache || !pSOTypeCache->AddObject((CSharedObject*)pItem))
		return false;

	SOCreated(get_owner(), (CSharedObject*)pItem, eSOCacheEvent_Incremental);
	return true;
}

void CCSPlayerInventory::RemoveEconItem(CEconItem* pItem)
{
	// Helper function to aid in removing items.
	if (!pItem)
		return;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (!pSOTypeCache)
		return;

	const c_utl_vector<CEconItem*>& pSharedObjects =
		pSOTypeCache->GetVecObjects<CEconItem*>();
	if (pSharedObjects.find(pItem) < 0)
		return;

	SODestroyed(get_owner(), (CSharedObject*)pItem, eSOCacheEvent_Incremental);
	pSOTypeCache->RemoveObject((CSharedObject*)pItem);

	pItem->Destruct();
}

std::pair<uint64_t, uint32_t> CCSPlayerInventory::GetHighestIDs()
{
	uint64_t maxItemID = 0;
	uint32_t maxInventoryID = 0;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (pSOTypeCache)
	{
		const c_utl_vector<CEconItem*>& vecItems =
			pSOTypeCache->GetVecObjects<CEconItem*>();

		for (CEconItem* it : vecItems)
		{
			if (!it)
				continue;

			// Checks if item is default.
			if ((it->m_ulID & 0xF000000000000000) != 0)
				continue;

			maxItemID = (maxItemID > it->m_ulID) ? maxItemID : it->m_ulID;
			maxInventoryID = (maxInventoryID > it->m_unInventory) ? maxInventoryID : it->m_unInventory;
		}
	}

	return std::make_pair(maxItemID, maxInventoryID);
}

c_econ_item_view* CCSPlayerInventory::GetItemViewForItem(uint64_t itemID)
{
	c_econ_item_view* pEconItemView = nullptr;

	const c_utl_vector<c_econ_item_view*>& pItems = GetItemVector();
	for (c_econ_item_view* it : pItems)
	{
		if (it && it->m_item_id() == itemID)
		{
			pEconItemView = it;
			break;
		}
	}

	return pEconItemView;
}

CEconItem* CCSPlayerInventory::GetSOCDataForItem(uint64_t itemID)
{
	CEconItem* pSOCData = nullptr;

	CGCClientSharedObjectTypeCache* pSOTypeCache = ::CreateBaseTypeCache(this);
	if (pSOTypeCache)
	{
		const c_utl_vector<CEconItem*>& vecItems =
			pSOTypeCache->GetVecObjects<CEconItem*>();

		for (CEconItem* it : vecItems)
		{
			if (it && it->m_ulID == itemID)
			{
				pSOCData = it;
				return pSOCData;

				break;
			}
		}
	}

	return pSOCData;
}

CCSInventoryManager* CCSInventoryManager::GetInstance()
{
	if (!g_memory->fnGetInventoryManager)
		return nullptr;
	return g_memory->fnGetInventoryManager();
}


void CEconItem::SetDynamicAttributeValue(int index, void* value)
{
	c_econ_item_schema* pItemSchema =
		g_interface->m_client->get_econ_item_system() ? g_interface->m_client->get_econ_item_system()->get_econ_item_schema() : nullptr;
	if (!pItemSchema)
		return;

	void* pAttributeDefinitionInterface =
		pItemSchema->get_attribute_definition_interface(index);
	if (!pAttributeDefinitionInterface)
		return;


	static auto floating=NativeRtti::SetterFromAttributeType(GetModuleHandleW(CLIENT_DLL),".?AVCSchemaAttributeType_Float@@");
    static auto integer=NativeRtti::SetterFromAttributeType(GetModuleHandleW(CLIENT_DLL),".?AVCSchemaAttributeType_Uint32@@");
    auto fn=(index==6 || index==7 || index==8) ? floating : integer;
    if(fn && value) fn(this,pAttributeDefinitionInterface,value);
}

CEconItem* CEconItem::CreateInstance()
{
	// ida: // #STR: "Update(CEconItem)", "CEconItem", "Create(CEconItem)", "BuildCacheSubscribed(CEconItem)", "Update(CEconEquipSlot)", "CEconEquipSlot", "Create(CEconEquipSlot)", "BuildCacheSubscribed(CEconEquipSlot)", "Update(CEconPersonaDataPublic)", "CEconPersonaDataPublic"
	// ida:  sub_E0F420(
	//	1,
	//	(unsigned int)CEconItem::CreateInstance,
	//	0,
	//	(unsigned int)"CEconItem",
	//	(__int64)"BuildCacheSubscribed(CEconItem)",
	//	(__int64)"Create(CEconItem)",
	//	(__int64)"Update(CEconItem)");
	if (!playerinv::fnCreateSharedObjectSubclassEconItemA)
	{
		puts("failed fnCreateSharedObjectSubclassEconItemA");
		return {};
	}
	return playerinv::fnCreateSharedObjectSubclassEconItemA();
}
