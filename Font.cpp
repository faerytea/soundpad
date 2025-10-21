#include "Font.hpp"

#ifdef __linux__

#include <fontconfig/fontconfig.h>

std::pair<std::string, std::string> getDefaultFontFiles() {
    if (!FcInit()) {
        SDL_Log("Cannt init fontconfig, use ImGui's defaults");
        return std::pair("", "");
    }
    auto fcConf = FcInitLoadConfigAndFonts();
    SDL_Log("FC conf: %s", FcConfigGetFilename(fcConf, nullptr));
    auto regularPattern = FcPatternCreate();
    FcPatternAddString(regularPattern, FC_FONTFORMAT, reinterpret_cast<const FcChar8 *>("TrueType"));
    FcPatternAddInteger(regularPattern, FC_WEIGHT, FC_WEIGHT_MEDIUM);
    FcPatternAddInteger(regularPattern, FC_WIDTH, FC_WIDTH_NORMAL);
    FcPatternAddInteger(regularPattern, FC_SLANT, FC_SLANT_ROMAN);
    FcConfigSubstitute(fcConf, regularPattern, FcMatchPattern);
    FcResult res;

    auto matchedRegular = FcFontMatch(fcConf, regularPattern, &res);
    std::string regularFile;
    if (res != FcResultMatch) {
        SDL_Log("Cannot find regular.ttf");
        regularFile = "";
    } else {
        unsigned char *path = nullptr;
        auto res = FcPatternGetString(matchedRegular, FC_FILE, 0, &path);
        if (res == FcResultMatch) {
            regularFile = std::string(reinterpret_cast<const char *>(path));
        } else {
            SDL_Log("Cannot get regular.ttf file");
            regularFile = "";
        }
    }
    FcPatternDestroy(regularPattern);

    auto monoPattern = FcPatternCreate();
    FcPatternAddString(monoPattern, FC_FONTFORMAT, reinterpret_cast<const FcChar8 *>("TrueType"));
    FcPatternAddInteger(monoPattern, FC_SPACING, FC_MONO);
    FcPatternAddInteger(regularPattern, FC_WEIGHT, FC_WEIGHT_MEDIUM);
    FcPatternAddInteger(regularPattern, FC_WIDTH, FC_WIDTH_NORMAL);
    FcPatternAddInteger(regularPattern, FC_SLANT, FC_SLANT_ROMAN);
    FcConfigSubstitute(fcConf, regularPattern, FcMatchPattern);

    auto matchedMono = FcFontMatch(fcConf, monoPattern, &res);
    std::string monoFile;
    if (res != FcResultMatch) {
        SDL_Log("Cannot find mono.ttf");
        monoFile = "";
    } else {
        unsigned char *path = nullptr;
        auto res = FcPatternGetString(matchedMono, FC_FILE, 0, &path);
        if (res == FcResultMatch) {
            monoFile = std::string(reinterpret_cast<const char *>(path));
        } else {
            SDL_Log("Cannot get mono.ttf file");
            monoFile = "";
        }
    }
    FcPatternDestroy(monoPattern);
    FcFini();
    return std::pair(regularFile, monoFile);
}

#elif __APPLE__

#else // assume windows

#endif
