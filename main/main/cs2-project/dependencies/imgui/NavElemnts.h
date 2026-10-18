#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "imgui.h"
#include "imgui_internal.h"
#include "c_color.h"

namespace elements
{
	bool Checkbox(const char* label, bool* v);
	bool ColorCheckbox(const char* label, bool* v, c_color* color);
	bool PopupCheckbox(const char* label, bool* v, bool* penis);
	bool PopupCheckbox2(const char* label, bool* v, bool* penis);
	bool ClrEdit4(const char* label, c_color* color);
	bool checkboxP(const char* label, bool* v, const ImVec2& size_arg);
	bool tab(const char* icon, bool boolean);
	bool subtab(const char* name, bool boolean);
}
