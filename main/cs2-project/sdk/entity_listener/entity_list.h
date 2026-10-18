#pragma once
#include <functional>
#include <mutex>
#include "../schema/class/entity.h"

class c_ent_listener final
{
    std::recursive_mutex mutex;
    std::map<uint32_t, std::vector<c_entity_instance*>> m_players;
public:
    void init();
    void on_level_init();
    void add(c_entity_instance* inst, c_base_handle handle);
    void level_shutdown();
    void remove(c_entity_instance* inst, c_base_handle handle);
    std::vector<c_entity_instance*> get(const char* class_name);
    static c_ent_listener& get_instance() {
        static c_ent_listener instance;
        return instance;
    }
};
inline c_ent_listener* g_entlistener = &c_ent_listener::get_instance();
