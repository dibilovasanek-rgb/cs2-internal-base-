#pragma once
#include "../../../memory/memory.h"
#include "../c_utl_string_token.h"

class Buffer {
public:
    uint32_t hash{};
    uint32_t pad{};
    const char* name{};
    Buffer() = default;
    explicit Buffer(const char* value) : hash(value ? MurmurHash2(value, static_cast<int>(strlen(value)), STRINGTOKEN_MURMURHASH_SEED) : 0), name(value) {}
};
static_assert(sizeof(Buffer) == 16);
class c_game_event;
class c_player_controller;

class c_game_event_helper
{
public:
	c_game_event_helper(c_game_event* Event) :
		Event{ std::move(Event) } {
	};

public:
	c_player_controller* get_player_controller();
	c_player_controller* get_attacker_controller();
	c_player_controller* get_controller(const char* controller_userid);
	c_player_controller* get_controller(const char* controller_userid, bool add_1);
	int get_damage();
	//int GetHealth();

protected:
	c_game_event* Event;
};

class c_player_pawn;

class C_GlowProperty {
public:
	c_player_pawn* GetParentEntity() {
		return *reinterpret_cast<c_player_pawn**>(reinterpret_cast<std::byte*>(this)+0x18);
	}
};

class c_game_event
{
public:
	virtual ~c_game_event() = 0;
	virtual const char* GetName() const = 0;

	virtual void Unk1() = 0;
	virtual void Unk2() = 0;
	virtual void Unk3() = 0;
	virtual void Unk4() = 0;
	std::int64_t get_int(std::string_view event_name) noexcept;
	virtual bool GetBool(const char* KeyName = nullptr, bool DefaultValue = false) = 0;
	virtual int GetInt(Buffer* KeyName = nullptr, bool DefaultValue = false) = 0;
	virtual uint64_t GetUint64(const char* KeyName = nullptr, uint64_t DefaultValue = 0) = 0;
	virtual float GetFloat(const char* KeyName = nullptr, float DefaultValue = 0.f) = 0;
	virtual const char* GetString(const char* KeyName = nullptr, const char* DefaultValue = "") = 0;
	virtual void Unk5() = 0;
	virtual void Unk6() = 0;
	virtual void Unk7() = 0;
	virtual void Unk8() = 0;
	virtual void GetControllerId(int& ControllerId, Buffer* Buffer = nullptr) = 0;

public:
	c_game_event_helper get_event_helper()
	{
		c_game_event_helper EventHelper(this);
		return EventHelper;
	}

	int get_int2(const char* Name, bool Unk);

	[[nodiscard]] float GetFloatNew(const CUtlStringToken1& keyToken, const float flDefault = 0.0f) const
	{
        Buffer key{}; key.hash = keyToken.GetHashCode1();
        return g_memory->CallVFunc<float>(const_cast<c_game_event*>(this), 9U, &key, flDefault);
	}
};

class i_game_event_listener
{
public:
	virtual ~i_game_event_listener() {}

	virtual void FireGameEvent(c_game_event* Event) = 0;
};

class c_game_event_manager
{
public:
	int LoadEventsFromFile(const char* szFileName)
	{
		return g_memory->CallVFunc<int>(this, 1U, szFileName);
	}

	void Reset()
	{
		g_memory->CallVFunc<void>(this, 2U);
	}

	bool AddListener(i_game_event_listener* pListener, const char* szName, bool bServerSide = false)
	{
		return  g_memory->CallVFunc<bool>(this, 3U, pListener, szName, bServerSide);
	}

	bool FindListener(i_game_event_listener* pListener, const char* szName)
	{
		return  g_memory->CallVFunc<bool>(this, 4U, pListener, szName);
	}

	void RemoveListener(i_game_event_listener* pListener)
	{
		g_memory->CallVFunc<void>(this, 5U, pListener);
	}
};

class c_event : public i_game_event_listener
{
public:
	float flHurtTime = 0.f;
	int damage = 0;

	bool Intilization();
	void FireGameEvent(c_game_event* event);
	void player_death(c_game_event* event);
	void bullet_impact(c_game_event* event);
	void player_hurt(c_game_event* event);
	//void EmplaceHitData(C_CSPlayerPawn* enity);
};
inline auto g_event = std::make_unique<c_event>();
