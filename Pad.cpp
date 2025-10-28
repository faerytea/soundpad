#include "Pad.hpp"

SDLLoopProp Pad::loop = SDLLoopProp();

bool Pad::loadSound(const std::string &path) {
    unloadSound();
    audio = MIX_LoadAudio(mixer, path.c_str(), true);
    if (audio) {
        auto lastSlash = path.find_last_of("/\\");
        name = path.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
        for (auto t : track) {
            if (!MIX_StopTrack(t, 0)) {
                SDL_Log("Failed to stop track on %c: %s", letter, SDL_GetError());
            }
            if (!MIX_SetTrackAudio(t, audio)) {
                SDL_Log("Failed to set track audio on %c: %s", letter, SDL_GetError());
            }
        }
    }
    return audio != nullptr;
}

bool Pad::processInput() {
    bool lmbDown = ImGui::IsMouseDown(0);
    bool rmb = ImGui::IsMouseClicked(1);
    bool hovered = ImGui::IsItemHovered();
    bool lmbReleased = ImGui::IsMouseReleased(0);
    bool active = ImGui::IsKeyDown(key) || (lmbDown && hovered);
    bool activate = active && !wasActive;
    bool release = !active && wasActive;
    
    // SDL_Log("Pad %c: lmbDown=%d rmb=%d hovered=%d lmbReleased=%d active=%d activate=%d release=%d wasActive=%d", letter, lmbDown, rmb, hovered, lmbReleased, active, activate, release, wasActive);
    if (activate) {
        auto io = ImGui::GetIO();
        auto ctrl = io.KeyCtrl;
        auto shift = io.KeyShift;
        auto alt = io.KeyAlt;
        request = table[ctrl ? 1 : 0][shift ? 1 : 0][alt ? 1 : 0][(state == IDLE || state == PAUSED) ? 0 : 1];
        SDL_Log("Pad %c activated: request=%d, ctrl = %d, shift = %d, alt = %d, state = %d", letter, request, ctrl, shift, alt, state);
    } else if (ImGui::IsKeyReleased(key) || (hovered && lmbReleased)) {
        SDL_Log("Catch release, state=%d, request=%d", state, request);
        if (state == LOOPED && request == HELD) {
            request = STOP;
        }
        SDL_Log("Pad %c key released: request=%d", letter, request);
    }
    if (hovered) {
        // check scroll
    }
    wasActive = active;
    return rmb && hovered;
}

void Pad::unloadSound() {
    if (audio) {
        MIX_DestroyAudio(audio);
        audio = nullptr;
        name = "";
    }
}

Pad::~Pad() {
    unloadSound();
    for (auto t : track) {
        MIX_DestroyTrack(t);
    }
    // SDL_Log("Pad %c destroyed", letter);
}

void Pad::fulfillRequest() {
    switch (request) {
    case NONE:
        break;
    case ONE_SHOT: {
        if (audio == nullptr) {
            break;
        }
        // find an idle track
        MIX_Track *idle = getIdleTrack();
        if (idle == nullptr) {
            break;
        }
        SDL_Log("Shouting track on %c", letter);
        if (!MIX_PlayTrack(idle, 0)) {
            SDL_Log("Failed to play track on %c: %s", letter, SDL_GetError());
            break;
        }
        break;
    }
    case STOP: {
        for (auto t : track) {
            SDL_Log("Stopping track on %c", letter);
            if (MIX_TrackPlaying(t) || MIX_TrackPaused(t)) {
                if (!MIX_StopTrack(t, 0)) {
                    SDL_Log("Failed to stop track on %c: %s", letter, SDL_GetError());
                }
            }
        }
        break;
    }
    case PAUSE: {
        for (auto t : track) {
            SDL_Log("Pausing track on %c", letter);
            if (MIX_TrackPlaying(t)) {
                if (!MIX_PauseTrack(t)) {
                    SDL_Log("Failed to pause track on %c: %s", letter, SDL_GetError());
                }
            }
        }
        break;
    }
    case RESUME: {
        for (auto t : track) {
            SDL_Log("Resuming track on %c", letter);
            if (MIX_TrackPaused(t)) {
                if (!MIX_ResumeTrack(t)) {
                    SDL_Log("Failed to resume track on %c: %s", letter, SDL_GetError());
                }
            }
        }
        break;
    }
    case LOOP: {
        if (!audio) {
            break;
        }
        MIX_Track *t = getIdleTrack();
        if (!t) {
            break;
        }
        SDL_Log("Playing looped track on %c", letter);
        if (!MIX_PlayTrack(t, loop)) {
            SDL_Log("Failed to play track on %c: %s", letter, SDL_GetError());
            break;
        }
        break;
    }
    case HELD: {
        if (state == PLAYING || state == LOOPED) {
            // do nothing, just wait for key release
        } else if (!audio) {
            break;
        } else {
            MIX_Track *t = getIdleTrack();
            if (!t) {
                break;
            }
            SDL_Log("Playing looped track on %c (%ld loops)", letter, SDL_GetNumberProperty(loop, MIX_PROP_PLAY_LOOPS_NUMBER, -2));
            if (!MIX_PlayTrack(t, loop)) {
                SDL_Log("Failed to play track on %c: %s", letter, SDL_GetError());
                break;
            }
        }
        break;
    }
    }
    request = request == HELD ? HELD : NONE;
}

