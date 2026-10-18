#pragma once
#include "fileIO.h"
#include <vector>
#include "../../../core/menu/menu.h"
#include <string>
#include "../../json.h"
#include "../../../sdk/game_class/color.h"
#include "hash.h"

enum hiteffects
{
	BRUTALPOINTS,
	SALUTBRO,
	EXPLOSIONBLUE,
	EXPLOSIONPURPLE,
	EXPLOSIONGREEN
};

enum mapeffects
{
	SNOWEFFECT,
	ASHEFFECT,
	RAINEFFECT,
	STAREFFECT
};

enum killeffects
{
	LIGHTING_BLUE,
	LIGHTING_PURPLE,
	LIGHTING_GREEN,
	PURPLETIME,
	GREENTIME,
	BLUETIME
};
//enum removals
//{
//	remove_flash,
//	remove_smoke,
//	remove_torso
//};
class CEconItem;

struct string_setting_t
{
	char text[128];
};

class node_t
{
public:
	virtual ~node_t() = default;

	virtual void add(node_t* node_ptr) = 0;

	virtual void load_setting(std::string& config_str) = 0;
	virtual void save_setting(std::string& config_str) = 0;
	//virtual bool set_value(std::string& feature) = 0;
};

class holder_t : public node_t
{
public:
	holder_t(const char* name) : m_name(name)
	{
	}

	holder_t(holder_t* holder_ptr, const char* name) : m_name(name) { holder_ptr->add(this); }

	void load_setting(std::string& node) override
	{
	}

	void save_setting(std::string& node) override
	{
	}

	//bool set_value(std::string& feature) override { return false; }

	void add(node_t* node_ptr) override;

	void load(const char* path, const char* file);

	void load(std::string& config);

	void save(std::string& config);

	void save(const char* path, const char* file);

	const char* m_name;
	std::vector<node_t*> m_nodes;
	std::string tempConfig;
};

class ISetting : public node_t
{
	void add(node_t* node) override
	{
	}
};

template <typename T>
class setting_t : ISetting
{
public:
	__forceinline setting_t(holder_t* holder_ptr, uint64_t name) : m_name(name) { holder_ptr->add(this); }

	__forceinline setting_t(holder_t* holder_ptr, uint64_t name, const T& rhs) : m_old_value(rhs), m_value(rhs), m_default(rhs), m_name(name) { holder_ptr->add(this); }

	bool changed()
	{
		uint8_t* data1 = reinterpret_cast<uint8_t*>(&m_value);
		uint8_t* data2 = reinterpret_cast<uint8_t*>(&m_old_value);

		for (size_t i = 0; i < sizeof(m_value); i++)
		{
			if (data1[i] != data2[i])
			{
				m_old_value = m_value;
				return true; // Objects are not equal
			}
		}
		return false;
	}

	void load_setting(std::string& config_str) override
	{
		m_value = m_default;

		uint8_t* data = (uint8_t*)&m_value;

		auto pos = config_str.find(std::to_string(m_name));
		if (pos == std::string::npos)
			return;

		pos += std::to_string(m_name).length() + 1;
		const char* buffer = config_str.data() + pos;
		for (size_t i = 0; i < sizeof(m_value); i++)
		{
			unsigned temp;
			sscanf(&buffer[2 * i], "%02X", &temp);
			data[i] = temp; //Set Value
		}
	}

	void save_setting(std::string& config_str) override
	{
		config_str.append(std::to_string(m_name) + ":");

		uint8_t* data = (uint8_t*)&m_value;

		char* buffer = new char[sizeof(m_value) * 2 + 1];
		for (size_t i = 0; i < sizeof(m_value); i++)
			sprintf(&buffer[2 * i], "%02X", data[i]);

		config_str.append(std::string(buffer) + ";");
		delete[] buffer;
	}

	setting_t& operator=(const T& rhs)
	{
		return *this;
	}

	__forceinline operator T& ()
	{
		return m_value;
	}

	__forceinline operator const T& () const
	{
		return m_value;
	}

	__forceinline T* operator &()
	{
		return &m_value;
	}

	__forceinline const T* operator &() const
	{
		return &m_value;
	}

	__forceinline T& operator ()()
	{
		return m_value;
	}

	__forceinline const T& operator ()() const
	{
		return m_value;
	}

	bool operator!=(const T& rhs) const
	{
		return m_value != rhs;
	}

private:
	T m_value;
	T m_old_value;
	T m_default;
	uint64_t m_name;
};

template <typename t>
class settingtype_t
{
public:
	settingtype_t(const t& value = { })
	{
		for (size_t i = 0; i < 1; ++i)
			m_data[i] = value;
	}

	t m_data[1];

	t& operator( )()
	{
		return m_data[0];
	}

	t& operator[](const size_t& iter)
	{
		return m_data[iter];
	}
};

// fuck it!
struct settingstring_t
{
	char buffer[128];

	settingstring_t()
	{
		clear();
	}

