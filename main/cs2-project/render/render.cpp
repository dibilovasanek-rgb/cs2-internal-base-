#include "render.h"
#include "../core/menu/menu.h"
#include "../sdk/input_system/inputsystem.h"
#include "../core/interfaces/interface.h"
#include "../memory/memory.h"
#include "../sdk/entity_listener/entity_list.h"
#include "../sdk/math/vector.h"
#include "../sdk/schema/class/player.h"
#include "../features/visual/esp.h"
#include "../dependencies/font/VISITOR.h"
#include "../dependencies/font/Chika.h"
#include "../dependencies/font/weapons_icons.h"
#include "../dependencies/imgui/imgui_internal.h"
#include "../features/visual/game_visuals/world.h"
#include "../features/visual/additionally.h"
#include "../dependencies/imgui/imgui.h"
#include "../sdk/sdk.h"
#include "..\..\..\sdk\game_class\interfaces\i_global_vars.h"
#include "../features/movement/movement.h"
#include "../../../sdk/game_class/interfaces/engine_client.h"
#include "../features/rage/ragebot.h"
#include "../dependencies/font/MuseoSansCyrl-800.h"
#include <Windows.h>
#include "../core/hooks/hooks.h"
#include "../dependencies/vtex_parser/vtex_parser.h"
#include <wrl/client.h>
#include "../entry.h"
#include "../dependencies/config/Include/config.h"
#include "liquid_glass.h"

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	const auto isMainWindow = [handle]()
		{
			return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle) && handle != GetConsoleWindow();
		};

	DWORD pID = 0;
	GetWindowThreadProcessId(handle, &pID);

	if (GetCurrentProcessId() != pID || !isMainWindow())
		return TRUE;

	*reinterpret_cast<HWND*>(lParam) = handle;
	return FALSE;
}
#define CXOR(str) str

void c_render::setup_inputSystem()
{
	while (!g_hook->g_window)
	{
		EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&g_hook->g_window));
	}

	g_hook->g_wnd_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_hook->g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_hook->hk_wnd_proc)));
	if (g_hook->g_wnd_proc == nullptr)
		return;
}

static void* __cdecl ImGuiAllocWrapper(const std::size_t nSize, [[maybe_unused]] void* pUserData = nullptr)
{
	const HANDLE hHeap = ::GetProcessHeap();
	return ::HeapAlloc(hHeap, 0UL, nSize);
}

static void __cdecl ImGuiFreeWrapper(void* pMemory, [[maybe_unused]] void* pUserData = nullptr) noexcept
{
	if (pMemory != nullptr)
	{
		const HANDLE hHeap = ::GetProcessHeap();
		::HeapFree(hHeap, 0UL, pMemory);
	}
}

