#pragma once
#include <memory>
#include <wtypes.h>
#include <d3d11.h>
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"
#include "../dependencies/imgui/imgui_impl_dx11.h"
#include "../sdk/math/matrix.h"	
#include "../sdk/game_class/color.h"
#include "../dependencies/vtex_parser/vtex.h" 

#include <chrono>
#include "../dependencies/vtex_parser/vtex_parser.h"
class ID3D11ShaderResourceView;

inline bool bMainWindowOpened = false;
inline bool bMainActive = false;
inline v_matrix m_view_matrix{};
inline bool bInitialized = false;

enum e_draw_flags
{
	drop_shadow,
	outline,
	centered,
	shadows,
	center_outline
};

class c_render
{
private:
	static void setup_inputSystem();
	static int get_correct_dxgi_format(int format);

	inline static bool m_should_dt;
	inline static int index_id;
	inline static float start_time;
	inline static float cycle_time;
	inline static int dt_shoots;
public:
	static inline ImDrawList* m_draw_list;
	static inline ImDrawList* esp_drawlist;
	static void create_render_target(IDXGISwapChain* swap_chain);
	static bool init();
	void text(const vec2_t& txt_pos, Color_t color, const int flags, ImFont* font, const std::string_view& message, const int font_size) const;
	bool OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void spectator(const char* username, ImTextureID iconTexture, float offset);
	bool LoadAndRenderImage(ID3D11Device* device, VTexParser::ImageData filepath, ID3D11ShaderResourceView** textureViewOut);
	ID3D11ShaderResourceView* load_texture_from_buffer(const uint8_t* image_data, int image_width, int image_height);
	void rect_filled_multi_color(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImColor clr_top_left, ImColor clr_top_right, ImColor clr_bottom_left, ImColor clr_bottom_right);
	static void counter_doubletap();
	static void render_imgui_dx11(IDXGISwapChain* swap_chain);
	static bool world_to_screen(const vec3_t& in, vec2_t& out);
	static void autopick_cricle();
	static void render_dmg(ImGuiIO& io);
	static bool circle_filled_3d(vec3_t& origin, float radius, ImColor color, float thickness);
	static inline ImFont* font_visitor, *font_chika, *font_weapons, *font_big_chika,*font_tab;
};

inline auto g_render = std::make_unique<c_render>();

struct c_fonts
{
public:
	ImFont* verdana;
	ImFont* verdana_small;
	ImFont* onetap_pixel;
	ImFont* tab;
	ImFont* main;
	ImFont* pixelfont;
	ImFont* fasolid;
	ImFont* opensans;
	ImFont* arialbd;
	ImFont* pEspName;
	ImFont* pEspFlagsName;
	ImFont* pEspWepName;
	ImFont* pEspHealth;

	ImFont* uansdasd;
};

inline const auto g_fonts = std::make_unique<c_fonts>();