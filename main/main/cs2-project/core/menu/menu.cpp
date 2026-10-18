#include "../../dependencies/imgui/imgui.h"
#include "menu.h"
#include <algorithm>
#include "../../dependencies/imgui/imgui_internal.h"
#include "../../dependencies/config/Include/config.h"
#include "../../render/render.h"
#include "../../render/liquid_glass.h"
#include "../../features/rage/ragebot.h"
#include "../../entry.h"
#include "../../sdk/input_system/inputsystem.h"
#include "../../features/rage/anti_aim.h"
#include "../../features/skinchanger/skinchanger.h"
#include "../../dependencies/vtex_parser/vtex_parser.h"
#include "../../sdk/game_class/interfaces/player_inventory.h"
#include "../hooks/hooks.h"
#include "widget.h"
#include "../match_tools/invalid_steam_logon.h"
#include "../../dependencies/imgui/stb_image.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "../../features/visual/game_visuals/custom_sky.h"
#include "../../features/visual/game_visuals/model_changer.h"
#include "../../features/emotes/emotes.h"
#define CXOR(str) str

bool s = false;
int inventory_tab = 0;
int skins_item_def = -1;
int saved_item_def = -1;
int paint_seed = 0;
int paint_wear = 0;
int stattrack = 0;
int current_sticker = -1;
std::array<gui_skins, 5> current_stickers{};
gui_skins current_skin;

struct menu_animation_t
{
    std::vector<ID3D11ShaderResourceView*> frames;
    std::vector<int> delays;
    std::chrono::steady_clock::time_point started{};
};
static menu_animation_t g_menu_mascot{};
static menu_animation_t g_menu_background{};

static bool load_menu_asset(const char* filename, menu_animation_t& animation)
{
    if (!g_hook || !g_hook->m_device || !g_render || !animation.frames.empty()) return !animation.frames.empty();
    char module_path[MAX_PATH]{};
    HMODULE module = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&load_menu_asset), &module);
    GetModuleFileNameA(module, module_path, sizeof(module_path));
    std::filesystem::path asset_root = std::filesystem::path(module_path).parent_path() / "assets";
    const std::string module_asset = (asset_root / filename).string();
    const std::string local_asset = std::string("assets\\") + filename;
    const char* candidates[] = { module_asset.c_str(), local_asset.c_str() };
    std::vector<unsigned char> file_data;
    for (const char* path : candidates) {
        std::ifstream file(path, std::ios::binary);
        if (!file) continue;
        file_data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        if (!file_data.empty()) break;
    }
    if (file_data.empty()) return false;
    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(file_data.data(), static_cast<int>(file_data.size()),
        &w, &h, &channels, 4);
    if (!pixels) return false;
    auto* texture = g_render->load_texture_from_buffer(pixels, w, h);
    if (texture) {
        animation.frames.push_back(texture);
        animation.delays.push_back(1000);
    }
    stbi_image_free(pixels);
    animation.started = std::chrono::steady_clock::now();
    return !animation.frames.empty();
}

static void load_menu_mascot()
{
    load_menu_asset("pink-transparent.gif", g_menu_mascot);
    // Use a single decoded frame from the MP4 as a lightweight background.
    // Keeping one GPU texture avoids the CPU/RAM spike caused by decoding GIF frames.
    load_menu_asset("jerking-car-man.bmp", g_menu_background);
}

static ID3D11ShaderResourceView* current_frame(menu_animation_t& animation)
{
    if (animation.frames.empty()) return nullptr;
    int total = 0;
    for (int delay : animation.delays) total += delay;
    if (total <= 0) return animation.frames.front();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - animation.started).count();
    int position = static_cast<int>(elapsed % total);
    for (size_t i = 0; i < animation.frames.size(); ++i) {
        if (position < animation.delays[i]) return animation.frames[i];
        position -= animation.delays[i];
    }
    return animation.frames.back();
}

void init_skin_image(gui_skins& skin)
{
    if (skin.type != type_glove) {
        for (auto& it : g_skins->item_weapons[skin.item_def])
        {
            if (!it.path_image.data())
                continue;

            auto vtex = VTexParser::Load(it.path_image.c_str());

            if (vtex.data.empty())
                continue;
            g_render->LoadAndRenderImage(g_hook->m_device, vtex, &it.texture_id);

            //it.texture_id = (int)g_render->load_texture_from_buffer(vtex.data.data(), vtex.w, vtex.h);
            it.is_initilization = true;
        }
    }
    else {
        for (auto& it : g_skins->item_glove)
        {
            if (!it.path_image.data())
                continue;

            auto vtex = VTexParser::Load(it.path_image.c_str());

            if (vtex.data.empty())
                continue;
            g_render->LoadAndRenderImage(g_hook->m_device, vtex, &it.texture_id);

            //it.texture_id = (int)g_render->load_texture_from_buffer(vtex.data.data(), vtex.w, vtex.h);
            it.is_initilization = true;
        }
    }
}

void init_skin()
{
    for (auto& it : g_skins->skin_added)
    {
        if (!it.path_image.data())
            return;

        if (it.is_initilization)
            continue;

        auto vtex = VTexParser::Load(it.path_image.c_str());

        if (vtex.data.empty())
            continue;
        g_render->LoadAndRenderImage(g_hook->m_device, vtex, &it.texture_id);

        //it.texture_id = (int)g_render->load_texture_from_buffer(vtex.data.data(), vtex.w, vtex.h);
        it.is_initilization = true;
    }
}

void init_stickers_image()
{
    for (auto& it : g_skins->item_stickers)
    {
        if (!it.path_image.data())
            return;

        auto vtex = VTexParser::Load(it.path_image.c_str());

        if (vtex.data.empty())
            continue;
        g_render->LoadAndRenderImage(g_hook->m_device, vtex, &it.texture_id);
        //	it.texture_id = (int)g_render->load_texture_from_buffer(vtex.data.data(), vtex.w, vtex.h);
        it.is_initilization = true;
    }
}

void draw_color_edit(const char* label, Color_t& color) {
    float col[4] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    };

    if (widgets::color_edit4(label, col, ALPHA)) {
        color = Color_t{
            static_cast<uint8_t>(col[0] * 255.0f),
            static_cast<uint8_t>(col[1] * 255.0f),
            static_cast<uint8_t>(col[2] * 255.0f),
            static_cast<uint8_t>(col[3] * 255.0f)
        };
    }
}

struct ClickData {
    ImVec2 pos;
    float value = 0.0f;
};
#define ALPHA ( ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar)

static int mode;

void render_subtabs(const char* render_subtab_items[], int* render_subtab_tab, int render_subtab_count)
{
    constexpr float tab_width = 132.0f;
    constexpr float tab_gap = 8.0f;
    ImGui::BeginGroup();
    for (int i = 0; i < render_subtab_count; ++i)
    {
        ImGui::SetCursorPos(ImVec2(12.0f + i * (tab_width + tab_gap), 12.0f));
        if (widgets::subtab(render_subtab_items[i], *render_subtab_tab == i))
            *render_subtab_tab = i;
    }
    ImGui::EndGroup();
}
namespace MENU
{