bool c_render::init()
{
	if (bInitialized)
		return true;

	setup_inputSystem(); // setup input system 

	ImGui::SetAllocatorFunctions(ImGuiAllocWrapper, ImGuiFreeWrapper);

	ImGui::CreateContext();

	// setup platform and renderer bindings
	if (!ImGui_ImplWin32_Init(g_hook->g_window))
		return false;

	if (!ImGui_ImplDX11_Init(g_hook->m_device, g_hook->m_context))
		return false;
	LiquidGlass::Init(g_hook->m_device, g_hook->m_context);

	auto& io = ImGui::GetIO();

	io.LogFilename = nullptr;
	io.IniFilename = nullptr;


	ImGuiStyle& style = ImGui::GetStyle();

	// Load fonts
	ImFontConfig cfg;
	cfg.PixelSnapH = true;
	cfg.OversampleH = 3;
	cfg.OversampleV = 1;
	cfg.FontDataOwnedByAtlas = false;
	cfg.MergeMode = true;

	{
		static const ImWchar icons_ranges[] = { 0xe005, 0xf6ff, 0 };

		io.Fonts->AddFontFromMemoryTTF(roboto, sizeof(roboto), 14.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
		io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 12.0f, &cfg, icons_ranges);
		io.Fonts->AddFontFromMemoryTTF(roboto, sizeof(roboto), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
		io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 16.0f, &cfg, icons_ranges);

		g_fonts->pEspName = io.Fonts->AddFontFromFileTTF(CXOR("C:\\Windows\\Fonts\\Verdana.ttf"), 11.5f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts->pEspHealth = io.Fonts->AddFontFromFileTTF(CXOR("C:\\Windows\\Fonts\\Verdana.ttf"), 11.5f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts->pEspWepName = io.Fonts->AddFontFromFileTTF(CXOR("C:\\Windows\\Fonts\\Verdana.ttf"), 11.5f, &cfg, io.Fonts->GetGlyphRangesDefault());
		g_fonts->pEspFlagsName = io.Fonts->AddFontFromFileTTF(CXOR("C:\\Windows\\Fonts\\Verdana.ttf"), 11.5f, &cfg, io.Fonts->GetGlyphRangesDefault());

		ImGui::StyleColorsDark();

		// new style
		auto& ss = ImGui::GetStyle();
		ss.WindowRounding = 14.f;
		ss.ChildRounding = 10.f;
		ss.PopupRounding = 10.f;
		ss.FramePadding = ImVec2(6, 3);
		ss.FrameRounding = 8.f;
		ss.FrameBorderSize = 1.f;
		ss.ScrollbarSize = 8.f;
		ss.ScrollbarRounding = 0.f;

		auto& c = ss.Colors;
		c[ImGuiCol_TextDisabled] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
		c[ImGuiCol_PopupBg] = ImColor(22, 22, 25, 210);
		c[ImGuiCol_ChildBg] = ImColor(18, 18, 21, 72);
		c[ImGuiCol_TitleBg] = ImColor(16, 16, 17);
		c[ImGuiCol_TitleBgActive] = ImColor(16, 16, 17);
		c[ImGuiCol_Border] = ImColor(27, 27, 32);
		c[ImGuiCol_Separator] = ImColor(27, 27, 30);
		c[ImGuiCol_SeparatorHovered] = ImColor(155, 150, 219);
		c[ImGuiCol_SeparatorActive] = ImColor(155, 150, 219);
		c[ImGuiCol_Button] = ImColor(35, 35, 40, 125);
		c[ImGuiCol_ButtonHovered] = ImColor(70, 70, 78, 155);
		c[ImGuiCol_ButtonActive] = ImColor(105, 105, 115, 180);
		c[ImGuiCol_FrameBg] = ImColor(34, 34, 39, 125);
		c[ImGuiCol_FrameBgHovered] = ImColor(65, 65, 72, 155);
		c[ImGuiCol_FrameBgActive] = ImColor(155, 150, 219);
		c[ImGuiCol_SliderGrab] = ImColor(155, 150, 219);
		c[ImGuiCol_SliderGrabActive] = ImColor(180, 175, 235);
		c[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		c[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		c[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.15f);
		c[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	}

	g_fonts->uansdasd = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 21.f, nullptr);
	g_fonts->verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	g_fonts->verdana_small = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	g_fonts->onetap_pixel = io.Fonts->AddFontFromMemoryTTF(onetap_pixel_font, sizeof(onetap_pixel_font), 8.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	g_fonts->tab = io.Fonts->AddFontFromMemoryTTF(MuseoSans, sizeof(MuseoSans), 18.0f), nullptr, io.Fonts->GetGlyphRangesCyrillic();
	g_fonts->main = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 20.0f), nullptr, io.Fonts->GetGlyphRangesCyrillic();
	g_fonts->opensans = io.Fonts->AddFontFromMemoryTTF(opensans, 150592, 16.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	//fonts.pixelfont = io.Fonts->AddFontFromMemoryTTF(pixelify, sizeof(pixelify), 11.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	ImFontConfig config;
	config.MergeMode = true;        // <<< обязательно слияние
	config.PixelSnapH = true;        // <<< для чёткости
	config.FontDataOwnedByAtlas = false;
	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 }; // Диапазон FontAwesome

	g_fonts->fasolid = io.Fonts->AddFontFromMemoryTTF(
		FontData,       // сюда твой указатель на данные ttf
		sizeof(FontData),       // сюда размер этих данных
		16.0f,                       // размер шрифта
		&config,
		icons_ranges
	);

	ImFontConfig icons_config;
	icons_config.FontDataOwnedByAtlas = false;
	font_visitor = io.Fonts->AddFontFromMemoryTTF(visitor, sizeof(visitor), 12, &icons_config, io.Fonts->GetGlyphRangesCyrillic());
	font_chika = io.Fonts->AddFontFromMemoryTTF(chika, sizeof(chika), 12, &icons_config, io.Fonts->GetGlyphRangesCyrillic());
	font_weapons = io.Fonts->AddFontFromMemoryTTF(weaponicons, sizeof(weaponicons), 12, &icons_config, io.Fonts->GetGlyphRangesCyrillic());
	font_big_chika = io.Fonts->AddFontFromMemoryTTF(chika, sizeof(chika), 17, &icons_config, io.Fonts->GetGlyphRangesCyrillic());
	font_tab = io.Fonts->AddFontFromMemoryTTF(MuseoSans, sizeof(MuseoSans), 17, &icons_config, io.Fonts->GetGlyphRangesCyrillic());

	// check loaded fonts
	//esp_drawlist = new ImDrawList(ImGui::GetDrawListSharedData());
	bInitialized = io.Fonts->Build();
	return bInitialized;
}

void c_render::text(const vec2_t& txt_pos, Color_t color, const int flags, ImFont* font, const std::string_view& message, const int font_size) const
{
	m_draw_list->PushTextureID(font->ContainerAtlas->TexID);

	const auto size = font->CalcTextSizeA(font_size, FLT_MAX, FLT_MAX, message.data());
	auto pos = ImVec2(txt_pos.x, txt_pos.y);
	auto outline_clr = Color_t(0, 0, 0, static_cast<int>(color.a * 0.12f));

	if (flags & centered)
		pos = ImVec2(txt_pos.x - size.x / 2.0f, txt_pos.y);

	if (flags & center_outline)
	{
		pos = ImVec2(txt_pos.x - size.x / 2.0f, txt_pos.y);
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y - 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y + 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y - 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x, pos.y), ImColor(0, 0, 0), message.data());


		/*m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y - 1), ImColor(0,0,0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y + 1), ImColor(0,0,0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y - 1), ImColor(0,0,0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), ImColor(0,0,0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x, pos.y + 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x, pos.y - 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y), ImColor(0, 0, 0), message.data());*/
	}

	if (flags & shadows)
	{
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y - 1), outline_clr.GetU32(), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), outline_clr.GetU32(), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x, pos.y + 1), outline_clr.GetU32(), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y), outline_clr.GetU32(), message.data());
	}

	if (flags & outline)
	{
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y - 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x - 1, pos.y + 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y - 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0), message.data());
		m_draw_list->AddText(font, font_size, ImVec2(pos.x, pos.y), ImColor(0, 0, 0), message.data());
	}

	m_draw_list->AddText(font, font_size, pos, color.GetU32(), message.data());
}

