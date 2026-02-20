#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "preface.hpp"

//#include <SDL3/SDL_filesystem.h>
#include <string>
#include <filesystem>
#include "Pad.hpp"

struct AppConfig {
    std::filesystem::path appdir;
    bool autosave = false;
    std::filesystem::path baseRoot;
    std::vector<std::filesystem::path> profiles;
    ImFont *fontMono;
    ImFont *fontRegular;
    std::pair<std::string, std::string> fontFiles;
};

// extern AppConfig appCfg;// = new AppConfig();

SoundPad *createDefault(MIX_Mixer *mixer);

SoundPad *loadSoundPad(std::filesystem::path &path, MIX_Mixer *mixer);

bool saveSoundPad(std::filesystem::path &path, SoundPad *pad);

ImFont *getFont(std::string &path, bool useVectorFallback = true);

AppConfig *loadAppConfig();

bool saveAppConfig(AppConfig *cfg);

#endif // CONFIG_HPP
