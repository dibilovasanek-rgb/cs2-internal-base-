#include "inputsystem.h"
#include "../../core/hooks/hooks.h"


void c_inputsystem::Destroy()
{
	bMainWindowOpened = false;
	::Sleep(200U);

	if (g_hook->g_wnd_proc != nullptr)
	{
		SetWindowLongPtrW(g_hook->g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_hook->g_wnd_proc));
		g_hook->g_wnd_proc = nullptr;
	}
}

bool c_inputsystem::OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// prevent process when e.g. binding something in-menu
	if (wParam != VK_INSERT && wParam != VK_END && bMainWindowOpened)
		return false;

	// current active key
	int nKey = 0;
	// current active key state
	KeyState_t state = KEY_STATE_NONE;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256U)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_DOWN;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256U)
		{
			nKey = static_cast<int>(wParam);
			state = KEY_STATE_UP;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		nKey = VK_LBUTTON;
		state = uMsg == WM_LBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		nKey = VK_RBUTTON;
		state = uMsg == WM_RBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		nKey = VK_MBUTTON;
		state = uMsg == WM_MBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
		nKey = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2);
		state = uMsg == WM_XBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	default:
		return false;
	}

	// save key states
	if (state == KEY_STATE_UP && arrKeyState[nKey] == KEY_STATE_DOWN) // if swap states it will be pressed state
		arrKeyState[nKey] = KEY_STATE_RELEASED;
	else
		arrKeyState[nKey] = state;

	return true;
}

bool c_inputsystem::GetBindState(KeyBind_t& keyBind)
{
	if (keyBind.uKey == 0U)
		return false;

	switch (keyBind.nMode)
	{
	case EKeyBindMode::HOLD:
		keyBind.bEnable = IsKeyDown(keyBind.uKey);
		break;
	case EKeyBindMode::TOGGLE:
		if (IsKeyReleased(keyBind.uKey))
			keyBind.bEnable = !keyBind.bEnable;
		break;
	}

	return keyBind.bEnable;
}

bool c_inputsystem::IsPressed(int key, int key_style)
{
	switch (key_style)
	{
	case 0:
		return IsKeyDown(key);
	case 1:
		return IsKeyReleased(key);
	case 2:
		return true;
	default:
		return true;
	}
}