void CircleFilled3D(const vec3_t& pos, float radius, Color_t in, Color_t out)
{
	ImVec2 center; vec2_t g_pos;

	if (!g_render->world_to_screen(pos, g_pos))
		return;
	center = ImVec2(g_pos.x, g_pos.y);
	g_render->m_draw_list->_PathArcToFastEx(center, radius, 0, 48, 0);
	const int count = g_render->m_draw_list->_Path.Size - 1;
	float step = (3.141592654f * 2.0f) / (count + 1);
	std::vector<ImVec2> point;

	for (float lat = 0.f; lat <= 3.141592654f * 2.0f; lat += step) {
		const auto& point3d = vec3_t(sin(lat), cos(lat), 0.f) * radius;
		vec2_t point2d;
		auto shit_pos = pos + point3d;
		if (g_render->world_to_screen(shit_pos, point2d))
			point.push_back(ImVec2(point2d.x, point2d.y));
	}
	if (in.a == 0 && out.a == 0 || radius < 0.5f || point.size() < count + 1)
		return;

	unsigned int vtx_base = g_render->m_draw_list->_VtxCurrentIdx;
	g_render->m_draw_list->PrimReserve(count * 3, count + 1);

	auto uv = ImGui::GetFontTexUvWhitePixel();
	g_render->m_draw_list->PrimWriteVtx(center, uv, ImColor(in.r, in.g, in.b, in.a));
	for (int n = 0; n < count; n++)
		g_render->m_draw_list->PrimWriteVtx(point[n + 1], uv, ImColor(out.r, out.g, out.b, out.a));

	for (int n = 0; n < count; n++) {
		g_render->m_draw_list->PrimWriteIdx((ImDrawIdx)(vtx_base));
		g_render->m_draw_list->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + n));
		g_render->m_draw_list->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + ((n + 1) % count)));
	}
	g_render->m_draw_list->_Path.Size = 0;
}


