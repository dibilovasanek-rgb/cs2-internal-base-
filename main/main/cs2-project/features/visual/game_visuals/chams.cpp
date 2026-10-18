#include "../../../sdk/game_class/interfaces/i_material_system.h"
#include "../../../sdk/game_class/strong_handle.h"	
#include "../../../sdk/game_class/interfaces/keyvalue3.h"
#include "../../../sdk/game_class/interfaces/igameentitysystem.h"
#include "../../../core/interfaces/interface.h"
#include "../../../sdk/sdk.h"
#include "chams.h"
#include <atomic>
#include "../../../dependencies/config/Include/config.h"
#include "../../../core/hooks/hooks.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"

c_strong_handle<CMaterial2> c_chams::CreateMaterial(const char* szMaterialName, const char szVmatBuffer[])
{
	CKeyValues3* pKeyValues3 = CKeyValues3::CreateMaterialResource();
	if (!pKeyValues3) return {};
    if (!pKeyValues3->LoadFromBuffer(szVmatBuffer)) return {};
    static std::atomic<unsigned> material_id{};
    const auto material_name = std::string("materials/dev/frls_") + std::to_string(++material_id) + ".vmat";

	c_strong_handle<CMaterial2> pCustomMaterial = {};
	
	if (g_memory->fnCreateMaterial)
	    g_memory->fnCreateMaterial(nullptr, &pCustomMaterial, material_name.c_str(), pKeyValues3, 0, 1);

	return pCustomMaterial;
}

struct CustomMaterial_t
{
	c_strong_handle<CMaterial2> pMaterial;
	c_strong_handle<CMaterial2> pMaterialInvisible;
};
			
static CustomMaterial_t arrMaterials[VISUAL_MATERIAL_MAX];

bool c_chams::Initialize()
{
	// check if we already initialized materials
	if (init)
		return init;

	arrMaterials[VISUAL_MATERIAL_PRIMARY_WHITE] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferWhiteVisible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferWhiteInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_ILLUMINATE] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferIlluminateVisible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferIlluminateInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_LATEX] = CustomMaterial_t{
	.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferLatexVisible),
	.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferLatexInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_GLOW] = CustomMaterial_t{
	.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGlowVisible),
	.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGlowInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_GOST] = CustomMaterial_t{
	.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGostVisible),
	.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGostInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_FATAL] = CustomMaterial_t{
	.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferFatalVisible),
	.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferFatalInvisible)
	};

	init = true;
	for (auto& [pMaterial, pMaterialInvisible] : arrMaterials)
	{
		if (pMaterial == nullptr || pMaterialInvisible == nullptr)
			init = false;
	}

	return init;
}

//bool c_chams::OverrideMaterial(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
//{
//	
//	catch (...) { std::cout << "OverrideMaterial error!\n"; };
//	return true;
//}

bool c_chams::local_playerChams(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = g_memory->getOrig<decltype(&g_hook->draw_object)>("hkdraw_object");

	/*arrMeshDraw->SetShaderType("csgo_unlitgeneric.vfx");
	arrMeshDraw->SetMaterialFunction("F_TRANSLUCENT", 1);*/
	arrMeshDraw->colValue.a = 100.f ;
	oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	return true;
}

