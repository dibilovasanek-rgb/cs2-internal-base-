#pragma once
#include <vector>
#ifdef _DEBUG
#define WINCALL(func) func
#else
#define WINCALL(func) func
//#define WINCALL(func) LI_FN(func).cached()
#endif

class entry_point
{
public:
    std::vector<std::string> models_agents;
    std::vector<std::string> models_knife;
    std::vector<std::string> custom_sounds;
    bool init_cheat = false;

};
inline auto g_entry = std::make_unique<entry_point>();