void c_render::autopick_cricle()
{
	g_hook->last_func = "autopick_cricle";
	Color_t color1 = settings.cfg().misc.autopick_color;
	color1.a = 160;
	Color_t color2 = settings.cfg().misc.autopick_color;
	color2.a = 0;

	if (!g_movement->m_autopeek.m_old_origin.is_zero() && g_movement->m_autopeek.m_autopeeking && sdk::m_local_pawn && sdk::m_local_pawn->m_is_alive())
		CircleFilled3D(g_movement->m_autopeek.m_old_origin, 25.f, color1, color2); // alpha: 160 , 0
}

void c_render::render_dmg(ImGuiIO& io)
{
	g_hook->last_func = "render_dmg";
	if (!g_interface->m_engine->is_in_game() || !g_interface->m_engine->is_connected())
		return;

	if (!sdk::m_local_pawn || !sdk::m_local_pawn->m_is_alive())
		return;
	ImVec2 displaySize = io.DisplaySize;
	vec2_t center = { displaySize.x / 2, displaySize.y / 2 };

//	g_render->text({ center.x + 10, center.y + 5 }, Color_t(255, 255, 255), outline, g_render->font_chika, std::to_string(settings.cfg().ragebot.r_min_damage), 10);
}

bool c_render::circle_filled_3d(vec3_t& origin, float radius, ImColor color, float thickness = 1.0f)
{
	std::vector<ImVec2> vert;

	const float step = (3.14f * 2.0f) / 50.f; // Шаг для построения круга

	for (float a = 0; a < (3.14f * 2.0f); a += step)
	{
		vec3_t start(radius * cosf(a) + origin.x, radius * sinf(a) + origin.y, origin.z);
		vec3_t end(radius * cosf(a + step) + origin.x, radius * sinf(a + step) + origin.y, origin.z);

		vec2_t start2d, end2d;
		if (!g_render->world_to_screen(start, start2d) || !g_render->world_to_screen(end, end2d))
			continue;

		// Добавляем точки в вектор
		vert.emplace_back(ImVec2(start2d.x, start2d.y));
		vert.emplace_back(ImVec2(end2d.x, end2d.y));
	}

	// Рисуем круг линиями (незаполненный)
	if (vert.size() > 3)
	{
		for (size_t i = 0; i < vert.size() - 1; i += 2)
		{
			ImGui::GetBackgroundDrawList()->AddLine(vert[i], vert[i + 1], color, thickness);
		}
		// Замыкаем круг (последняя линия)
		ImGui::GetBackgroundDrawList()->AddLine(vert.back(), vert.front(), color, thickness);
	}

	return true;
}

int c_render::get_correct_dxgi_format(int format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return format;
}

bool c_render::world_to_screen(const vec3_t& in, vec2_t& out)
{
	const auto& matrix = m_view_matrix;

	const float flWidth = matrix[3][0] * in.x + matrix[3][1] * in.y + matrix[3][2] * in.z + matrix[3][3];

	// check is point can't fit on screen, because it's behind us
	if (flWidth < 0.001f)
		return false;

	// compute the scene coordinates of a point in 3D
	const float flInverse = 1.0f / flWidth;
	out.x = (matrix[0][0] * in.x + matrix[0][1] * in.y + matrix[0][2] * in.z + matrix[0][3]) * flInverse;
	out.y = (matrix[1][0] * in.x + matrix[1][1] * in.y + matrix[1][2] * in.z + matrix[1][3]) * flInverse;

	// screen transform
	// get the screen position in pixels of given point
	const ImVec2 vecDisplaySize = ImGui::GetIO().DisplaySize;
	out.x = (vecDisplaySize.x * 0.5f) + (out.x * vecDisplaySize.x) * 0.5f;
	out.y = (vecDisplaySize.y * 0.5f) - (out.y * vecDisplaySize.y) * 0.5f;

	return true;
}

