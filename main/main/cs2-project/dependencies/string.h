#pragma once
#include <string>
#include <filesystem>
#include <stringapiset.h>
#include <fileapi.h>

#include "../../../memory/pe64.h"
#include "../../../entry.h"

enum DirectionType : int {
	PlayerModels,
	WeaponModels,
	SkyboxModels,
};

namespace STR {
	static wchar_t* Str2WC(std::string strString) {
		int iLength = MultiByteToWideChar(CP_UTF8, 0, strString.c_str(), static_cast<int>(strString.size()), NULL, 0);
		wchar_t* wszString = new wchar_t[iLength + 1];

		MultiByteToWideChar(CP_UTF8, 0, strString.c_str(), static_cast<int>(strString.size()), wszString, iLength);
		wszString[iLength] = L'\0';

		return wszString;
	}

	static std::string WC2Str(const wchar_t* wszString) {
		int iLength = WideCharToMultiByte(CP_UTF8, 0, wszString, -1, nullptr, 0, nullptr, nullptr);
		char* szString = new char[iLength];

		WideCharToMultiByte(CP_UTF8, 0, wszString, -1, szString, iLength, nullptr, nullptr);
		return std::string(szString);
	}

    static std::string GetDirectory() {
        wchar_t filename[32768]{};
        const auto module = GetModuleHandleW(L"client.dll");
        if (!module) return {};
        const DWORD length = GetModuleFileNameW(module, filename, 32768);
        if (!length || length >= 32768) return {};
        auto directory = std::filesystem::path(filename).parent_path().parent_path().parent_path().parent_path();
        auto utf8 = directory.u8string();
        return std::string(utf8.begin(), utf8.end()) + "\\";
    }

	static std::string GetGameDirectory() {
		std::string strDirectory = GetDirectory();
        if (strDirectory.empty()) return {};
		return strDirectory + ("bin\\win64\\");
	}

	static std::string GetDirectory(DirectionType eType) {
		std::string strDirectory = GetDirectory();
        if (strDirectory.empty()) return {};

		switch (eType) {
		case PlayerModels:
			return strDirectory + ("csgo\\characters\\");
		case WeaponModels:
			return strDirectory + ("csgo\\weapons\\");
		case SkyboxModels:
			return strDirectory + ("csgo\\materials\\skybox\\");
		}
        return {};
	}

	static bool CreateGameDirectory(std::string strFolder, std::string strName) {
		if (!CreateDirectoryW(STR::Str2WC(strFolder + strName), nullptr)) {
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
				std::cout << ("directory \"") << strName.c_str() << ("\" has already been created.") << std::endl;
			else
				return false;
		}
		else
			std::cout << ("directory \"") << strName.c_str() << ("\" has been created.") << std::endl;

		return true;
	}

	static bool CreateDirectories() {
		bool bSuccess = true;

		std::string strGameDirectory = GetGameDirectory();
		if (strGameDirectory.empty())
			return false;

		bSuccess &= CreateGameDirectory(strGameDirectory, ("AIRFLOW"));
		strGameDirectory += ("AIRFLOW\\");
		bSuccess &= CreateGameDirectory(strGameDirectory, ("configs"));
		bSuccess &= CreateGameDirectory(strGameDirectory, ("sounds"));
		bSuccess &= CreateGameDirectory(strGameDirectory, ("scripts"));
		strGameDirectory += ("scripts\\");
		bSuccess &= CreateGameDirectory(strGameDirectory, ("libs"));


		std::string strDirectory = GetDirectory();
        if (strDirectory.empty()) return {};

		strGameDirectory += ("csgo\\");
		bSuccess &= CreateGameDirectory(strDirectory, ("weapons"));

		bSuccess &= CreateGameDirectory(strDirectory, ("materials"));
		bSuccess &= CreateGameDirectory(strDirectory + ("materials\\"), ("skybox"));

		bSuccess &= CreateGameDirectory(strDirectory, ("characters"));
		bSuccess &= CreateGameDirectory(strDirectory + ("characters\\"), ("models"));


		return bSuccess;
	}
}