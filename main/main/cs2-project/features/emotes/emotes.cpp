#include "emotes.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "../../dependencies/imgui/imgui.h"
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <fstream>

namespace emotes {
namespace {
    EmotesConfig g_config{};
    std::vector<EmoteEntry> g_catalog;
    std::filesystem::path g_pack;
    bool g_initialized = false;
    bool g_playing = false;
    bool g_key_was_down = false;
    std::chrono::steady_clock::time_point g_last_scan{};

    std::filesystem::path module_dir() {
        HMODULE module{};
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&module_dir), &module);
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(module, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
    }

    void scan_catalog() {
        g_catalog.clear();
        g_pack = module_dir() / L"emotes";
        const auto icons = g_pack / L"icons";
        std::error_code ec;
        if (std::filesystem::exists(icons, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(icons, ec)) {
                if (ec || !entry.is_regular_file()) continue;
                const auto ext = entry.path().extension().wstring();
                if (_wcsicmp(ext.c_str(), L".png") != 0 && _wcsicmp(ext.c_str(), L".jpg") != 0)
                    continue;
                g_catalog.push_back({entry.path().stem().string(), entry.path()});
            }
        }
        std::sort(g_catalog.begin(), g_catalog.end(), [](const EmoteEntry& a, const EmoteEntry& b) {
            return a.name < b.name;
        });
        g_last_scan = std::chrono::steady_clock::now();
    }

    void load_config() {
        const auto ini = (module_dir() / L"emotes.ini").wstring();
        wchar_t buf[64]{};
        GetPrivateProfileStringW(L"emotes", L"enabled", L"0", buf, 64, ini.c_str());
        g_config.enabled = std::wcstol(buf, nullptr, 0) != 0;
        GetPrivateProfileStringW(L"emotes", L"music", L"0", buf, 64, ini.c_str());
        g_config.music = std::wcstol(buf, nullptr, 0) != 0;
        GetPrivateProfileStringW(L"emotes", L"volume", L"70", buf, 64, ini.c_str());
        g_config.volume = std::clamp(std::wcstol(buf, nullptr, 0) / 100.0f, 0.0f, 1.0f);
        GetPrivateProfileStringW(L"emotes", L"wheel_key", L"0x42", buf, 64, ini.c_str());
        g_config.wheel_key = std::clamp(static_cast<int>(std::wcstol(buf, nullptr, 0)), 1, 0xFE);
        GetPrivateProfileStringW(L"emotes", L"selected", L"0", buf, 64, ini.c_str());
        g_config.selected = std::max(0, static_cast<int>(std::wcstol(buf, nullptr, 0)));
    }

    void save_config() {
        const auto ini = (module_dir() / L"emotes.ini").wstring();
        WritePrivateProfileStringW(L"emotes", L"enabled", g_config.enabled ? L"1" : L"0", ini.c_str());
        WritePrivateProfileStringW(L"emotes", L"music", g_config.music ? L"1" : L"0", ini.c_str());
        wchar_t buf[32]{};
        _snwprintf_s(buf, _countof(buf), _TRUNCATE, L"%d", static_cast<int>(g_config.volume * 100.0f));
        WritePrivateProfileStringW(L"emotes", L"volume", buf, ini.c_str());
        _snwprintf_s(buf, _countof(buf), _TRUNCATE, L"0x%02X", g_config.wheel_key);
        WritePrivateProfileStringW(L"emotes", L"wheel_key", buf, ini.c_str());
        _snwprintf_s(buf, _countof(buf), _TRUNCATE, L"%d", g_config.selected);
        WritePrivateProfileStringW(L"emotes", L"selected", buf, ini.c_str());
    }
}

void initialize() {
    if (g_initialized) return;
    load_config();
    scan_catalog();
    g_initialized = true;
}

void shutdown() {
    if (!g_initialized) return;
    stop();
    save_config();
    g_catalog.clear();
    g_initialized = false;
}

void frame() {
    if (!g_initialized) return;
    if (std::chrono::steady_clock::now() - g_last_scan > std::chrono::seconds(2)) scan_catalog();
    const bool down = g_config.enabled && (GetAsyncKeyState(g_config.wheel_key) & 0x8000) != 0;
    if (down && !g_key_was_down) trigger_selected();
    g_key_was_down = down;
}

void draw_menu() {
    if (!g_initialized) initialize();
    ImGui::TextDisabled("Emote pack");
    ImGui::Checkbox("Enable emotes", &g_config.enabled);
    ImGui::Checkbox("Music", &g_config.music);
    ImGui::SliderFloat("Volume", &g_config.volume, 0.0f, 1.0f, "%.2f");
    ImGui::InputInt("Wheel key (VK)", &g_config.wheel_key);
    g_config.wheel_key = std::clamp(g_config.wheel_key, 1, 0xFE);
    if (g_catalog.empty()) {
        ImGui::TextDisabled("No icons found");
        ImGui::TextWrapped("Expected: emotes\\icons and emotes\\csgo_addons next to the DLL.");
    } else {
        std::vector<const char*> names;
        names.reserve(g_catalog.size());
        for (const auto& e : g_catalog) names.push_back(e.name.c_str());
        g_config.selected = std::clamp(g_config.selected, 0, static_cast<int>(names.size()) - 1);
        ImGui::Combo("Emote", &g_config.selected, names.data(), static_cast<int>(names.size()));
        ImGui::TextDisabled("%s", g_playing ? "playing" : "ready");
    }
    if (ImGui::Button("Play selected", ImVec2(-1, 0))) trigger_selected();
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(-1, 0))) stop();
    ImGui::TextDisabled("Pack: %s", g_pack.string().c_str());
    save_config();
}

EmotesConfig& config() { return g_config; }
const std::vector<EmoteEntry>& catalog() { return g_catalog; }
const std::filesystem::path& pack_directory() { return g_pack; }
bool pack_available() { return std::filesystem::exists(g_pack / L"csgo_addons"); }
bool is_playing() { return g_playing; }
void trigger_selected() { if (g_config.enabled && !g_catalog.empty()) g_playing = true; }
void stop() { g_playing = false; g_key_was_down = false; }

} // namespace emotes
