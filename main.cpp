/**
 * Soundpad - A simple crossplatform soundpad app.
 * Copyright © 2025 faerytea
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "preface.hpp"
#include <SDL3/SDL_main.h>
#include "soundpad.hpp"
#include "Config.hpp"
#include "Font.hpp"
#include "Help.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static MIX_Mixer *mixer = NULL;
static AppConfig *appCfg = nullptr;

const SDL_DialogFileFilter ttfFileFilter = { "TrueType Font", "ttf" };
const char *defaultFontDir =
#ifdef _WIN32
                            "C:\\Windows\\Fonts\\"
#elif __APPLE__
                            "/Library/Fonts/"
#else
                            "/usr/share/fonts/truetype/"
#endif
;

struct AppState {
    SoundPad *selected = nullptr;
    std::filesystem::path currentProfile;
    Pad *selectedPad = nullptr;
    Uint64 lastFrame = 0;
    char **requestStrings = new char *[7] {
        "NONE",
        "ONE_SHOT",
        "STOP",
        "PAUSE",
        "RESUME",
        "LOOP",
        "HELD",
    };
    const Help *helpWindow = nullptr;
#ifdef FPS
    Uint64 fps = 0;
    Uint64 lastFpsReset = 0;
#endif
};

const char *helpContent[] = {
                    "\tTo interact with a pad, click on it (or press its corresponding key). Ctrl, Alt and Shift modifiers can be used.",
                    "\tTo configure a pad, click on it with right mouse button.",
                    "\tRequest explanaition:",
                    "\t- NONE: Do nothing.",
                    "\t- ONE_SHOT: Play the sound once from the start (without interrupting the current playback, even from the same key).",
                    "\t- STOP: Stop the sound playback.",
                    "\t- PAUSE: Pause the sound playback (can be resumed).",
                    "\t- RESUME: Resume a paused sound playback.",
                    "\t- LOOP: Continuously play the sound in a loop until stopped.",
                    "\t- HELD: Play the sound while the key is held down, stop when released.",
                };
const Help appHelp = {
    "Help",
    helpContent,
    10
};

const char *aboutContent[] = {
                    "Pretty simple soundpad on SDL3 + Dear ImGui.",
                    "This program is free software: you can redistribute it and/or modify",
                    "it under the terms of the GNU General Public License as published by",
                    "the Free Software Foundation, either version 3 of the License, or",
                    "(at your option) any later version.",
                    "This program is distributed in the hope that it will be useful,",
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of",
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
                    "GNU General Public License for more details.",
                    "You should have received a copy of the GNU General Public License",
                    "along with this program.  If not, see <https://www.gnu.org/licenses/>.",
                    "Source code: https://github.com/faerytea/soundpad",
                    "This project uses SDL3, SDL_mixer (both under zlib), and Dear ImGui (MIT License).",
                };
const Help appAbout = {
    "About Soundpad v1.2.2",
    aboutContent,
    13
};

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("ft's soundpad", "1.0", "name.faerytea.soundpad");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("Sounpad", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        SDL_Log("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mixer == nullptr) {
        SDL_Log("Couldn't create mixer device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("SDL init success");

    appCfg = loadAppConfig();
    if (!appCfg) {
        return SDL_APP_FAILURE;
    }

    *appstate = new AppState();

    SDL_Log("Appdir: %s", appCfg->appdir.u8string().c_str());

    // SDL_SetRenderLogicalPresentation(renderer, 1280 * main_scale, 800 * main_scale, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (ImGui_ImplSDL3_ProcessEvent(event)) return SDL_APP_CONTINUE;
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
    auto state = static_cast<AppState *>(appstate);
    auto now = SDL_GetTicks();
    auto lastAI = state->lastFrame;
    if (now - lastAI < 16) {
        SDL_Delay(16 - now + lastAI);
        return SDL_APP_CONTINUE;
    }
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(32);
        return SDL_APP_CONTINUE;
    }
    state->lastFrame = now;
#ifdef FPS
    ++(state->fps);
    auto nowNs = SDL_GetTicksNS();
    auto trackSpan = (nowNs - state->lastFpsReset);
    if (trackSpan == 0) trackSpan = 1;
    auto realFPS = (state->fps * 1000000000) / trackSpan;
    if (trackSpan > 1000000000) {
        state->fps = 0;
        state->lastFpsReset = nowNs;
    }
#endif
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    auto sp = state->selected;
    if (sp == nullptr) {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("Select profile", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        for (auto pi = appCfg->profiles.begin(); pi != appCfg->profiles.end();) {
            auto &p = *pi;
            auto tp = p.filename().stem().u8string();
            if (ImGui::Button((std::string("X##") + tp).c_str())) {
                std::filesystem::remove(p);
                std::filesystem::remove_all(std::filesystem::u8path(tp) / "");
                appCfg->profiles.erase(pi);
            } else {
                ++pi;
            }
            ImGui::SameLine();
            if (ImGui::Button(p.filename().u8string().c_str(), ImVec2(-1, 0))) {
                SoundPad *newPad = loadSoundPad(p, mixer);
                state->selected = newPad;
                state->currentProfile = p;
            }
        }
        ImGui::Text("...or create a new one:");
        static char newProfileName[254];
        ImGui::InputTextWithHint("##new profile", "new profile", newProfileName, sizeof(newProfileName) - 4);
        if (ImGui::Button("Create", ImVec2(-1, 0))) {
            SDL_Log("Appdir on click: %s", appCfg->appdir.u8string().c_str());
            std::string_view newNameView(newProfileName);
            auto len = newNameView.size();
            SDL_Log("Creating new profile '%s'", newProfileName);
            if (len > 0) {
                newProfileName[len] = '.';
                newProfileName[len + 1] = 'c';
                newProfileName[len + 2] = 'f';
                newProfileName[len + 3] = 'g';
                newProfileName[len + 4] = 0;
                auto newPath = appCfg->appdir / "profiles" / std::filesystem::u8path(newProfileName);
                if (std::filesystem::exists(newPath)) {
                    SDL_Log("Profile %s already exists", newPath.u8string().c_str());
                } else {
                    std::filesystem::create_directories(newPath.parent_path());
                    SoundPad *newPad = createDefault(mixer);
                    if (newPad) {
                        static_cast<AppState *>(appstate)->selected = newPad;
                        SDL_Log("Created new profile %s", newPath.u8string().c_str());
                        saveSoundPad(newPath, newPad);
                        appCfg->profiles.push_back(newPath);
                        state->currentProfile = newPath;
                    } else {
                        SDL_Log("Failed to create new profile %s", newPath.u8string().c_str());
                    }
                }
            }
            std::fill(newProfileName, newProfileName + sizeof(newProfileName), 0);
        }
        ImGui::End();
    } else {
        ImGui::BeginMainMenuBar();
        if (ImGui::MenuItem("Change profile")) {
            saveSoundPad(state->currentProfile, state->selected);
            state->currentProfile = std::filesystem::path();
            delete state->selected;
            state->selected = nullptr;
            state->selectedPad = nullptr;
        }
        if (appCfg->autosave) {
            ImGui::Text("Autosave enabled");
        } else {
            if (ImGui::MenuItem(("Save " + state->currentProfile.filename().u8string()).c_str())) {
                if (saveSoundPad(state->currentProfile, state->selected)) {
                    SDL_Log("Saved profile %s", state->currentProfile.u8string().c_str());
                } else {
                    SDL_Log("Failed to save profile %s", state->currentProfile.u8string().c_str());
                }
            }
        }
        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Autosave", nullptr, &(appCfg->autosave));
            if (ImGui::MenuItem("Base sound dir")) {
                SDL_ShowOpenFolderDialog(
                    [](void *userdata, const char * const *filelist, int filter) {
                        if (filelist && filelist[0]) {
                            auto p = static_cast<AppConfig *>(userdata);
                            p->baseRoot = std::filesystem::u8path(filelist[0]);
                        }
                    },
                    appCfg,
                    window,
                    appCfg->baseRoot.u8string().c_str(),//"/home/faerytea/.steam/debian-installation/steamui/sounds/", // tmp
                    false
                );
            }
            if (ImGui::BeginMenu("Fonts")) {
                auto regularPath = appCfg->fontFiles.first.empty() ? "embedded" : appCfg->fontFiles.first;
                auto regularLastSlash = regularPath.find_last_of("/\\");
                auto regularName = regularPath.substr(regularLastSlash == std::string::npos ? 0 : regularLastSlash + 1);
                if (ImGui::MenuItem(("Regular: " + regularName).c_str())) {
                    SDL_ShowOpenFileDialog(
                        [](void *userdata, const char * const *filelist, int filter) {
                            if (filelist && filelist[0]) {
                                auto p = static_cast<AppConfig *>(userdata);
                                auto ttf = std::filesystem::u8path(filelist[0]);
                                p->fontFiles.first = ttf.u8string();
                                // auto &io = ImGui::GetIO();
                                // p->fontRegular = getFont(p->fontFiles.first);
                                // io.FontDefault = p->fontRegular;
                                saveAppConfig(p);
                            }
                        },
                        appCfg,
                        window,
                        &ttfFileFilter,
                        1,
                        regularLastSlash == std::string::npos
                            ? defaultFontDir
                            : std::filesystem::u8path(regularPath.substr(0, regularLastSlash)).u8string().c_str(),
                        false
                    );
                }
                if (ImGui::MenuItem("Reset to embedded##regular", nullptr, false, regularPath != "embedded")) {
                    appCfg->fontFiles.first = "embedded";
                    // auto &io = ImGui::GetIO();
                    // appCfg->fontRegular = getFont(appCfg->fontFiles.first);
                    saveAppConfig(appCfg);
                }
                ImGui::Separator();
                auto monoPath = appCfg->fontFiles.second.empty() ? "embedded" : appCfg->fontFiles.second;
                auto monoLastSlash = monoPath.find_last_of("/\\");
                auto monoName = monoPath.substr(monoLastSlash == std::string::npos ? 0 : monoLastSlash + 1);
                if (ImGui::MenuItem(("Mono: " + monoName).c_str())) {
                    SDL_ShowOpenFileDialog(
                        [](void *userdata, const char * const *filelist, int filter) {
                            if (filelist && filelist[0]) {
                                auto p = static_cast<AppConfig *>(userdata);
                                auto ttf = std::filesystem::u8path(filelist[0]);
                                p->fontFiles.second = ttf.u8string();
                                // auto &io = ImGui::GetIO();
                                // p->fontMono = getFont(p->fontFiles.second);
                                saveAppConfig(p);
                            }
                        },
                        appCfg,
                        window,
                        &ttfFileFilter,
                        1,
                        monoLastSlash == std::string::npos
                            ? defaultFontDir
                            : std::filesystem::u8path(monoPath.substr(0, monoLastSlash)).u8string().c_str(),
                        false
                    );
                }
                if (ImGui::MenuItem("Reset to embedded##mono", nullptr, false, monoPath != "embedded")) {
                    appCfg->fontFiles.second = "embedded";
                    // auto &io = ImGui::GetIO();
                    // appCfg->fontMono = getFont(appCfg->fontFiles.second);
                    saveAppConfig(appCfg);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset to system's default")) {
                    appCfg->fontFiles = getDefaultFontFiles();
                    // auto &io = ImGui::GetIO();
                    // appCfg->fontRegular = getFont(appCfg->fontFiles.first);
                    // io.FontDefault = appCfg->fontRegular;
                    // appCfg->fontMono = getFont(appCfg->fontFiles.second);
                    saveAppConfig(appCfg);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help##menu")) {
            if (ImGui::MenuItem("Help##item")) {
                state->helpWindow = &appHelp;
            }
            if (ImGui::MenuItem("About")) {
                state->helpWindow = &appAbout;
            }
            ImGui::EndMenu();
        }
#ifdef FPS
        ImGui::Text("FPS: %lu", realFPS);
#endif
        ImGui::EndMainMenuBar();
        if (state->selected) {
            Pad *selectedPad = ShowSoundPad(*sp, state->selectedPad == nullptr, appCfg->fontMono);
            if (state->selectedPad == nullptr) {
                state->selectedPad = selectedPad;
            }
        }
        static bool cfgAlt, cfgCtrl, cfgShift;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            state->selectedPad = nullptr;
        }
        if (state->selectedPad != nullptr) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            char name[2] = {state->selectedPad->letter, 0};
            ImGui::Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
            if (ImGui::Button("X", ImVec2(0, 0))) {
                state->selectedPad->unloadSound();
                if (appCfg->autosave) {
                    saveSoundPad(state->currentProfile, state->selected);
                }
            }
            ImGui::SameLine();
            auto &sName = state->selectedPad->name;
            if (ImGui::Button(sName.empty() ? "Select..." : sName.c_str(), ImVec2(-1, 0))) {
                SDL_ShowOpenFileDialog(
                    [](void *userdata, const char * const *filelist, int filter) {
                        if (filelist && filelist[0]) {
                            auto p = static_cast<std::tuple<Pad *, AppConfig *, AppState *> *>(userdata);
                            auto pad = std::get<0>(*p);
                            auto appCfg = std::get<1>(*p);
                            auto state = std::get<2>(*p);
                            auto appDir = appCfg->appdir;
                            auto path = std::filesystem::u8path(filelist[0]);
                            if (pad->loadSound(path.u8string())) {
                                SDL_Log("Loaded sound %s on pad %c", pad->name.c_str(), pad->letter);
                                auto base = appDir / "profiles" / state->currentProfile.stem();
                                std::filesystem::create_directories(base);
                                std::filesystem::copy_file(
                                    path, 
                                    base / std::filesystem::u8path(pad->name), 
                                    std::filesystem::copy_options::create_hard_links | std::filesystem::copy_options::skip_existing
                                );
                                if (appCfg->autosave) {
                                    saveSoundPad(state->currentProfile, state->selected);
                                }
                            } else {
                                SDL_Log("Failed to load sound on pad %c", pad->letter);
                            }
                        }
                    },
                    new std::tuple<Pad *, AppConfig *, AppState *>(state->selectedPad, appCfg, state), // will be deleted by the dialog
                    window,
                    nullptr,
                    0,
                    appCfg->baseRoot.u8string().c_str(),
                    false
                );
            }
            ImGui::Separator();
            if (ImGui::BeginTable("Transitions", 3)) {
                if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) cfgCtrl = true;
                if (ImGui::IsKeyDown(ImGuiMod_Alt)) cfgAlt = true;
                if (ImGui::IsKeyDown(ImGuiMod_Shift)) cfgShift = true;
                if (ImGui::IsKeyReleased(ImGuiMod_Ctrl)) cfgCtrl = false;
                if (ImGui::IsKeyReleased(ImGuiMod_Alt)) cfgAlt = false;
                if (ImGui::IsKeyReleased(ImGuiMod_Shift)) cfgShift = false;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Checkbox("Ctrl", &cfgCtrl);
                ImGui::TableNextColumn();
                ImGui::Checkbox("Alt", &cfgAlt);
                ImGui::TableNextColumn();
                ImGui::Checkbox("Shift", &cfgShift);
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("States", 2, ImGuiTableFlags_SizingFixedSame)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableHeader("When inactive:");
                ImGui::TableNextColumn();
                ImGui::TableHeader("When playing:");
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##fromSilence", state->requestStrings[state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][0]])) {
                    for (int i = NONE; i <= HELD; ++i) {
                        bool isSelected = (state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][0] == i);
                        if (ImGui::Selectable(state->requestStrings[i], isSelected)) {
                            state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][0] = static_cast<PadStateRequest>(i);
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                            if (appCfg->autosave) {
                                saveSoundPad(state->currentProfile, state->selected);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##fromPlaying", state->requestStrings[state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][1]])) {
                    for (int i = NONE; i <= HELD; ++i) {
                        bool isSelected = (state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][1] == i);
                        if (ImGui::Selectable(state->requestStrings[i], isSelected)) {
                            state->selectedPad->table[cfgCtrl ? 1 : 0][cfgShift ? 1 : 0][cfgAlt ? 1 : 0][1] = static_cast<PadStateRequest>(i);
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                            if (appCfg->autosave) {
                                saveSoundPad(state->currentProfile, state->selected);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::EndTable();
            }
            ImGui::Separator();
            float volume = state->selectedPad->volume();
            float prevVolume = volume;
            ImGui::SliderFloat("Volume", &volume, 0.f, 2.f);
            if (prevVolume != volume) {
                state->selectedPad->volume(volume);
                if (appCfg->autosave) {
                    saveSoundPad(state->currentProfile, state->selected);
                }
            }
            if (ImGui::Button("Close", ImVec2(-1, 0))) {
                state->selectedPad = nullptr;
            }
            if (!appCfg->autosave) {
                if (ImGui::Button("Save", ImVec2(-1, 0))) {
                    saveSoundPad(state->currentProfile, state->selected);
                }
            }
            ImGui::End();
        } else {
            cfgAlt = false;
            cfgCtrl = false;
            cfgShift = false;
        }
        bool showHelp = state->helpWindow != nullptr;
        if (showHelp) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
            if (ImGui::Begin(state->helpWindow->title, &showHelp, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                for (unsigned i = 0; i < state->helpWindow->contentLines; ++i) {
                    ImGui::Text(state->helpWindow->content[i]);
                }
            }
            ImGui::End();
            if (!showHelp) {
                state->helpWindow = nullptr;
            }
        }
    }

    ImGui::Render();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer, .5, 0, .5, 1);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    auto regularName = std::string_view(appCfg->fontRegular->GetDebugName());
    auto monoName = std::string_view(appCfg->fontMono->GetDebugName());
    auto isRegLoadedEmbedded = regularName == "ProggyClean.ttf" || regularName == "ProggyForever.ttf";
    auto isMonoLoadedEmbedded = monoName == "ProggyClean.ttf" || monoName == "ProggyForever.ttf";
    auto isRegSetEmbedded = appCfg->fontFiles.first == "embedded" || appCfg->fontFiles.first.empty();
    auto isMonoSetEmbedded = appCfg->fontFiles.second == "embedded" || appCfg->fontFiles.second.empty();
    auto reloadRegular = (isRegLoadedEmbedded && isRegSetEmbedded) ? false : regularName != appCfg->fontFiles.first.substr(appCfg->fontFiles.first.find_last_of("/\\") + 1);
    auto reloadMono = (isMonoLoadedEmbedded && isMonoSetEmbedded) ? false : monoName != appCfg->fontFiles.second.substr(appCfg->fontFiles.second.find_last_of("/\\") + 1);
    if (reloadRegular || reloadMono) {
        ImGui::GetIO().Fonts->Clear();
        appCfg->fontRegular = getFont(appCfg->fontFiles.first);
        appCfg->fontMono = getFont(appCfg->fontFiles.second, false);
        SDL_Log("Reloaded fonts: regular: %s, mono: %s", regularName.data(), monoName.data());
        // SDL_Log("Debug info: regLoadEmb %d monoLoadEmb %d", isRegLoadedEmbedded, isMonoLoadedEmbedded);
        // SDL_Log("Debug info: regSetEmb %d monoSetEmb %d", isRegSetEmbedded, isMonoSetEmbedded);
        // SDL_Log("Debug info: reloadRegular %d reloadMono %d", reloadRegular, reloadMono);
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    /* SDL will clean up the window/renderer for us. */
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    auto state = static_cast<AppState *>(appstate);
    auto cp = state->currentProfile;
    // if (state->selected) {
    //     if (saveSoundPad(cp, state->selected)) {
    //         SDL_Log("Saved profile %s", cp.c_str());
    //     } else {
    //         SDL_Log("Failed to save profile %s", cp.c_str());
    //     }
    // }
    delete[] state->requestStrings;
    delete state->selected;
    delete state;
    saveAppConfig(appCfg);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    MIX_DestroyMixer(mixer);
    MIX_Quit();
    SDL_Quit();
}