void Pad::resolveState() {
    bool anyPlaying = false;
    bool anyPaused = false;
    bool anyLooped = false;
    for (auto t : track) {
        if (MIX_TrackPlaying(t)) {
            anyPlaying = true;
            if (MIX_TrackLooping(t)) {
                anyLooped = true;
                // SDL_Log("Track on %c is looped", letter);
            }
        }
        if (MIX_TrackPaused(t)) {
            anyPaused = true;
        }
    }
    if (anyPlaying) {
        state = anyLooped ? LOOPED : PLAYING;
    } else if (anyPaused) {
        state = PAUSED;
    } else {
        state = IDLE;
    }
}

MIX_Track *Pad::getIdleTrack() {
    MIX_Track *idle = nullptr;
    for (auto t : track) {
        if (!MIX_TrackPlaying(t) && !MIX_TrackPaused(t)) {
            idle = t;
            break;
        }
    }
    if (idle == nullptr) {
        SDL_Log("Creating new track on %c", letter);
        idle = MIX_CreateTrack(mixer);
        if (idle) {
            track.push_back(idle);
            MIX_SetTrackAudio(idle, audio);
        } else {
            SDL_Log("Failed to create new track on %c: %s", letter, SDL_GetError());
        }
    }
    return idle;
}

bool Pad::render(ImVec2 &size, bool interactive, ImFont *letterFont, float fontSize) {
    ImDrawList *draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::PushID(letter);

    ImGui::InvisibleButton("", size);

    ImU32 bg, bright;
    switch (state) {
    case IDLE:
        bg = IM_COL32(10, 10, 10, 255);
        bright = IM_COL32(20, 20, 20, 255);
        break;
    case PLAYING:
        bg = IM_COL32(10, 80, 10, 255);
        bright = IM_COL32(20, 100, 20, 255);
        break;
    case PAUSED:
        bg = IM_COL32(80, 80, 10, 255);
        bright = IM_COL32(100, 100, 20, 255);
        break;
    case LOOPED:
        bg = IM_COL32(80, 0, 100, 255);
        bright = IM_COL32(100, 5, 120, 255);
        break;
    default:
        break;
    }

    auto percent = volume();

    auto pMax = ImVec2(pos.x + size.x, pos.y + size.y),
         pMidT = ImVec2(pos.x + size.x * percent / 2, pos.y),
         pMidB = ImVec2(pos.x + size.x * percent / 2, pos.y + size.y);

    draw->AddRectFilled(pos, pMidB, bg);
    draw->AddRectFilled(pMidT, pMax, bright);

    if (ImGui::IsItemHovered()) {
        draw->AddRect(pos, pMax, IM_COL32(0, 150, 0, 255), 0, 0, 1);
    }

    ImGui::PushFont(letterFont, fontSize);// * 10 / 8);
    auto letterSize = ImGui::CalcTextSize(&letter, &letter + 1);
    auto letterTL = ImVec2(pos.x + (size.x - letterSize.x)/2, pos.y + (size.y - letterSize.y)/2);
    // draw->AddRect(letterTL, ImVec2(letterTL.x + letterSize.x, letterTL.y + letterSize.y), IM_COL32(150, 0, 150, 255), 0, 0, 1);
    // draw->AddText(nullptr, size.y, pos, IM_COL32(100, 100, 100, 255), &label, &label + 1);
    letterFont->RenderChar(draw, fontSize, letterTL, IM_COL32(200, 200, 200, 255), letter);
    // draw->AddText(letterTL, IM_COL32(80, 80, 80, 255), &letter, &letter + 1);
    ImGui::PopFont();
    auto nameSize = ImGui::CalcTextSize(name.c_str(), 0, false, size.x);
    auto namePos = ImVec2(pos.x + (size.x - nameSize.x) / 2, pos.y + (size.y - nameSize.y) / 2);
    draw->AddRectFilled(namePos, ImVec2(namePos.x + nameSize.x, namePos.y + nameSize.y), IM_COL32(128, 128, 128, 128));
    draw->AddText(nullptr, 0, namePos, IM_COL32(255, 255, 255, 255), name.c_str(), nullptr, size.x);

    bool res = interactive ? processInput() : false;
    fulfillRequest();
    resolveState();

    ImGui::PopID();
    return res;
}

bool Pad::volume(float volume) {
    bool res = true;
    for (auto &t : track) {
        res &= MIX_SetTrackGain(t, volume);
    }
    return res;
}

#ifdef _MSC_VER // 0.f / 0.f is not permitted, so we need limits<float>::nan to acquire nan
#include <limits>
#endif // _MSC_VER

float Pad::volume() {
    if (track.empty()) {
        SDL_Log("No tracks on pad %c?..", letter);
#ifdef _MSC_VER
        return std::numeric_limits<float>::quiet_NaN();
#else // any sane compiler
        return 0.f / 0.f;
#endif // _MSC_VER
    }
    return MIX_GetTrackGain(track[0]);
}
