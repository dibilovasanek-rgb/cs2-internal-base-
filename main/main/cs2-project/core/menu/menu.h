#pragma once
#include <string>
#include <vector>

inline int animation_thirdperson = 0;
inline float flPitch = 0;
inline float flYaw = 0;

inline bool on_thirdperson = false;
inline bool manual_left = false;
inline bool manual_right = false;

inline int tab = 0;
inline int cfg = 0;
inline std::vector<std::string> all_CFG;
inline std::vector<const char*> cfg_to_cstr;
inline const char* hitboxes[] = { "head", "chest", "stomach", "arms", "legs", "feet" };
inline const char* multi_points[] = { "head", "chest", "stomach", "arms", "legs", "feet" };

enum Tab : int
{
	RAGEBOT,
	LEGITBOT,
	VISUALS,
	WORLD,
	MISC,
	CONFIG
};


inline const char* crosshairsName[] = {
	"default",
	"crosshair_1",
	"realno"
};

inline const char* chamsStyle[] = {
	"WHITE",
	"ILLUMINATE",
	"LATEX",
	"GLOW",
	"GOST",
	"FATAL"
};

inline const char* flags_array[] = {
	"C4",
	"KITS",
	"HK",
	"FLASHED",
	"ZEUS",
	"DEFUSER",
	"IMMUNE",
	"MONEY"
};

inline const char* flags_removals[] = {
	"REMOVE FLASH",
	"REMOVE SMOKE",
	"REMOVE TORSO",
};

inline const char* sky_name[] = {
	"defoult",
	"cloudy",
	"sunset",
	"night",
	"italy",
	"mirage",
	"nuke",
	"aztec2",
	"dust"
};

inline const char* pen_type[] = {
	"SKELETONE",
	"CIRCLE"
};

inline const char* aa_styles[] = {
	"BACKWARDS",
	"CUSTOM",
	"JITTER"
};

namespace menu
{
	void render_menu();
	void configs();
	void misc();
	void legitbot();
	void ragebot();
	void world();
	void visuals();
}