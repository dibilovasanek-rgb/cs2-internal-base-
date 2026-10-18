#pragma once
#include "../../render/render.h"

class IInputSystem
{
public:
	bool IsRelativeMouseMode()
	{
		// @ida: 'IInputSystem::SetRelativeMouseMode'.
		return *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x4D);
	}

	void* GetSDLWindow()
	{
		// @ida: IInputSystem::DebugSpew -> #STR: "Current coordinate bias %s: %g,%g scale %g,%g\n"
		return *reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(this) + 0x26A8);
	}
};

enum class EKeyBindMode : int
{
	HOLD = 0,
	TOGGLE
};

struct KeyBind_t
{
	constexpr KeyBind_t(const char* szName, const unsigned int uKey = 0U, const EKeyBindMode nMode = EKeyBindMode::HOLD) :
		szName(szName), uKey(uKey), nMode(nMode) {
	}

	bool bEnable = false;
	const char* szName = nullptr;
	unsigned int uKey = 0U;
	EKeyBindMode nMode = EKeyBindMode::HOLD;
};

class c_inputsystem
{
public:
	using KeyState_t = std::uint8_t;

	enum EKeyState : KeyState_t
	{
		KEY_STATE_NONE,
		KEY_STATE_DOWN,
		KEY_STATE_UP,
		KEY_STATE_RELEASED
	};
	// last processed key states
	KeyState_t arrKeyState[256] = {};

	/// @returns: true if key is being held, false otherwise
	[[nodiscard]] bool IsKeyDown(const std::uint32_t uButtonCode)
	{
		return arrKeyState[uButtonCode] == KEY_STATE_DOWN;
	}

	/// @returns: true if key has been just released, false otherwise
	[[nodiscard]] bool IsKeyReleased(const std::uint32_t uButtonCode)
	{
		if (arrKeyState[uButtonCode] == KEY_STATE_RELEASED)
		{
			arrKeyState[uButtonCode] = KEY_STATE_UP;
			return true;
		}

		return false;
	}

	void Destroy();

	bool OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool GetBindState(KeyBind_t& keyBind);

	bool IsPressed(int key, int key_style);

	static c_inputsystem& get_instance() {
		static c_inputsystem instance;
		return instance;
	}
};
inline c_inputsystem* g_inputsystem = &c_inputsystem::get_instance();