    inline int rage_subtab = 0;
    inline int legit_subtab = 0;
    inline int world_subtab = 0;
    inline int visuals_subtab = 0;
    inline int skins_subtab = 0;
    inline int misc_subtab = 0;
    inline int cloud_subtab = 0;
}

void tab_ragebot()
{

    const char* rage_items[] = { "Rage", "Anti-Aim" };

    render_subtabs(rage_items, &MENU::rage_subtab, IM_ARRAYSIZE(rage_items));

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##Ragebot_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::BeginGroup();
        {
            switch (MENU::rage_subtab) {
            case 0: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox("RageBot", &settings.cfg().ragebot.b_ragebot);

                    if(settings.cfg().ragebot.b_ragebot)
                    {
                        widgets::checkbox("doubletap", &settings.cfg().ragebot.m_doubletap);
                        widgets::multicombo("RageBot hiboxes", settings.cfg().ragebot.m_hitboxes, hitboxes, IM_ARRAYSIZE(hitboxes));
                        widgets::multicombo("RageBot multi points", settings.cfg().ragebot.m_multi_points, multi_points, IM_ARRAYSIZE(multi_points));
                        widgets::slider_int("RageBot scale", &settings.cfg().ragebot.m_multipoint_scale, 1, 100);

                        widgets::slider_int("RageBot min damage", &settings.cfg().ragebot.r_min_damage, 0, 101);
                        widgets::slider_int("RageBot min damage override", &settings.cfg().ragebot.r_min_damage_over, 0, 101);
                        widgets::keybind("RageBot override key", &settings.cfg().ragebot.key_min_damage_over, &settings.cfg().ragebot.style_min_damage_over);
                        widgets::slider_int("RageBot hitchance", &settings.cfg().ragebot.r_hitchance, 0, 100);
                    }
                }
                ImGui::EndChild();
            } break;
            }

            switch (MENU::rage_subtab) {
            case 1: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Anti-aim settings"));
                {
                    widgets::checkbox("Anti-aim", &settings.cfg().ragebot.b_anti_aim);
                    if (settings.cfg().ragebot.b_anti_aim)
                    {
                        widgets::combo("[Anti-aim] styles", &settings.cfg().ragebot.aa_style, aa_styles, IM_ARRAYSIZE(aa_styles));

                        if (settings.cfg().ragebot.aa_style == CUSTOM) {
                            widgets::slider_float("Anti-aim yaw", &settings.cfg().ragebot.aa_yaw, -179, 179);
                            widgets::slider_float("Anti-aim pitch", &settings.cfg().ragebot.aa_pitch, -119, 119);
                        }
                        if (settings.cfg().ragebot.aa_style == JITTER)
                            widgets::slider_int("Anti-aim speed", &settings.cfg().ragebot.jitter_speed, 1, 5);

                        widgets::keybind("Left Manual Key", &settings.cfg().ragebot.i_key_left_manual, &settings.cfg().ragebot.i_style_manual);
                        widgets::keybind("Right Manual Key", &settings.cfg().ragebot.i_key_right_manual, &settings.cfg().ragebot.i_style_manual);
                    }
                    widgets::combo("Crosshairs", &settings.cfg().misc.crosshairs, crosshairsName, IM_ARRAYSIZE(crosshairsName));

                }
                ImGui::EndChild();
            } break;
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}

