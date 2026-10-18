#pragma once

#include "discord_register.h"
#include "discord_rpc.h"
#include <memory>
#include <atomic>
#include <mutex>

class c_Discord {
public:
    void Initialize();
    void Shutdown();
    void Update(const char* map = nullptr);

private:
    std::atomic_bool initialized_{false};
    std::mutex mutex_;
};

inline auto g_Discord = std::make_unique<c_Discord>();
