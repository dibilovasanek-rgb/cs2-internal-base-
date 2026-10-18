#pragma once
#include "../../schema/class/player.h"

enum prediction_reason
{
    client_command_tick,
    demo_preentity,
    demo_simulation,
    postnetupdate,
    server_starved_and_added_usercmds,
    client_frame_simulate
};

class c_prediction {
public:
    char gap0[48];
    int reason;
    bool InPrediction;
    bool EnginePause;
    char gap36[6];
    int incoming_acknowledged;
    std::uint64_t delta_tick;
    std::uint64_t slot;
    c_player_controller* ControllerEntity;
    char gap58[4];
    int int5C;
    char gap60[32];
    bool bFirstPrediction;
    char pad90[15];
    int storage_entity_size;
    char gap94[4];
    void* storage_entitys;
    char gapA0[48];
    int count_penisov;
    char gapD4[4];
    void* position;
    char gapE0[8];
    float real_time;
    char gapEC[12];
    int max_client;
    char gapFC[12];
    std::uintptr_t* unk_class;
    float current_time;
    char gap11C[12];
    int tick_count;
    char gap12C[4];
    char gap130[2];
    char byte132;
    char gap133[3];
    bool HasUpdate;
};