void tab_legitbot()
{
    const char* legit_items[] = { ("General") };
    render_subtabs(legit_items, &MENU::legit_subtab, IM_ARRAYSIZE(legit_items));

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##Legitbot_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::BeginGroup();
        {
            switch (MENU::legit_subtab) {
            case 0: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox("Legit Aim", &settings.cfg().legitbot.b_aim_fov);
                    if (settings.cfg().legitbot.b_aim_fov)
                    {
                        widgets::checkbox("[Legit Aim] visible check", &settings.cfg().legitbot.legit_visible);
                        widgets::slider_float("[Legit Aim] fov", &settings.cfg().legitbot.aim_Fov, 0, 180);
                        widgets::slider_float("[Legit Aim] smooth", &settings.cfg().legitbot.legit_smooth, 0.1, 1);
                        widgets::checkbox("[Legit Aim] draw fov", &settings.cfg().legitbot.b_drawFov);
                    }
                    widgets::checkbox("TriggerBot", &settings.cfg().legitbot.b_trigger);
                    if (settings.cfg().legitbot.b_trigger)
                    {
                        widgets::slider_int("[Trigger Bot] damage", &settings.cfg().legitbot.trigger_min_dmg, 0, 101);
                        widgets::slider_int("[Trigger Bot] hitchance", &settings.cfg().legitbot.trigger_hitchance, 0, 100);
                    }
                    //ImGui::SliderFloat("fov", &settings.cfg().legitbot.aim_Fov, 0, 160);
                    //ImGui::SliderFloat("smooth", &settings.cfg().legitbot.legit_smooth, 0, 50);
                    //ImGui::Combo("bone", &settings.cfg().legitbot.legit_bone, Bones, IM_ARRAYSIZE(Bones));
                }
                ImGui::EndChild();
            } break;
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine(0, 12);
        ImGui::BeginGroup();
        {
            switch (MENU::legit_subtab) {
            case 0: {
                widgets::child(ICON_FA_GUN, CXOR("Binds")); {
                    ImGui::Text(CXOR("Later..."));
                }
                ImGui::EndChild();
            } break;
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}

void tab_visuals()
{
    const char* visuals_items[] = { "Enemy", "Team", "Local", "Emotes" };
    render_subtabs(visuals_items, &MENU::visuals_subtab, IM_ARRAYSIZE(visuals_items));

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##Visuals_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::BeginGroup();
        {
            switch (MENU::visuals_subtab) {
            case 0: { // Enemy
                widgets::child(ICON_FA_SLIDERS, CXOR("esp"));
                {
                    widgets::checkbox("Enemy ESP", &settings.cfg().visuals.b_esp);
                    if (settings.cfg().visuals.b_esp) {
                        widgets::checkbox("Active ESP Only by sound", &settings.cfg().visuals.b_sound_only_esp);
                        widgets::checkbox("Active ESP Only visible", &settings.cfg().visuals.b_onlyvisible);

                        widgets::checkbox("Bounding box", &settings.cfg().visuals.b_box);
                        if (settings.cfg().visuals.b_box)
                        {
                            draw_color_edit("Bounding box color", settings.cfg().visuals.box_color);
                        }
                        widgets::checkbox("Healthbar", &settings.cfg().visuals.b_healthBar);
                        if (settings.cfg().visuals.b_healthBar)
                        {
                            draw_color_edit("Healthbar color", settings.cfg().visuals.heathbar_color);
                            widgets::checkbox("Healthbar gradient", &settings.cfg().visuals.b_gradienthpbar);
                            if (settings.cfg().visuals.b_gradienthpbar)
                            {
                                draw_color_edit("Healthbar gradient color", settings.cfg().visuals.c_gradienthpbar2);
                            }
                            widgets::checkbox("Healthbar glow", &settings.cfg().visuals.b_hpbar_glow);
                        }
                        widgets::checkbox("Nicknames", &settings.cfg().visuals.b_nicknames);
                        if (settings.cfg().visuals.b_nicknames)
                        {
                            widgets::checkbox("Nicknames player avatar", &settings.cfg().visuals.b_playeravatar);
                        }
                        widgets::multicombo("Flags", settings.cfg().visuals.m_flags.data(), flags_array, IM_ARRAYSIZE(flags_array));
                        widgets::checkbox("Sound esp", &settings.cfg().visuals.b_sound_esp);
                        widgets::checkbox("Weapon icons", &settings.cfg().visuals.b_weaponicon);
                        widgets::checkbox("Weapon name", &settings.cfg().visuals.b_weaponname);
                        widgets::checkbox("Weapon bar", &settings.cfg().visuals.b_weaponbar);
                        if (settings.cfg().visuals.b_weaponbar)
                        {
                            draw_color_edit("Weapon bar color", settings.cfg().visuals.c_weaponbar);
                            widgets::checkbox("Weapon bar gradient", &settings.cfg().visuals.b_gradientammobar);
                            if (settings.cfg().visuals.b_gradientammobar)
                            {
                                draw_color_edit("Weapon bar gradient color", settings.cfg().visuals.c_gradientammobar2);
                            }
                            widgets::checkbox("Weapon bar glow", &settings.cfg().visuals.b_ammobar_glow);
                        }
                        widgets::checkbox("OOF Arrow", &settings.cfg().visuals.b_oof);
                        draw_color_edit("OOF Arrow color", settings.cfg().visuals.oof_color);
                        if (settings.cfg().visuals.b_oof) {
                            widgets::slider_float("OOF Arrow radius", &settings.cfg().visuals.oof_radius, 20, 200);
                            widgets::slider_int("OOF Arrow size", &settings.cfg().visuals.oof_size, 0, 6);
                        }
                        widgets::checkbox("Skeletone", &settings.cfg().visuals.b_skeleton);
                        if (settings.cfg().visuals.b_skeleton) {
                            draw_color_edit("Skeletone color", settings.cfg().visuals.skeleton_color);
                        }

                        widgets::checkbox("Glow", &settings.cfg().visuals.b_glow_esp);
                        if (settings.cfg().visuals.b_glow_esp) {
                            draw_color_edit("Glow color", settings.cfg().visuals.glow_color);
                        }

                        widgets::checkbox("Onhits wireframe", &settings.cfg().visuals.b_onhits);
                        if (settings.cfg().visuals.b_onhits) {
                            draw_color_edit("Onhits color", settings.cfg().visuals.onhit_color);
                        }
                    }

                }
                ImGui::EndChild();
            } break;
            case 1: { // Team
                widgets::child(ICON_FA_SLIDERS, CXOR("Team ESP"));
                {
                    ImGui::Text("Later");

                }
                ImGui::EndChild();
            } break;
            case 2: { // Local
                widgets::child(ICON_FA_SLIDERS, CXOR("Local ESP"));
                {
                    widgets::checkbox("Local Glow", &settings.cfg().visuals.b_local_glow);
                    if (settings.cfg().visuals.b_local_glow)
                    {
                        draw_color_edit("Local glow color", settings.cfg().visuals.glow_local_color);
                    }

                    widgets::checkbox("Aspect ratio", &settings.cfg().misc.b_aspect_ratio);
                    if (settings.cfg().misc.b_aspect_ratio)
                        ImGui::SliderFloat("Aspect ratio value", &settings.cfg().misc.aspect_ratio_value, 1.f, 3.f);
                    widgets::checkbox("Thirperson", &settings.cfg().misc.b_third_person);
                    if (settings.cfg().misc.b_third_person)
                    {
                        widgets::keybind("Thirperson key", &settings.cfg().misc.i_key_thirdperson, &settings.cfg().misc.i_style_thirdperson);
                        widgets::slider_int("Thirperson distance", &settings.cfg().misc.thirdperson_dist, 30, 150);
                    }
                    widgets::slider_float("World Field Of View", &settings.cfg().misc.p_fov, 70, 130);
                    widgets::slider_float("Viewmodel FOV", &settings.cfg().misc.flviewmodelfov, 70, 130);
                    widgets::slider_float("Hand X", &settings.cfg().misc.hand_X, -10, 10);
                    widgets::slider_float("Hand Y", &settings.cfg().misc.hand_Y, -10, 10);
                    widgets::slider_float("Hand Z", &settings.cfg().misc.hand_Z, -10, 10);

                    widgets::checkbox("Scope fov", &settings.cfg().visuals.b_scope_fov);
                    if (settings.cfg().visuals.b_scope_fov)
                    {
                        widgets::slider_int("Scope fov zoom 1", &settings.cfg().visuals.zoom_1_override, 0, 80);
                        widgets::slider_int("Scope fov zoom 2", &settings.cfg().visuals.zoom_2_override, 0, 80);
                    }

                }
                ImGui::EndChild();
            } break;
            case 3: { // Emotes
                widgets::child(ICON_FA_SLIDERS, CXOR("Emotes"));
                emotes::draw_menu();
                ImGui::EndChild();
            } break;
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine(0, 12);
        ImGui::BeginGroup();
        {
            switch (MENU::visuals_subtab) {
            case 0: { // Enemy
                widgets::child(ICON_FA_EYE, CXOR("Enemy Chams"));
                {
                    widgets::checkbox("Enemy chams settings", &settings.cfg().visuals.b_chams);
                    if (settings.cfg().visuals.b_chams) {
                        draw_color_edit("Enemy chams color", settings.cfg().visuals.chams_visible);
                        widgets::combo("Enemy chams material", &settings.cfg().visuals.chams_style, chamsStyle, IM_ARRAYSIZE(chamsStyle));
                        widgets::checkbox("Enemy chams invisible", &settings.cfg().visuals.b_chams_invisible);
                        if (settings.cfg().visuals.b_chams_invisible) {
                            draw_color_edit("Chams invisible color", settings.cfg().visuals.chams_invisible);
                            widgets::combo("Chams invisible material", &settings.cfg().visuals.chams_style_invisible, chamsStyle, IM_ARRAYSIZE(chamsStyle));
                        }
                    }
                }
                ImGui::EndChild();
            } break;
            case 1: { // Team
                widgets::child(ICON_FA_USERS, CXOR("Team Chams"));
                {
                    ImGui::Text("Later...");

                }
                ImGui::EndChild();
            } break;
            case 2: { // Local
                widgets::child(ICON_FA_USER_SHIELD, CXOR("Local Chams"));
                {
                    widgets::checkbox("Hand chams settings", &settings.cfg().visuals.b_chams_hand);
                    if (settings.cfg().visuals.b_chams_hand) {
                        draw_color_edit("Hand color", settings.cfg().visuals.chams_hand_visible);
                            widgets::combo("Hand visible material", &settings.cfg().visuals.chams_style_hand, chamsStyle, IM_ARRAYSIZE(chamsStyle));
                            widgets::checkbox("Hand invisible", &settings.cfg().visuals.b_chams_hand_invisible);
                            if (settings.cfg().visuals.b_chams_hand_invisible)
                            {
                                draw_color_edit("Hand invisible color", settings.cfg().visuals.chams_hand_invisible);
                                widgets::combo("Hand invisible material", &settings.cfg().visuals.chams_style_hand_invisible, chamsStyle, IM_ARRAYSIZE(chamsStyle));
                            }
                    }

                    widgets::checkbox("Attachment chams settings", &settings.cfg().visuals.b_chams_weapons);
                    if (settings.cfg().visuals.b_chams_weapons)
                    {
                        draw_color_edit("Attachment color", settings.cfg().visuals.chams_weapon_visible);
                        widgets::combo("Attachment material", &settings.cfg().visuals.chams_style_weapons, chamsStyle, IM_ARRAYSIZE(chamsStyle));

                        widgets::checkbox("Attachment invisible", &settings.cfg().visuals.b_chams_weapons_invisible);
                        if (settings.cfg().visuals.b_chams_weapons_invisible)
                        {
                           draw_color_edit("Attachment invisible color", settings.cfg().visuals.chams_weapon_invisible);
                           widgets::combo("Attachment invisible material", &settings.cfg().visuals.chams_style_weapons_invisible, chamsStyle, IM_ARRAYSIZE(chamsStyle));
                        }
                    }

                }
                ImGui::EndChild();
            } break;
            case 3: { // Emotes
                widgets::child(ICON_FA_MUSIC, CXOR("Emote pack"));
                ImGui::TextWrapped("Emotes use the existing render and frame lifecycle.");
                ImGui::TextDisabled("No second DLL hooks are installed.");
                ImGui::EndChild();
            } break;
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();

}

void tab_world()
{
    const char* world_items[] = { ("General"), ("Custom sky")};
    render_subtabs(world_items, &MENU::world_subtab, IM_ARRAYSIZE(world_items));

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##World_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {


        ImGui::BeginGroup();
        {
            switch (MENU::world_subtab) {
            case 0: {
                widgets::child(ICON_FA_SUN, CXOR("Modulation"));
                {

                    widgets::checkbox("World", &settings.cfg().worldModulation.b_worldModulation);
                    if (settings.cfg().worldModulation.b_worldModulation)
                    {
                        draw_color_edit("World color", settings.cfg().worldModulation.worldColor);
                        widgets::checkbox("World Particles", &settings.cfg().worldModulation.b_ParticlesEffect);
                        if (settings.cfg().worldModulation.b_ParticlesEffect)
                            draw_color_edit("Particles color", settings.cfg().worldModulation.particleColor);
                        draw_color_edit("World Cloud and Sun color", settings.cfg().worldModulation.cloudColor);
                        draw_color_edit("World Sky color", settings.cfg().worldModulation.skyColor);
                        widgets::slider_float("World Exposure", &settings.cfg().worldModulation.fl_exposure, 0, 100);
                        widgets::checkbox("World World blur", &settings.cfg().worldModulation.b_world_blur);
                        if (settings.cfg().worldModulation.b_world_blur)
                        {
                            widgets::checkbox("World blur Dynamic blur", &settings.cfg().worldModulation.b_dynamic_blur);
                        }
                    }
                }
                ImGui::EndChild();
            } break;
            case 1: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox("Sky materials changer", &settings.cfg().visuals.b_customsky);
                    if (ImGui::Button(CXOR("Refresh"), ImVec2(ImGui::GetWindowWidth() - 25, 25)))
                        g_customsky->UpdateSkyboxes();
                }
                ImGui::EndChild();
            }
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine(0, 12);
        ImGui::BeginGroup();
        {
            switch (MENU::world_subtab) {
            case 0: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Other world visuals"));
                {
                    widgets::checkbox("Dropped weapons", &settings.cfg().visuals.b_weapon_in_world);

                    widgets::checkbox("Bullet tracer", &settings.cfg().worldModulation.b_bullet_trace);
                    if (settings.cfg().worldModulation.b_bullet_trace)
                        draw_color_edit("Bullet tracer color", settings.cfg().worldModulation.bullet_color);

                    widgets::checkbox("Weapons glow", &settings.cfg().visuals.b_weapons_glow);
                    if (settings.cfg().visuals.b_weapons_glow)
                        draw_color_edit("Weapons glow Weapons dropped color", settings.cfg().visuals.glow_weapons_color);

                    widgets::checkbox("Dropped bomb esp", &settings.cfg().visuals.b_bomb_esp);

                    widgets::checkbox("Hitmarker", &settings.cfg().visuals.b_hitmarker);

                    widgets::checkbox("Map particles", &settings.cfg().visuals.m_mapeffect);
                    if (settings.cfg().visuals.m_mapeffect)
                    {
                        const char* particlesmap[] = { "Snow","Ash","Rain", "Star"};
                        widgets::combo("Particles type", &settings.cfg().visuals.i_mapeffect, particlesmap, IM_ARRAYSIZE(particlesmap));
                        draw_color_edit("Particles color", settings.cfg().visuals.c_mapeffect);
                    }

                    widgets::checkbox("Hit particles", &settings.cfg().visuals.m_hiteffect);
                    if (settings.cfg().visuals.m_hiteffect)
                    {
                        const char* hit_effect[] = { "Lightning","Skull","Coins","Fireworks" };
                        widgets::combo("Hit effect", &settings.cfg().visuals.i_hiteffectpart, hit_effect, IM_ARRAYSIZE(hit_effect));
                    }

                    widgets::checkbox("Kill particles", &settings.cfg().visuals.m_killeffect);
                    if (settings.cfg().visuals.m_killeffect)
                    {
                        const char* kill_effect[] = { "Lightning","Skull","Coins","Flower","Tombstone" };
                        widgets::combo("Kill effect", &settings.cfg().visuals.i_killeffectpart, kill_effect, IM_ARRAYSIZE(kill_effect));
                    }
                    
                    

                
                }
                ImGui::EndChild();
            } break;
            case 1: {

                widgets::child(ICON_FA_SORT, CXOR("Materials list"));
                {
                    for (size_t i = 0U; i < g_customsky->vecSkyboxes.size(); i++)
                    {
                        Skybox_t pSkybox = g_customsky->vecSkyboxes[i];

                            if (widgets::selectable(pSkybox.strSkyboxName.c_str(), (g_customsky->nSelectedSkybox == i)))
                            g_customsky->nSelectedSkybox = i;
                    }
                }
                ImGui::EndChild();
            }
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}

//void tab_skins()
//{
//    ImGui::SetCursorPos(BeginChildPos);
//    {
//
//        ImGui::BeginChild("Inventory Changer", ImVec2(700, 382), true);
//        {
//            ImGui::PushFont(g_render->font_tab);
//            switch (inventory_tab) {
//            case 0: { // Items List
//                ImGui::BeginChild("TopPanel##Items", ImVec2(0, 90), false);
//
//                if (ImGui::Button("Add Item##MainButton", ImVec2(80, 80))) { // ���������� ID
//                    inventory_tab = 1;
//                }
//                ImGui::EndChild();
//
//                ImGui::BeginChild("ItemsGrid##Scroll", ImVec2(0, 0), true);
//                const int columns = 5;
//                ImGui::Columns(columns, "##items_grid", false);
//
//                const float item_width = (ImGui::GetContentRegionAvail().x -
//                    (columns - 1) * ImGui::GetStyle().ItemSpacing.x) / columns;
//
//                for (size_t i = 0; i < g_skins->skin_added.size(); i++) {
//                    auto& skin = g_skins->skin_added[i];
//                    ImGui::PushID(static_cast<int>(i)); // ����� ���������� ����
//
//                    // ������ ������ ���������
//                    ImGui::BeginGroup();
//
//                    // ������ � ���������� ���������������
//                    const std::string btn_id = "##ItemBtn" + std::to_string(i);
//                    if (skin.texture_id != 0) {
//                        ImGui::ImageButton(btn_id.c_str(), (ImTextureID)skin.texture_id,
//                            ImVec2(item_width, item_width));
//                    }
//                    else {
//                        ImGui::Button("Loading...", ImVec2(item_width, item_width));
//                    }
//
//                    // ��������� ����� � ����������� ID
//                    ImGui::PushID("Name");
//                    ImGui::TextWrapped("%s", skin.name.c_str());
//                    ImGui::PopID();
//
//                    //ImGui::PushID("Paint");
//                    //ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", skin.paint_name.c_str());
//                    //ImGui::PopID();
//
//                    ImGui::EndGroup();
//
//                    // ����������� ����
//                    if (ImGui::BeginPopupContextItem("ItemContextMenu")) { // ���������� ID ����
//                        if (ImGui::MenuItem("Delete")) {
//                            CCSPlayerInventory* inv = CCSPlayerInventory::GetInstance();
//                            inv->RemoveEconItem(inv->GetSOCDataForItem(skin.id));
//                            g_skins->skin_added.erase(g_skins->skin_added.begin() + i);
//                        }
//                        ImGui::EndPopup();
//                    }
//
//                    ImGui::NextColumn();
//                    ImGui::PopID(); // ��� static_cast<int>(i)
//                }
//                ImGui::Columns(1);
//                ImGui::EndChild();
//                break;
//            }
//
//            case 1: { // Weapon List
//                ImGui::BeginChild("WeaponNav##Buttons", ImVec2(0, 90), false);
//                if (ImGui::Button("<- Back", ImVec2(80, 80))) {
//                    inventory_tab = 0;
//                }
//
//                ImGui::SameLine();
//                if (ImGui::Button("Knife", ImVec2(100, 100))) {
//                    inventory_tab = 3;
//                }
//
//                ImGui::SameLine();
//                if (ImGui::Button("Agent", ImVec2(100, 100))) {
//                    inventory_tab = 2;
//                }
//
//                ImGui::SameLine();
//                if (ImGui::Button("Glove", ImVec2(100, 100))) {
//                    inventory_tab = 7;
//                }
//                ImGui::EndChild();
//
//                // ����� ������
//                ImGui::BeginChild("WeaponsGrid##Scroll", ImVec2(0, 0), true);
//                ImGui::Columns(5, "##weapons_grid", false);
//                for (auto& skin : g_skins->item_default) {
//                    if (skin.path_image.empty())
//                        continue;
//
//                    if (ImGui::ImageButton(skin.path_image.c_str(), (ImTextureID)skin.texture_id, ImVec2(80, 80))) {
//                        skins_item_def = skin.item_def;
//                        inventory_tab = 4;
//                    }
//                    ImGui::TextWrapped("%s", skin.name.c_str());
//                    ImGui::NextColumn();
//                }
//                ImGui::Columns(1);
//                ImGui::EndChild();
//                break;
//            }
//
//            case 2: { // Agent List
//                ImGui::BeginChild("Agent List", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//                if (ImGui::Button("<- Back", ImVec2(100, 100))) {
//                    inventory_tab = 1;
//                }
//
//                ImGui::Columns(5, "##agents_grid", false);
//                for (auto& skin : g_skins->item_agent) {
//                    if (skin.path_image.empty())
//                        continue;
//
//                    if (ImGui::ImageButton(skin.path_image.c_str(), (ImTextureID)skin.texture_id, ImVec2(80, 80))) {
//                        CEconItem* item = CEconItem::CreateInstance();
//                        if (item) {
//                            CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
//                            auto highestIDs = pInventory->GetHighestIDs();
//                            item->m_ulID = highestIDs.first + 1;
//                            item->m_unAccountID = pInventory->get_owner().m_id;
//                            item->m_unDefIndex = skin.item_def;
//                            item->m_nRarity = skin.rarity;
//                            item->m_unInventory = highestIDs.second + 1;
//                            item->m_unDefIndex = skin.item_def;
//                            pInventory->AddEconItem(item);
//
//                            g_skins->skin_added.push_back({
//            .item_def = current_skin.item_def,
//            .paint_name = current_skin.paint_name,
//            .name = current_skin.name,
//            .skin_key = current_skin.skin_key,
//            .paint_id = current_skin.paint_id,
//            .rarity = current_skin.rarity,
//            .texture_id = current_skin.texture_id,
//            .type = current_skin.type,
//            .path_image = current_skin.path_image,
//            .is_initilization = true,
//            .paint_seed = paint_seed,
//            .paint_wear = paint_wear,
//            .stattrak = stattrack,
//            .stickers = current_skin.stickers,
//            .custom_name = current_skin.custom_name,
//            .id = item->m_ulID
//                                });
//                            inventory_tab = 0;
//                        }
//                    }
//                    ImGui::TextWrapped("%s", skin.name.c_str());
//                    ImGui::NextColumn();
//                }
//                ImGui::Columns(1);
//                ImGui::EndChild();
//                break;
//            }
//
//            case 5: { // Skin Settings
//                ImGui::Begin("Skin Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindow _NoCollapse);
//
//                // Preview
//                ImGui::Image((ImTextureID)current_skin.texture_id, ImVec2(256, 192));
//
//                // ���������
//                if (current_skin.type != type_glove) {
//                    ImGui::SliderInt("StatTrak", &stattrack, 0, 1337);
//                    ImGui::SliderInt("Paint Seed", &paint_seed, 0, 1000);
//                }
//                ImGui::SliderInt("Wear", &paint_wear, 0.0f, 1.0f, "%.3f");
//
//                // �������
//                ImGui::Separator();
//                ImGui::Text("Stickers:");
//                for (int i = 0; i < 5; i++) {
//                    if (current_stickers[i].path_image.empty())
//                        continue;
//                    ImGui::PushID(i);
//                    if (current_stickers[i].texture_id != 0) {
//                        if (ImGui::ImageButton(current_stickers[i].path_image.c_str(), (ImTextureID)current_stickers[i].texture_id, ImVec2(40, 40))) {
//                            current_sticker = i;
//                            inventory_tab = 6;
//                        }
//                    }
//                    else {
//                        if (ImGui::Button("+", ImVec2(40, 40))) {
//                            current_sticker = i;
//                            inventory_tab = 6;
//                        }
//                    }
//                    ImGui::SameLine();
//                    ImGui::PopID();
//                }
//
//                if (ImGui::Button("Add to Inventory", ImVec2(150, 30))) {
//                    CEconItem* item = CEconItem::CreateInstance();
//                    if (item) {
//                        CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
//                        auto highestIDs = pInventory->GetHighestIDs();
//
//                        // ������� ���������
//                        item->m_ulID = highestIDs.first + 1;
//                        item->m_unAccountID = pInventory->get_owner().m_id;
//                        item->m_unDefIndex = current_skin.item_def;
//                        item->m_nRarity = current_skin.rarity;
//
//                        // ��������� �����
//                        item->SetPaintKit(current_skin.paint_id);
//                        item->SetPaintSeed(paint_seed);
//                        item->SetPaintWear(paint_wear / 100.0f);
//                        item->SetStatTrak(stattrack);
//
//                        pInventory->AddEconItem(item);
//
//                        // ��������� � ��������� ������
//                        g_skins->skin_added.push_back({
//                            .item_def = current_skin.item_def,
//                            .paint_name = current_skin.paint_name,
//                            .name = current_skin.name,
//                            .skin_key = current_skin.skin_key,
//                            .paint_id = current_skin.paint_id,
//                            .rarity = current_skin.rarity,
//                            .texture_id = current_skin.texture_id,
//                            .type = current_skin.type,
//                            .path_image = current_skin.path_image,
//                            .is_initilization = true,
//                            .paint_seed = paint_seed,
//                            .paint_wear = paint_wear,
//                            .stattrak = stattrack,
//                            .stickers = current_skin.stickers,
//                            .custom_name = current_skin.custom_name,
//                            .id = item->m_ulID
//                            });
//
//                        // ���������� ���������
//                        inventory_tab = 0;
//                        skins_item_def = -1;
//                        paint_wear = 0;
//                        paint_seed = 0;
//                        stattrack = 0;
//                    }
//
//                    ImGui::SameLine();
//                    if (ImGui::Button("Cancel", ImVec2(150, 30))) {
//                        inventory_tab = 0;
//                    }
//                }
//                ImGui::End();
//                break;
//            }
//
//            case 3: { // Knife Selection
//                ImGui::BeginChild("Knife List", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//                if (ImGui::Button("<- Back", ImVec2(100, 100))) {
//                    inventory_tab = 1;
//                    skins_item_def = -1;
//                }
//
//                ImGui::Columns(5, "##knives_grid", false);
//                for (auto& skin : g_skins->item_knife_default) {
//                    if (skin.path_image.empty())
//                        continue;
//
//                    if (ImGui::ImageButton(skin.path_image.c_str(), (ImTextureID)skin.texture_id, ImVec2(80, 80))) {
//                        skins_item_def = skin.item_def;
//                        inventory_tab = 3; // �������� �� ���� ������� ��� ������ �����
//                    }
//                    ImGui::TextWrapped("%s", skin.name.c_str());
//                    ImGui::NextColumn();
//                }
//                ImGui::Columns(1);
//
//                // ���� ������ ���������� ��� - ���������� ��� �����
//                if (skins_item_def != -1) {
//                    ImGui::Separator();
//                    auto& skins = *g_skins;
//                    auto it = std::find_if(skins.item_knife_default.begin(), skins.item_knife_default.end(),
//                        [&](const auto& x) { return x.item_def == skins_item_def; });
//
//                    if (it != skins.item_knife_default.end()) {
//                        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Select Skin for %s:", it->name.c_str());
//                    }
//
//                    ImGui::Columns(5, "##knife_skins_grid", false);
//                    for (auto& skin : g_skins->item_weapons[skins_item_def]) {
//                        if (skin.skin_key != get_alternate_icon_key_for_weapon_paint_wear_item(skin.item_def, skin.paint_id, 0))
//                            continue;
//
//                        if (skin.path_image.empty())
//                            continue;
//
//                        if (ImGui::ImageButton(skin.path_image.c_str(), (ImTextureID)skin.texture_id, ImVec2(80, 80))) {
//                            current_skin = skin;
//                            inventory_tab = 5; // ��������� � ����������
//                        }
//                        ImGui::TextWrapped("%s", skin.paint_name.c_str());
//                        ImGui::NextColumn();
//                    }
//                    ImGui::Columns(1);
//                }
//                ImGui::EndChild();
//                break;
//            }
//
//            case 4: { // Weapon Skins
//                ImGui::BeginChild("Weapon Skins", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//                if (ImGui::Button("<- Back", ImVec2(100, 100))) {
//                    inventory_tab = 1;
//                    skins_item_def = -1;
//                }
//
//                if (skins_item_def != -1) {
//                    ImGui::SameLine();
//                    auto& skins = *g_skins;
//
//                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Skins for: %s",
//                        std::find_if(skins.item_default.begin(), skins.item_default.end(),
//                            [&](auto x) { return x.item_def == skins_item_def; })->name.c_str());
//
//                    ImGui::Columns(5, "##weapon_skins_grid", false);
//                    for (auto& skin : g_skins->item_weapons[skins_item_def]) {
//                        if (skin.skin_key != get_alternate_icon_key_for_weapon_paint_wear_item(skin.item_def, skin.paint_id, 0))
//                            continue;
//
//                        if (skin.path_image.empty())
//                            continue;
//
//                        if (ImGui::ImageButton(skin.path_image.c_str(), (ImTextureID)skin.texture_id, ImVec2(80, 80))) {
//                            current_skin = skin;
//                            inventory_tab = 5; // ��������� � ����������
//                        }
//                        ImGui::TextWrapped("%s", skin.paint_name.c_str());
//                        ImGui::NextColumn();
//                    }
//                    ImGui::Columns(1);
//                }
//                ImGui::EndChild();
//                break;
//            }
//
//            case 6: { // Stickers
//                ImGui::BeginChild("Stickers", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//                if (ImGui::Button("<- Back", ImVec2(100, 100))) {
//                    inventory_tab = 5;
//                }
//
//                ImGui::Columns(5, "##stickers_grid", false);
//                for (auto& sticker : g_skins->item_stickers) {
//                    if (sticker.type != type_item::type_sticker) continue;
//                    if (sticker.paint_name.size() == 0)
//                        continue;
//                    if (ImGui::ImageButton(sticker.paint_name.c_str(), (ImTextureID)sticker.texture_id, ImVec2(80, 80))) {
//                        current_stickers[current_sticker] = sticker;
//                        inventory_tab = 5; // ������������ � ���������� �����
//                    }
//                    ImGui::TextWrapped("%s", sticker.name.c_str());
//                    ImGui::NextColumn();
//                }
//                ImGui::Columns(1);
//                ImGui::EndChild();
//                break;
//            }
//
//            case 7: { // Gloves
//                ImGui::BeginChild("Gloves", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//                if (ImGui::Button("<- Back", ImVec2(100, 100))) {
//                    inventory_tab = 1;
//                }
//
//                ImGui::Columns(5, "##gloves_grid", false);
//                for (auto& glove : g_skins->item_glove) {
//                    if (glove.paint_name.size() == 0)
//                        continue;
//                    if (ImGui::ImageButton(glove.paint_name.c_str(), (ImTextureID)glove.texture_id, ImVec2(80, 80))) {
//                        current_skin = glove;
//                        inventory_tab = 5; // ��������� � ����������
//                    }
//                    ImGui::TextWrapped("%s", glove.paint_name.c_str());
//                    ImGui::NextColumn();
//                }
//                ImGui::Columns(1);
//                ImGui::EndChild();
//                break;
//            }
//
//            default:
//                inventory_tab = 0;
//                break;
//            }
//            ImGui::PopFont();
//        }
//        ImGui::EndChild();
//
//    }
//}

void tab_main()
{
    const char* misc_items[] = { ("General"), ("Models"), ("Knife models")};
    render_subtabs(misc_items, &MENU::misc_subtab, IM_ARRAYSIZE(misc_items));

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##Misc_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {

        ImGui::BeginGroup();
        {
            switch (MENU::misc_subtab) {
            case 0: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox("Hotkey list", &settings.cfg().visuals.b_render_binds);

                    /*ImGui::SliderFloat("pich", &settings.cfg().misc.rotPitch, -10, 10);
                    ImGui::SliderFloat("yaw", &settings.cfg().misc.rotYaw, -10, 10);
                    ImGui::SliderFloat("roll", &settings.cfg().misc.rotRoll, -10, 10);*/

                    widgets::checkbox("Bunnyhope", &settings.cfg().movement.b_bunnyhope);
                    widgets::checkbox("Autostrafer", &settings.cfg().movement.b_autostrafe);
                    widgets::checkbox("Autopeek", &settings.cfg().movement.b_auto_pick);
                    if (settings.cfg().movement.b_auto_pick)
                    {
                        widgets::keybind("Autopeek key", &settings.cfg().misc.i_key_autopeek, &settings.cfg().misc.i_style_autopeek);
                    }
                    widgets::checkbox("Auto accept", &settings.cfg().misc.auto_accept);
                    widgets::checkbox("Spectators", &settings.cfg().visuals.b_spectators);
                    widgets::multicombo("Removals", settings.cfg().visuals.m_removals.data(), flags_removals, IM_ARRAYSIZE(flags_removals));
                    ImGui::Spacing();
                    ImGui::TextDisabled("Camera");
                    widgets::checkbox("Thirdperson", &settings.cfg().misc.b_third_person);
                    if (settings.cfg().misc.b_third_person)
                    {
                        widgets::keybind("Thirdperson key", &settings.cfg().misc.i_key_thirdperson, &settings.cfg().misc.i_style_thirdperson);
                        widgets::slider_int("Thirdperson distance", &settings.cfg().misc.thirdperson_dist, 30, 150);
                    }
                    ImGui::Spacing();
                    ImGui::TextDisabled("Match tools");
                    if (ImGui::Button("Cancel match", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f)))
                        invalid_steam_logon::start();
                    if (ImGui::BeginPopupModal("CancelMatchInfo", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::TextWrapped("Match tool started.");
                        if (ImGui::Button("OK", ImVec2(90, 0))) ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                }
                ImGui::EndChild();
            } break;

            case 1: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox(CXOR("Model changer"), &settings.cfg().visuals.b_modelchanger);

                    if (ImGui::Button(CXOR("Refresh"), ImVec2(ImGui::GetWindowWidth() - 25, 25)))
                        g_modelchanger->UpdatePlayerModels();
                }
                ImGui::EndChild();
            } break;
            case 2: {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    widgets::checkbox(CXOR("Knife changer"), &settings.cfg().visuals.b_knifechanger);

                    if (ImGui::Button(CXOR("Refresh"), ImVec2(ImGui::GetWindowWidth() - 25, 25)))
                        g_modelchanger->UpdateWeaponModels();
                }
                ImGui::EndChild();
            }
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine(0, 12);
        ImGui::BeginGroup();
        {
            switch (MENU::misc_subtab) {
            case 0: {
                widgets::child(ICON_FA_USERS_VIEWFINDER, CXOR("Other"));
                {
                    ImGui::Text("Later...");
                }
                ImGui::EndChild();
            } break;
            case 1: {
                widgets::child(ICON_FA_SORT, CXOR("Models list"));
                {
                    for (size_t i = 0U; i < g_modelchanger->vecPlayerModels.size(); i++)
                    {
                        Model_t pModel = g_modelchanger->vecPlayerModels[i];

                        if (widgets::selectable(pModel.strModelName.c_str(), (g_modelchanger->nSelectedPlayerModel == i)))
                            g_modelchanger->nSelectedPlayerModel = i;
                    }
                }
                ImGui::EndChild();
            } break;
            case 2: {
                widgets::child(ICON_FA_SORT, CXOR("Models list"));
                {
                    for (size_t i = 0U; i < g_modelchanger->vecWeaponModels.size(); i++)
                    {
                        Model_t pModel = g_modelchanger->vecWeaponModels[i];

                        if (widgets::selectable(pModel.strModelName.c_str(), (g_modelchanger->nSelectedWeaponModel == i)))
                            g_modelchanger->nSelectedWeaponModel = i;
                    }
                }
                ImGui::EndChild();
            }
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}

void tab_cloud()
{
    const char* cloud_items[] = { ("Configs"), ("Scripts") };
    render_subtabs(cloud_items, &MENU::cloud_subtab, IM_ARRAYSIZE(cloud_items));

    static char cfg_name[64]{};
    static std::string selected_cfg;

    ImGui::SetCursorPos(BeginChildPos);
    ImGui::BeginChild(CXOR("##Cloud_tab"), BeginChildSize, true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::BeginGroup();
        {
            switch (MENU::cloud_subtab)
            {
            case 0:
            {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                {
                    const float button_width = ImGui::GetContentRegionAvail().x;
                    const float button_height = 24.0f;

                    ImGui::SetNextItemWidth(button_width);
                    ImGui::InputText("##cfg_name", cfg_name, sizeof(cfg_name));
                    selected_cfg = std::string(cfg_name);

                    if (ImGui::Button("Create", ImVec2(button_width, button_height)) && !selected_cfg.empty())
                    {
                        settings.save(selected_cfg.c_str());
                        settings.init_all_cfg();  // Обновляем список после сохранения
                    }


                    if (ImGui::Button("Load", ImVec2(button_width, button_height)) && selected_cfg.length() > 0)
                    {
                        settings.load(selected_cfg.c_str());
                        try {
                            settings.LoadEconItems("negr");
                        }
                        catch (...) {
                            std::cout << "LoadEconItems" << std::endl;
                        }
                    }

                    if (ImGui::Button("Save", ImVec2(button_width, button_height)) && selected_cfg.length() > 0)
                    {
                        settings.save(selected_cfg.c_str());
                        settings.init_all_cfg();
                    }

                    if (ImGui::Button("Refresh", ImVec2(button_width, button_height)))
                    {
                        settings.init_all_cfg();
                    }

                    if (ImGui::Button("Open Folder", ImVec2(button_width, button_height)))
                    {
                        ShellExecute(NULL, "open", settings.GetConfigPath().c_str(), NULL, NULL, SW_SHOW);
                    }
                }
                ImGui::EndChild();
                break;
            }

            case 1:
            {
                widgets::child(ICON_FA_SLIDERS, CXOR("Main"));
                ImGui::Text("Later...");
                ImGui::EndChild();
                break;
            }
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, 12);
        ImGui::BeginGroup();
        {
            switch (MENU::cloud_subtab)
            {
            case 0:
            {
                widgets::child(ICON_FA_SERVER, CXOR("List"));
                {
                    for (const auto& cfg : settings.get_all_configs())
                    {
                        if (widgets::selectable(cfg.c_str(), cfg == selected_cfg, ImGuiSelectableFlags_AllowDoubleClick))
                        {
                            selected_cfg = cfg;
                            strncpy_s(cfg_name, sizeof(cfg_name), cfg.c_str(), _TRUNCATE);
                        }
                    }
                }
                ImGui::EndChild();
                break;
            }

            case 1:
            {
                widgets::child(ICON_FA_SERVER, CXOR("List"));
                ImGui::Text("null");
                ImGui::EndChild();
                break;
            }
            }
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}


void navbar()
{
    const float navbar_width = 166.f;
    const float button_width = 150.f;

    ImGui::BeginChild(CXOR("##Navbar"), ImVec2(navbar_width, wsize.y - 48.f), false,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetCursorPos(ImVec2(8, 8));   if (widgets::tab(ICON_FA_SKULL, CXOR("Rage"), tab == 0, button_width)) tab = 0;
        ImGui::SetCursorPos(ImVec2(8, 56));  if (widgets::tab(ICON_FA_GUN, CXOR("Legit"), tab == 1, button_width)) tab = 1;
        ImGui::SetCursorPos(ImVec2(8, 104)); if (widgets::tab(ICON_FA_USER, CXOR("Visuals"), tab == 2, button_width)) tab = 2;
        ImGui::SetCursorPos(ImVec2(8, 152)); if (widgets::tab(ICON_FA_SUN, CXOR("World"), tab == 6, button_width)) tab = 6;
        ImGui::SetCursorPos(ImVec2(8, 200)); if (widgets::tab(ICON_FA_CUBE, CXOR("Utilities"), tab == 4, button_width)) tab = 4;
        ImGui::SetCursorPos(ImVec2(8, 248)); if (widgets::tab(ICON_FA_CLOUD, CXOR("Profiles"), tab == 5, button_width)) tab = 5;
    }
    ImGui::EndChild();
}
void menu::render_menu()
{
	//g_hook->last_func = "render_menu";

    if (!bMainWindowOpened) return;
    constexpr float menu_alpha = 1.f;

    // click effect
    static std::vector<ClickData> clicks;

    auto im_io = ImGui::GetIO();
    auto s_size = im_io.DisplaySize;

    if (im_io.MouseClicked[0])
        clicks.push_back({ ImGui::GetMousePos() });

    for (size_t i = 0; i < clicks.size(); ++i)
    {
        clicks[i].value = ImClamp(clicks[i].value + 0.1f, 0.f, 1.f);

        ImGui::GetForegroundDrawList()->AddCircleFilled(
            clicks[i].pos,
            10 * clicks[i].value,
            ImGui::GetColorU32(ImGuiCol_FrameBg, (1 - clicks[i].value) * menu_alpha)
        );

        if (clicks[i].value == 1.f)
            clicks.erase(clicks.begin() + i);
    }


    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 12));
    ImGui::SetNextWindowSize(wsize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2((s_size.x - wsize.x) / 2.f, (s_size.y - wsize.y) / 2.f), ImGuiCond_FirstUseEver);
    ImGui::Begin(("##Menu"), NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
    {
        ImVec2 p = ImGui::GetWindowPos();
        ImDrawList* d = ImGui::GetWindowDrawList();

        load_menu_mascot();
        LiquidGlass::Params glass{};
        glass.exponent = 8.0f; glass.blur = 3.0f;
        glass.aberration = 1.5f; glass.tint = 0.92f; glass.rim = 0.12f;
        LiquidGlass::Draw(d, ImVec2(p.x + wsize.x * .5f, p.y + wsize.y * .5f),
            ImVec2(wsize.x * .5f, wsize.y * .5f), glass, IM_COL32(255,255,255,235));

        if (auto* background = current_frame(g_menu_background))
        {
            d->AddImageRounded((ImTextureID)background, p, p + wsize,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 179), 14.f);
        }

        auto add_alpha = [](ImColor col, float alpha) {
            return ImColor(col.Value.x, col.Value.y, col.Value.z, col.Value.w * alpha);
            };

        d->AddText(ImVec2(p.x + 18, p.y + wsize.y - 22),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.6f * menu_alpha)), ("STRWM"));

        ImGui::SetCursorPos(ImVec2(wsize.x - 178.f, 12.f));
        navbar();

        ImGui::SetCursorPos(ImVec2(12, 12));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::BeginGroup();
        switch (tab)
        {
        case 0: tab_ragebot(); break;
        case 1: tab_legitbot(); break;
        case 2: tab_visuals(); break;
        case 4: tab_main(); break;
        case 5: tab_cloud(); break;
        case 6: tab_world(); break;
        }
        ImGui::EndGroup();
        ImGui::PopStyleVar(2);
    }
    ImGui::End();

    if (auto* mascot = current_frame(g_menu_mascot))
    {
        const ImVec2 mascot_size(96.f, 96.f);
        const ImVec2 mascot_pos(ImGui::GetWindowPos().x - mascot_size.x + 28.f,
            ImGui::GetWindowPos().y - 24.f);
        ImGui::GetForegroundDrawList()->AddImageRounded((ImTextureID)mascot, mascot_pos,
            mascot_pos + mascot_size, ImVec2(0, 0), ImVec2(1, 1),
            IM_COL32(255, 255, 255, 255), 10.f);
    }
    ImGui::PopStyleVar(3);
}
