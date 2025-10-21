#ifndef PAD_HPP
#define PAD_HPP

#include "preface.hpp"
#include "Utils.hpp"
#include <string>
#include <vector>

enum PadState {
    IDLE,
    PLAYING,
    PAUSED,
    LOOPED,
};

enum PadStateRequest {
    NONE,
    ONE_SHOT,
    STOP,
    PAUSE,
    RESUME,
    LOOP,
    HELD,
};

class SDLLoopProp {
public:
    SDL_PropertiesID id;
    SDLLoopProp(): id(SDL_CreateProperties()) {
        if (!SDL_SetNumberProperty(id, MIX_PROP_PLAY_LOOPS_NUMBER, -1)) {
            SDL_Log("Failed to set loop property %s", SDL_GetError());
        }
    }
    ~SDLLoopProp() {
        if (id != 0) {
            SDL_DestroyProperties(id);
        }
    }
    operator SDL_PropertiesID() const { return id; }
};

class Pad {
public:
    const char letter;
    const ImGuiKey key;
    PadState state = IDLE;
    PadStateRequest request = NONE;
    PadStateRequest table[2][2][2][2] = {
        {   // NO ctrl
            {   // NO shift
                {ONE_SHOT, ONE_SHOT}, // NO alt
                {HELD, NONE}      // alt
            },
            {   // shift
                {LOOP, STOP}, // NO alt
                {NONE, NONE}      // alt
            }
        },
        {   // ctrl
            {   // NO shift
                {RESUME, PAUSE}, // NO alt
                {NONE, NONE}   // alt
            },
            {   // shift
                {NONE, NONE}, // NO alt
                {NONE, NONE}  // alt
            }
        }
    };

    MIX_Mixer *mixer;
    std::vector<MIX_Track *> track = std::vector<MIX_Track *>();

    MIX_Audio *audio = nullptr;
    std::string name = "";

    Pad(const char letter, MIX_Mixer *mixer)
        : letter(letter)
        , key(ImGuiKeyFromChar(letter))
        , mixer(mixer)
    {
        track.reserve(8);
        track.push_back(MIX_CreateTrack(mixer));
    }
    ~Pad();
    Pad(Pad &&o)
        : letter(o.letter)
        , key(o.key)
        , state(o.state)
        , request(o.request)
        , table{o.table[0][0][0][0], o.table[0][0][0][1], o.table[0][0][1][0], o.table[0][0][1][1],
                o.table[0][1][0][0], o.table[0][1][0][1], o.table[0][1][1][0], o.table[0][1][1][1],
                o.table[1][0][0][0], o.table[1][0][0][1], o.table[1][0][1][0], o.table[1][0][1][1],
                o.table[1][1][0][0], o.table[1][1][0][1], o.table[1][1][1][0], o.table[1][1][1][1]}
        , mixer(o.mixer)
        , track(std::move(o.track))
        , audio(o.audio)
        , name(std::move(o.name))
        , wasActive(o.wasActive)
    {
        o.mixer = nullptr;
        o.audio = nullptr;
        // SDL_Log("Pad %c moved", letter);
    }

    bool loadSound(const std::string &path);

    bool render(ImVec2 &size, bool interactive, ImFont *letterFont);

    bool processInput();

    void fulfillRequest();

    void resolveState();

    void unloadSound();

    bool volume(float volume);

    float volume();
private:
    MIX_Track *getIdleTrack();
    bool wasActive = false;
    static SDLLoopProp loop;
};

typedef std::vector<std::vector<Pad> > SoundPad;

#endif // PAD_HPP
