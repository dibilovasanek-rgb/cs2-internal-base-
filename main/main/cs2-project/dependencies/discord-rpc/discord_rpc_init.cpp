#include "discord_rpc_init.h"
#include "../../sdk/sdk.h"
#include <string>
#include <ctime>

void c_Discord::Initialize()
{
    std::lock_guard lock(mutex_);
    bool expected = false;
    if (!initialized_.compare_exchange_strong(expected, true))
        return;

    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("1560712865332076694", &Handle, 1, NULL);
}

void c_Discord::Shutdown()
{
    std::lock_guard lock(mutex_);
    if (!initialized_.exchange(false))
        return;
    Discord_Shutdown();
}

void c_Discord::Update(const char* map)
{
    std::lock_guard lock(mutex_);
    if (!initialized_.load())
        return;

    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    auto elapsed = std::time(nullptr);
    std::string state;

    if (map != nullptr && *map != '\0' && strcmp(map, "<empty>") != 0)
        state = std::string("Playing on ") + map;
    else
        state = std::string("Staying on main menu");

    discordPresence.largeImageText = "STRWM";               
    discordPresence.state = state.c_str();
    discordPresence.largeImageKey = "strwm_logo"; // Asset key configured in the Discord application dashboard.
    discordPresence.startTimestamp = elapsed;   
    // Discord accepts an application asset key here, not an arbitrary URL.
    discordPresence.smallImageKey = "strwm_logo";

    discordPresence.button1_label = "Telegram";
    discordPresence.button1_url = "https://t.me/norssup";

        //discordPresence.smallImageText = sdk::m_controller->m_sanitized_player_name(); // ���� ����� ���

    Discord_UpdatePresence(&discordPresence);
}   