void c_render::create_render_target(IDXGISwapChain* swap_chain)
{
	g_hook->last_func = "create_render_target";
	ID3D11Texture2D* back_buffer = NULL;
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	if (back_buffer)
	{
		DXGI_SWAP_CHAIN_DESC sd;
		swap_chain->GetDesc(&sd);

		D3D11_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = static_cast<DXGI_FORMAT>(get_correct_dxgi_format(sd.BufferDesc.Format));

		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		if (FAILED(g_hook->m_device->CreateRenderTargetView(back_buffer, &desc, &g_hook->m_render_view)))
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if (FAILED(g_hook->m_device->CreateRenderTargetView(back_buffer, &desc, &g_hook->m_render_view)))
			{
				if (FAILED(g_hook->m_device->CreateRenderTargetView(back_buffer, NULL, &g_hook->m_render_view)))
				{
				}
			}
		}

		back_buffer->Release();
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool c_render::OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_inputsystem->OnWndProc(hWnd, uMsg, wParam, lParam);

	// switch menu state
	if (g_inputsystem->IsKeyReleased(VK_INSERT))
	{

		bMainWindowOpened = !bMainWindowOpened;

		// handle IsRelativeMouseMode original
		static const auto og = g_memory->getOrig<decltype(&g_hook->IsRelativeMouseMode)>("hkIsRelativeMouseMode");
		og(g_interface->InputSystem, bMainWindowOpened ? false : bMainActive);
	}
	// handle ImGui's window messages and block game's input if menu is opened
	return bMainWindowOpened ? ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) || bMainWindowOpened : bMainWindowOpened;
}

void c_render::spectator(const char* username, ImTextureID iconTexture, float offset)
{
	ImVec2 display_size = ImGui::GetIO().DisplaySize;
	ImVec2 window_size = ImVec2(120, 30);
	ImVec2 window_pos = { display_size.x - window_size.x, display_size.y / 2 };

	m_draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(0.1f, 0.1f, 0.1f, 1.0f), 5.0f);
	m_draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(0.5f, 0.1f, 0.6f, 1.0f), 5.0f);

	ImVec2 text_size = ImGui::CalcTextSize(username);
	float text_x = window_pos.x + 5.0f;
	float text_y = window_pos.y + (window_size.y - text_size.y) / 2.0f;
	m_draw_list->AddText(ImVec2(text_x, text_y), ImColor(255, 255, 255), username);

	if (iconTexture)
	{
		ImVec2 avatar_pos = ImVec2(window_pos.x + text_size.x + 10.0f, window_pos.y + (window_size.y - 20) / 2.0f);

		m_draw_list->AddRectFilled(avatar_pos, ImVec2(avatar_pos.x + 20, avatar_pos.y + 20), ImColor(0.2f, 0.2f, 0.2f, 1.0f), 5.0f);

		m_draw_list->AddImage(iconTexture, avatar_pos, ImVec2(avatar_pos.x + 20, avatar_pos.y + 20));
	}
}


bool c_render::LoadAndRenderImage(ID3D11Device* device, VTexParser::ImageData filepath, ID3D11ShaderResourceView** textureViewOut) {

	if (filepath.data.empty()) {
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = filepath.w;
	textureDesc.Height = filepath.h;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = filepath.data.data();
	initData.SysMemPitch = filepath.w * 4;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, texture.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	hr = device->CreateShaderResourceView(texture.Get(), nullptr, textureViewOut);
	if (FAILED(hr)) {
		return false;
	}
	return true;
}


ID3D11ShaderResourceView* c_render::load_texture_from_buffer(const uint8_t* image_data, int image_width, int image_height)
{
	if (image_data == nullptr || image_width <= 0 || image_height <= 0)
		return nullptr;
	ID3D11ShaderResourceView* out_srv = nullptr;
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = nullptr;
	D3D11_SUBRESOURCE_DATA subResource = {};
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4; // 4 áàéòà íà ïèêñåëü (RGBA)

	HRESULT hr = g_hook->m_device->CreateTexture2D(&desc, &subResource, &pTexture);
	if (FAILED(hr))
		return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = g_hook->m_device->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);

	if (FAILED(hr))
	{
		pTexture->Release();
		return nullptr;
	}

	pTexture->Release();
	return out_srv;
}