	settingstring_t(const char* text)
	{
		set(text);
	}

	__forceinline void set(std::string& str)
	{
		set(str.c_str());
	}

	__forceinline void set(const char* text)
	{
		clear();
		strncpy_s(buffer, 128, text, 128);
	}

	__forceinline std::string get() const
	{
		return buffer;
	}

	__forceinline bool empty() const
	{
		return buffer[0] == 0;
	}

	__forceinline void clear()
	{
		memset(buffer, 0, 128);
	}

	__forceinline const char* c_str()
	{
		return buffer;
	}
};

struct Visuals {
	bool b_esp = false;
	bool b_customsky = false;
	bool b_modelchanger = false;
	bool b_box = false;
	bool b_sound_only_esp = false;
	bool b_onlyvisible = false;
	bool b_healthBar = false;
	bool b_nicknames = false;
	bool b_playeravatar = false;
	bool b_weapons = false;
	bool b_knifechanger = false;
	bool b_skeleton = false;
	bool b_hpbar_glow = false;
	bool b_ammobar_glow = false;
	bool b_bomb_esp = false;
	bool b_weapon_in_world = false;
	bool b_hitmarker = false;
	bool b_show_penetration = false;
	bool b_spectators = false;
	bool b_ping_flag = false;
	bool b_oof = false;
	bool b_have_c4 = false;
	bool b_render_binds = false;
	bool b_chams = false;
	bool b_chams_hand = false;
	bool b_chams_invisible = false;
	bool b_chams_hand_invisible = false;
	bool b_chams_weapons = false;
	bool b_chams_weapons_invisible = false;
	bool b_scope_fov = false;
	bool b_sound_esp = false;
	bool b_weaponicon = false;
	bool b_weaponname = false;
	bool b_weaponbar = false;


	Color_t c_mapeffect = { 255, 255, 255 };
	Color_t c_weaponbar = { 0, 255, 0 };
	Color_t chams_visible = { 255, 255, 255 };
	Color_t chams_invisible = { 255, 255, 255 };

	Color_t chams_hand_visible = { 255, 255, 255 };
	Color_t chams_hand_invisible = { 255, 255, 255 };

	Color_t chams_weapon_invisible = { 255, 255, 255 };
	Color_t chams_weapon_visible = { 255, 255, 255 };
	int chams_style = 0;
	int oof_size = 0;
	int chams_style_invisible = 0;
	int chams_style_hand_invisible = 0;
	int chams_style_hand = 0;
	int chams_style_weapons = 0;
	int chams_style_weapons_invisible = 0;

	int zoom_1_override = 0;
	int zoom_2_override = 0;
	Color_t penetration_color = { 255, 255, 255 };
	float oof_radius = 100.f;
	bool b_onhits = false;
	Color_t onhit_color = { 255, 255, 255 };
	bool b_glow_esp = false;
	bool b_weapons_glow = false;
	bool b_local_glow = false;
	std::array<bool, 8> m_flags;
	std::array<bool, 3> m_removals;

	Color_t glow_color = { 255, 0, 0 };
	Color_t glow_weapons_color = { 255, 255, 255 };
	Color_t glow_local_color = { 255, 255, 255 };

	Color_t box_color = { 255,255,255 };
	Color_t heathbar_color = { 0,255,0};
	bool b_gradienthpbar = false;
	bool b_gradientammobar = false;
	Color_t c_gradientammobar2 = Color_t(255, 0, 0);
	Color_t c_gradienthpbar2 = Color_t(255, 0, 0);
	Color_t skeleton_color = { 255,255,255 };
	Color_t oof_color = { 255,255,255 };

	Color_t for_test = { 255,0,0};

	bool m_mapeffect = false;
	int i_mapeffect = SNOWEFFECT;
	bool m_hiteffect = false;
	int i_hiteffectpart = BRUTALPOINTS;
	bool m_killeffect = false;
	int i_killeffectpart = LIGHTING_BLUE;

	//bool m_removals;
	//int i_removalspart = remove_flash;

	Visuals() = default;
};

struct WorldModulation {
	bool b_worldModulation = false;
	bool b_ParticlesEffect = false;
	bool b_bullet_trace = false;
	bool b_world_blur = false;
	bool b_dynamic_blur = false;

	Color_t worldColor = { 255, 255, 255 };
	Color_t skyColor = { 255, 255, 255 };
	Color_t particleColor = { 255, 255, 255 };
	Color_t cloudColor = { 255, 255, 255 };
	Color_t sunColor = { 255, 255, 255 };
	Color_t wallColor = { 255, 255, 255 };
	float fl_exposure = 0.f;
	Color_t bullet_color = { 255, 255, 255 };

	WorldModulation() = default;
};

struct Movement {
	bool b_bunnyhope = false;
	bool b_autostrafe = false;
	int smooth_stafe = 100;
	bool b_auto_pick = false;

	Movement() = default;
};

