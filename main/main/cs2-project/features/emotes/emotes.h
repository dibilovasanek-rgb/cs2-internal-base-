#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace emotes {

struct EmotesConfig {
    bool enabled = false;
    bool music = false;
    float volume = 0.70f;
    int wheel_key = 'B';
    int selected = 0;
};

struct EmoteEntry {
    std::string name;
    std::filesystem::path file;
};

// Adapter owned by the main DLL. It intentionally does not install a second
// Present/FrameStage hook; those are driven by the existing c_hook lifecycle.
void initialize();
void shutdown();
void frame();
void draw_menu();

EmotesConfig& config();
const std::vector<EmoteEntry>& catalog();
const std::filesystem::path& pack_directory();
bool pack_available();
bool is_playing();
void trigger_selected();
void stop();

} // namespace emotes