void c_render::rect_filled_multi_color(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImColor clr_top_left, ImColor clr_top_right, ImColor clr_bottom_left, ImColor clr_bottom_right) {
	draw_list->AddRectFilledMultiColor(p_min, p_max, clr_top_left, clr_top_right, clr_bottom_right, clr_bottom_left);
}

void c_render::counter_doubletap() {
	static ImVec2 pos = { 100, 100 };
	static bool is_dragging = false;

	bool is_rap = sdk::m_local_pawn->ActiveWeapon() &&
		!sdk::m_local_pawn->ActiveWeapon()->is_knife() &&
		!sdk::m_local_pawn->ActiveWeapon()->is_grenade() &&
		!sdk::m_local_pawn->ActiveWeapon()->is_zeus();

	if (sdk::m_local_pawn->ActiveWeapon()) {
		if (is_rap) {
			m_should_dt = true;

			if (sdk::m_local_pawn->ActiveWeapon()->m_attribute_manager()) {
				if (index_id == 0) {
					index_id = sdk::m_local_pawn->ActiveWeapon()->m_attribute_manager()->m_item()->m_item_definition_index();
				}

				int max_clip = sdk::m_local_pawn->ActiveWeapon()->m_weapon_data()->m_imaxclip1();
				float cycle_time = sdk::m_local_pawn->ActiveWeapon()->m_weapon_data()->m_cycle_time();

				if (index_id != sdk::m_local_pawn->ActiveWeapon()->m_attribute_manager()->m_item()->m_item_definition_index()) {
					start_time = g_interface->m_globalVars->flCurrentTime + cycle_time;
					dt_shoots = 0;
					index_id = sdk::m_local_pawn->ActiveWeapon()->m_attribute_manager()->m_item()->m_item_definition_index();
				}
				else if (g_interface->m_globalVars->flCurrentTime > start_time && dt_shoots < 2) {
					dt_shoots++;
					start_time = g_interface->m_globalVars->flCurrentTime + cycle_time;
				}
			}
		}
		else {
			dt_shoots = 0;
			m_should_dt = false;
		}
	}

	if (m_should_dt) {
		float radius = 20.0f;
		ImU32 color = IM_COL32(40, 35, 35, 150);
		ImU32 outline = IM_COL32(255, 255, 255, 255);
		ImU32 fill_color = IM_COL32(200, 0, 0, 150);

		if (bMainWindowOpened) {
			ImVec2 mouse_pos = ImGui::GetMousePos();
			bool hovered = ImLengthSqr(mouse_pos - pos) <= radius * radius;

			if (hovered && ImGui::IsMouseClicked(0)) {
				is_dragging = true;
			}
			if (!ImGui::IsMouseDown(0)) {
				is_dragging = false;
			}
			if (is_dragging) {
				pos = mouse_pos;
			}
		}

		float fill_ratio = dt_shoots / 2.0f;
		ImVec2 fill_pos = { pos.x - radius, pos.y - radius };
		ImVec2 fill_size = { 2 * radius, 2 * radius * fill_ratio };

		m_draw_list->AddCircleFilled(pos, radius, color);
		//m_draw_list->AddRectFilled(fill_pos, { fill_pos.x + fill_size.x, fill_pos.y + fill_size.y }, fill_color);
		m_draw_list->AddCircle(pos, radius, outline, 32, 2.0f);

		std::string text = std::to_string(dt_shoots);
		ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
		m_draw_list->AddText({ pos.x - text_size.x / 2, pos.y - text_size.y / 2 }, IM_COL32(255, 255, 255, 255), text.c_str());
	}
}