struct Ragebot {
	bool b_ragebot = false;
	bool b_knifebot = false;
	bool b_anti_aim = false;
	bool b_show_penetration = false;
	int r_min_damage = 0;
	int r_min_damage_over = 0;
	int key_min_damage_over = 0;
	int style_min_damage_over = 0;
	int r_hitchance = 0;
	bool m_doubletap;
	bool m_hitboxes[6];
	bool m_multi_points[6];
	int m_multipoint_scale = 0;

	int aa_style = 0;
	float aa_yaw = 0.f;
	float aa_pitch = 0.f;
	int jitter_speed = 2;

	int i_key_left_manual = 'C';
	int i_key_right_manual = 'Z';
	int i_style_manual = 0;

	Ragebot() = default;
};

struct legit_bot {
	bool legit_visible = false;
	float legit_smooth = 0.5f;
	int legit_bone = 0;
	bool b_drawFov = false;
	bool b_aim_fov = false;
	float aim_Fov = 0.f;
	float Radius = 0.f;
	bool b_trigger = false;
	int trigger_hitchance = 0;
	int trigger_min_dmg = 0;

	legit_bot() = default;
};

struct misc {
	bool auto_accept = false;
	int crosshairs = 3;
	float hand_X = 0.f;
	float hand_Y = 0.f;
	float hand_Z = 0.f;
	float aspect_ratio_value = 1.f;
	bool b_aspect_ratio = false;
	bool b_third_person = false;
	float p_fov = 90.f;
	int i_key_thirdperson = VK_MBUTTON;
	int i_style_thirdperson = 1;
	int thirdperson_dist = 0;
	int iSky = 0;
	int icustom_sound = 0;
	float flviewmodelfov = 70;
	int i_key_autopeek = 0;
	int i_style_autopeek = 0;
	float rotPitch = 0;
	float rotYaw = 0;
	float rotRoll = 0;
	Color_t autopick_color = Color_t(255,255,255);
	misc() = default;
};

struct skins {
	int custom_agent = 0;
	int custom_knife = 0;

	skins() = default;
};


struct placeholder_struct {
	Visuals visuals;
	WorldModulation worldModulation;
	Movement movement;
	Ragebot ragebot;
	legit_bot legitbot;
	misc misc;
	skins skins;
	placeholder_struct() = default;
};

struct EconItemData {
	uint16_t unDefIndex;
	uint8_t nQuality;
	uint8_t nRarity;
	float paintKit;
	float paintSeed;
	float paintWear;
	int statTrak;
};

class c_settings
{
public:
	bool tooltips = false;
	holder_t m_holder{ "" };

	setting_t<placeholder_struct> cfg = {
		&m_holder,
		fnvc("cfg"),
		placeholder_struct()
	};


	std::string GetConfigPath();

	void save(const char* name)
	{
		if (!DoesPathExist(GetConfigPath()))
			CreateFolder(GetConfigPath());
		m_holder.save(GetConfigPath().c_str(), name);
	}

	bool exists(const char* name)
	{
		if (!DoesPathExist(GetConfigPath()))
			CreateFolder(GetConfigPath());
		GetConfigPath() += "\\" + std::string(name);
		if (std::filesystem::exists(GetConfigPath()))
			return true;
		return false;
	}

	void load(const char* name)
	{
		if (!DoesPathExist(GetConfigPath()))
			return;
		m_holder.load(GetConfigPath().c_str(), name);
	}

	void _delete(const char* name)
	{
		if (!DoesPathExist(GetConfigPath()))
			return;
		GetConfigPath() += "\\" + std::string(name);
		if (std::filesystem::exists(GetConfigPath()))
			std::filesystem::remove(GetConfigPath());
	}

	std::vector<std::string> get_all_configs()
	{
		std::vector<std::string> configs;
		if (!DoesPathExist(GetConfigPath()))
			return configs;
		//get all files in folder
		try
		{
			for (const auto& entry : std::filesystem::directory_iterator(GetConfigPath()))
				if (std::filesystem::is_regular_file(entry.path()))
					configs.push_back(entry.path().filename().string());
		}
		catch (const std::exception& e)
		{
		}
		return configs;
	}

	void init_all_cfg()
	{
		all_CFG = get_all_configs();
		cfg_to_cstr.clear();
		for (const auto& item : all_CFG) {
			cfg_to_cstr.push_back(item.c_str());
		}
	}
	void save_conf(std::string& config) { m_holder.save(config); }
	void load_conf(std::string& config) { m_holder.load(config); }

	//Place holders that can come in handy
	template <typename T>
	void Set(T* setting, T value)
	{
		*setting = value;
	}

	template <typename T>
	void Toggle(T* setting)
	{
		*setting = !*setting;
	}
	void SaveEconItem(CEconItem* pItem, float seed, float wear, int statrack, const std::string& filename);
	std::vector<CEconItem*> LoadEconItems(const std::string& filename);
};
extern c_settings settings;