bool c_chams::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = g_memory->getOrig<decltype(&g_hook->draw_object)>("hkdraw_object");
	if (!init)
		return false;

	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return false;

	if (arrMeshDraw == nullptr)
		return false;

	if (arrMeshDraw->pSceneAnimatableObject == nullptr)
		return false;

	c_base_handle hOwner = arrMeshDraw->pSceneAnimatableObject->hOwner;

	if (!hOwner.is_valid())
		return false;

	auto pEntity = g_interface->m_game_resource_service->get_game_entity_system()->get<c_base_entity>(hOwner);
	if (pEntity == nullptr)
		return false;

	auto pPlayerPawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(hOwner);

	if (pPlayerPawn == sdk::m_local_pawn) {
		if (sdk::m_local_pawn->m_is_scoped() || sdk::m_local_pawn->is_throwing())
			return local_playerChams(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
	}
	
	//arrMeshDraw->SetMaterialFunction("F_TRANSLUCENT", 1);
	//if (CRT::StringCompare(pEntity->get_entity_class_name(), "C_CSPlayerPawn") != 0)
	//{
	//	if (CRT::StringCompare(pEntity->get_entity_class_name(), "C_CS2HudModelArms") != 0)
	//	{
	//		if (CRT::StringCompare(pEntity->get_entity_class_name(), "C_CS2HudModelWeapon") != 0)
	//			return false;

	//		if (!C_GET(bool, Vars.bVisualChamsWeapon))
	//			return false;

	//		return OverrideMaterialWeapon(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
	//	}

	//	if (!C_GET(bool, Vars.bVisualChamsHands))
	//		return false;

	//	return OverrideMaterialHands(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
	//}
	
	// hands
	if (settings.cfg().visuals.b_chams_hand) {
		if (pEntity->find_class(fnv1a::hash("C_CS2HudModelArms"))) {

			const CustomMaterial_t customMaterial = arrMaterials[std::clamp(settings.cfg().visuals.chams_style_hand,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
			const CustomMaterial_t customMaterialInvisible = arrMaterials[std::clamp(settings.cfg().visuals.chams_style_hand_invisible,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
			if (settings.cfg().visuals.b_chams_hand_invisible)
			{
				arrMeshDraw->pMaterial = customMaterialInvisible.pMaterialInvisible;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
				arrMeshDraw->colValue = settings.cfg().visuals.chams_hand_invisible;
				oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
			}

			arrMeshDraw->pMaterial = customMaterial.pMaterial;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
			arrMeshDraw->colValue = settings.cfg().visuals.chams_hand_visible;
			oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
			return true;
		}
	}

	if (settings.cfg().visuals.b_chams_weapons) {
		if (pEntity->find_class(fnv1a::hash("C_CS2HudModelWeapon"))) {
			const CustomMaterial_t customMaterial = arrMaterials[std::clamp(settings.cfg().visuals.chams_style_weapons,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
			const CustomMaterial_t customMaterialInvisible = arrMaterials[std::clamp(settings.cfg().visuals.chams_style_weapons_invisible,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
			if (settings.cfg().visuals.b_chams_weapons_invisible)
			{
				arrMeshDraw->pMaterial = customMaterialInvisible.pMaterialInvisible;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
				arrMeshDraw->colValue = settings.cfg().visuals.chams_weapon_invisible;
				oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
			}
			arrMeshDraw->pMaterial = customMaterial.pMaterial;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
			arrMeshDraw->colValue = settings.cfg().visuals.chams_weapon_visible;
			oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
			return true;
		}
	}

	// players
	if (pEntity->find_class(fnv1a::hash("C_CSPlayerPawn")))
	{
		if (!settings.cfg().visuals.b_chams)
			return false;

		const CustomMaterial_t customMaterial = arrMaterials[std::clamp(settings.cfg().visuals.chams_style,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
		const CustomMaterial_t customMaterialInvisible = arrMaterials[std::clamp(settings.cfg().visuals.chams_style_invisible,0,static_cast<int>(VISUAL_MATERIAL_MAX)-1)]; //////////////////
		auto pPlayerPawn = g_interface->m_game_resource_service->get_game_entity_system()->get<c_player_pawn>(hOwner);
		if (pPlayerPawn == nullptr)
			return false;

		if (pPlayerPawn->m_team_num() == sdk::m_local_pawn->m_team_num())
			return false;

		if (pPlayerPawn->m_is_alive() <= 0)
			return false;

		if (settings.cfg().visuals.b_chams_invisible)
		{
			arrMeshDraw->pMaterial = customMaterialInvisible.pMaterialInvisible;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
			arrMeshDraw->colValue = settings.cfg().visuals.chams_invisible;
			oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		}

		arrMeshDraw->pMaterial = customMaterial.pMaterial;
            arrMeshDraw->pMaterialSecondary = arrMeshDraw->pMaterial;
		arrMeshDraw->colValue = settings.cfg().visuals.chams_visible;
		oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		return true;
	}

	

	//if (!C_GET(bool, Vars.bVisualChams))
	//	return false;

	return false;
}