void RenderNotification()
{
	g_hook->last_func = "RenderNotification";
	static float progress = 0.0f;
	static float alpha = 1.0f;
	static bool fadingOut = false;

	ImVec2 windowSize(300, 60);
	ImVec2 screenPos(ImGui::GetIO().DisplaySize.x - 350, 50);

	if (alpha != 0.0f)
	{
		if (g_entry->init_cheat) {
			fadingOut = true;
			bMainWindowOpened = true;
			static const auto og = g_memory->getOrig<decltype(&g_hook->IsRelativeMouseMode)>("hkIsRelativeMouseMode");
			og(g_interface->InputSystem, bMainWindowOpened ? false : bMainActive);
		}

		if (fadingOut)
		{
			alpha -= 0.1f;
			if (alpha <= 0.0f)
			{
				alpha = 0.0f;
				return;
			}
		}

		ImGui::SetNextWindowPos(screenPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(windowSize);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.078f, 0.078f, 0.078f, alpha));

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.608f, 0.588f, 0.855f, alpha));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, alpha));

		ImGui::PushFont(g_render->font_tab);

		ImGui::Begin("##Notification", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored(ImVec4(0.608f, 0.588f, 0.855f, alpha), "STRWM");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, alpha), "Connect to server");

		ImGui::TextDisabled("Initialization cheat [24]");
		ImGui::ProgressBar(progress, ImVec2(-1.0f, 4.0f));

		ImGui::PopFont();

		progress += 0.005f;
		if (progress >= 1.0f) progress = 0.0f;

		ImGui::End();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
	}
}


void c_render::render_imgui_dx11(IDXGISwapChain* swap_chain)
{
	g_hook->last_func = "render_imgui_dx11";
	if (!g_hook->m_render_view)
	{
		create_render_target(swap_chain);
	}
	else
	{
		LiquidGlass::CaptureRenderTarget();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		g_world->hitmarker(Color_t(255, 255, 255));

		m_draw_list = ImGui::GetBackgroundDrawList();
        g_world->run_onshot();

		if (bMainWindowOpened)
			menu::render_menu();

		g_world->DrawScopeOverlay(m_draw_list);
		g_world->DrawBomb(m_draw_list);
		g_world->penitrations(m_draw_list);
		g_world->weapon_esp();
		ADD::drawBinds(m_draw_list);
		render_dmg(ImGui::GetIO());
		g_esp->draw_esp();
        if (settings.cfg().legitbot.b_aim_fov && settings.cfg().legitbot.b_drawFov && sdk::m_local_pawn && sdk::m_local_pawn->m_is_alive()) {
            const auto size = ImGui::GetIO().DisplaySize;
            const float fov = std::clamp(settings.cfg().legitbot.aim_Fov, 0.f, 89.f);
            const float radius = std::tan(fov * 0.01745329252f) * size.x * 0.5f;
            m_draw_list->AddCircle({size.x * 0.5f, size.y * 0.5f}, radius, IM_COL32(255,255,255,150), 96);
        }
		g_world->active_grande(m_draw_list);
		autopick_cricle();
	    RenderNotification();
		g_world->test(m_draw_list);

		//counter_doubletap();

		ImGui::Render();
		g_hook->m_context->OMSetRenderTargets(1, &g_hook->m_render_view, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}
/*
+========++======++++====+=+++==+++++++===+++=======+=====+==================+==++==========+++++++
+++=+==========+++++++++=+==+++===++++++===+=+++++++=++==++++=++++==+==+++++==+==++=++====+=+++++=
+++=+++++++====++=++++++++=+=+++++++++++==+=++====++=+=+++++++++====+++++=+=+=+===+===+====++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++=+++=+++++++++++++=============++====+=++++++++++
+++++++++++++===+=++++++++++++++++++++++++++++++++++++++++++++++++++++++=+=++==++++=+++++++==+=+++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=++++++++
++++++++++++==++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++###############################+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++@@%@%%%@@@@@@@@@%@@@@@@%@@%%@@@%@@@%#++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++@%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++#%%+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++%%%%%#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**%%%%%%%%%+++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%+++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%%%%%%%%%++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%+++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#+++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
github dibilovasanek-rgb aka strawman
*/