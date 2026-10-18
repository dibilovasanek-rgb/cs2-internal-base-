#include "custom_sky.h"

#include "../../../dependencies/protobufs/include/google/protobuf/stubs/common.h"
#include "chams.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

#include "../../../sdk/game_class/interfaces/i_resource_system.h"
#include "../../../sdk/schema/class/entity.h"
#include "../../string.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"
#include "../../../dependencies/config/Include/config.h"
#include "../../../core/interfaces/interface.h"

void Skybox_t::InitSkybox() {
	if (bInitialized)
		return;

	std::string strSkyMaterial =
		R"#(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d}
	format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
	{
		shader = "sky.vfx"
		g_flBrightnessExposureBias = 0
		g_flRenderOnlyExposureBias = 0
		g_tSkyTexture = resource:")#" + strSkyboxPath + R"#("
	})#";

	pSkyboxMaterial = g_chams->CreateMaterial(("materials/dev/primary_white.vmat"), strSkyMaterial.c_str());
	bInitialized = pSkyboxMaterial && pSkyboxMaterial.pBinding;
}

void Skybox_t::DeleteSkybox() const {
	if (!bInitialized)
		return;

	g_interface->m_resource_handle_utils->DeleteResource(pSkyboxMaterial.pBinding);
}

void CSkyboxChanger::LoadDefaultSkyboxes() {
	for (std::string strSkyboxPath : arrSkyboxes) {
		size_t cPrefixPos = strSkyboxPath.find("materials/skybox/");
		std::string strSkyboxName = cPrefixPos == std::string::npos ? strSkyboxPath : strSkyboxPath.substr(cPrefixPos + 17);

		if (cPrefixPos == std::string::npos) {
			strSkyboxName = strSkyboxPath;
		}

		size_t cSkyPos = strSkyboxPath.find("sky_");
		if (cSkyPos != std::string::npos) {
			strSkyboxName = strSkyboxPath.substr(cSkyPos + 4);
		}

		size_t cFirstUnderscore = strSkyboxName.find('_');
		if (cFirstUnderscore != std::string::npos) {
			size_t cSecondUnderscore = strSkyboxName.find('_', cFirstUnderscore + 1);
			if (cSecondUnderscore != std::string::npos) {
				strSkyboxName = strSkyboxName.substr(0, cSecondUnderscore);
			}
			else {
				strSkyboxName = strSkyboxName.substr(0, cFirstUnderscore);
			}
		}

		if (strSkyboxName != "de_dust2") {
			strSkyboxName.erase(std::remove_if(strSkyboxName.begin(), strSkyboxName.end(), ::isdigit), strSkyboxName.end());
		}

		Skybox_t pSkybox;
		pSkybox.strSkyboxPath = strSkyboxPath;
		pSkybox.strSkyboxName = strSkyboxName.empty() ? "unnamed" : strSkyboxName;
		pSkybox.InitSkybox();

		g_customsky->vecSkyboxes.push_back(pSkybox);
	}
}

void CSkyboxChanger::UpdateSkyboxes() {
	std::string strSkyboxesDirectory = STR::GetDirectory(SkyboxModels);

	for (size_t i = 0; i < g_customsky->vecSkyboxes.size(); ) {
		const Skybox_t& pSkybox = g_customsky->vecSkyboxes[i];
		std::string strFilePath = strSkyboxesDirectory + pSkybox.strRawSkyboxName;

		if (!std::filesystem::exists(strFilePath)) {
			pSkybox.DeleteSkybox();
			g_customsky->vecSkyboxes.erase(g_customsky->vecSkyboxes.begin() + i);
		}
		else {
			++i;
		}
	}

	if (!std::filesystem::exists(strSkyboxesDirectory)) return;
	for (const auto& entry : std::filesystem::directory_iterator(strSkyboxesDirectory)) {
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension().string() != ".vtex_c")
			continue;

		std::string strRawFileName = entry.path().filename().string();
		std::string strSkyboxPath = "materials/skybox/" + strRawFileName;

		size_t cExrPos = strRawFileName.find("_exr");
		size_t cCubePos = strRawFileName.find("_cube");
		size_t cCutPos = (std::min)(cExrPos, cCubePos);

		std::string strFileName;
		if (cCutPos != std::string::npos) {
			strFileName = strRawFileName.substr(0, cCutPos);
		}
		else {
			size_t dotPos = strRawFileName.find_last_of('.');
			strFileName = dotPos != std::string::npos ? strRawFileName.substr(0, dotPos) : strRawFileName;
		}

		if (strFileName.empty()) {
			strFileName = "unnamed_" + std::to_string(g_customsky->vecSkyboxes.size());
		}

		auto it = std::find_if(g_customsky->vecSkyboxes.begin(), g_customsky->vecSkyboxes.end(),
			[&](const Skybox_t& pSkybox) { return pSkybox.strSkyboxName == strFileName; });

		if (it == g_customsky->vecSkyboxes.end()) {
			Skybox_t pSkybox;
			pSkybox.strSkyboxPath = strSkyboxPath.substr(0, strSkyboxPath.size() - 2);
			pSkybox.strSkyboxName = strFileName;
			pSkybox.strRawSkyboxName = strRawFileName;
			pSkybox.InitSkybox();

			g_customsky->vecSkyboxes.push_back(pSkybox);
		}
	}
}

void CSkyboxChanger::PrecacheSkyboxes() {
	if (!g_interface->m_engine->is_connected() || !g_interface->m_engine->is_in_game())
		return;

	if (!settings.cfg().visuals.b_customsky)
		return;

	unsigned long long uSelectedSkybox = g_customsky->nSelectedSkybox;
	if (uSelectedSkybox == ~1U)
		return;

	std::vector<Skybox_t> vecSkyboxes = g_customsky->vecSkyboxes;
	if (vecSkyboxes.size() <= uSelectedSkybox)
		return;

	vecSkyboxes[uSelectedSkybox].bPreCached = true;
	g_interface->m_resource_system->PreCache1(vecSkyboxes[uSelectedSkybox].strSkyboxPath.c_str());
}