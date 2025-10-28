#include "Config.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <unordered_map>

#include "Font.hpp"

std::string_view trim(std::string_view s) {
    size_t start = 0;
    while (start < s.size() && isspace(s[start])) ++start;
    size_t end = s.size();
    while (end > start && isspace(s[end - 1])) --end;
    return s.substr(start, end - start);
}

ImFont *getFont(std::string &path) {
    auto &io = ImGui::GetIO();
    if (path == "embedded" || path.empty()) {
        return io.Fonts->AddFontDefault();
    }
    ImFont *font = io.Fonts->AddFontFromFileTTF(path.c_str());
    if (!font) {
        SDL_Log("Failed to load font from %s, using default", path.c_str());
        font = io.Fonts->AddFontDefault();
    }
    return font;
}

AppConfig *loadAppConfig() {
    auto res = new AppConfig();
    std::string prefsDir(SDL_GetPrefPath("faerytea", "soundpad"));
    // appCfg.appdir = prefsDir;
    // std::filesystem::path appDir(prefsDir);
    //new (&appCfg.appdir) std::filesystem::path(prefsDir);
    std::filesystem::path appDir = std::filesystem::u8path(prefsDir);
    res->appdir = appDir;
    res->baseRoot = std::filesystem::u8path(SDL_GetUserFolder(SDL_Folder::SDL_FOLDER_MUSIC));
    if (!std::filesystem::exists(appDir)) {
        if (!std::filesystem::create_directories(appDir)) {
            SDL_Log("Failed to create prefs dir %s", prefsDir.c_str());
            return nullptr;
        }
    }
    SDL_Log("App dir: %s", res->appdir.u8string().c_str());
    // load config
    std::ifstream cfg(appDir / "config.ini");
    std::string monoTTF = "", regularTTF = "";
    if (!cfg.is_open()) {
        SDL_Log("No app config found, using defaults");
    } else {
        std::string line;
        while (std::getline(cfg, line)) {
            auto kv = trim(line);
            if (kv.empty() || kv[0] == '#') continue;
            auto eqIx = kv.find('=');
            if (eqIx == std::string::npos) {
                SDL_Log("Invalid config line: %s", line.c_str());
                continue;
            }
            auto key = trim(kv.substr(0, eqIx));
            auto value = trim(kv.substr(eqIx + 1));
            if (key == "autosave") {
                res->autosave = (value == "1" || value == "true" || value == "yes");
            } else if (key == "baseroot") {
                auto tmpPath = std::filesystem::u8path(value);
                if (std::filesystem::exists(tmpPath)) {
                    res->baseRoot = tmpPath;
                } else {
                    SDL_Log("Path %s is not valid", std::string(value).c_str());
                }
            } else if (key == "monofont") {
                if (std::filesystem::exists(value)) {
                    monoTTF = value;
                }
            } else if (key == "font") {
                if (std::filesystem::exists(value)) {
                    regularTTF = value;
                }
            } else {
                SDL_Log("Unknown config key: %s", key.data());
            }
        }
        cfg.close();
    }
    if (monoTTF.empty() && regularTTF.empty()) {
        auto defaultTTFs = getDefaultFontFiles();
        if (regularTTF.empty()) regularTTF = defaultTTFs.first;
        if (monoTTF.empty()) monoTTF = defaultTTFs.second;
    }
    SDL_Log("Loading '%s' and '%s'", regularTTF.c_str(), monoTTF.c_str());
    auto &io = ImGui::GetIO();
    auto *regular = getFont(regularTTF);
    auto *mono = getFont(monoTTF);
    if (regular->GetDebugName() == "ProggyClean.ttf") {
        regularTTF = "embedded";
    }
    if (mono->GetDebugName() == "ProggyClean.ttf") {
        monoTTF = "embedded";
    }
    res->fontFiles = std::pair(regularTTF, monoTTF);
    SDL_Log("Using '%s' as monospace font", mono->GetDebugName());
    SDL_Log("Using '%s' as regular font", regular->GetDebugName());
    res->fontRegular = regular;
    res->fontMono = mono;
    // load profiles
    std::filesystem::path profiles(appDir / "profiles");
    if (std::filesystem::exists(profiles)) {
        if (std::filesystem::is_directory(profiles)) {
            for (auto &p : std::filesystem::directory_iterator(profiles)) {
                if (p.is_regular_file()) {
                    res->profiles.push_back(p);
                }
            }
        } else {
            SDL_Log("Profiles path %s exists but is not a directory", profiles.u8string().c_str());
        }
    } else {
        if (!std::filesystem::create_directories(profiles)) {
            SDL_Log("Failed to create profiles dir %s", profiles.u8string().c_str());
        }
    }
    SDL_Log("Got %ld profiles", res->profiles.size());
    return res;
}

SoundPad *createDefault(MIX_Mixer *mixer) {
    auto psp = new SoundPad();
    psp->reserve(4);
    psp->emplace_back(std::vector<Pad>());
    auto &nums = psp->at(0);
    nums.reserve(10);
    for (char c : std::string("1234567890")) {
        nums.emplace_back(Pad(c, mixer));
    }
    psp->emplace_back(std::vector<Pad>());
    auto &qwe = psp->at(1);
    qwe.reserve(10);
    for (char c : std::string("QWERTYUIOP")) {
        qwe.emplace_back(Pad(c, mixer));
    }
    psp->emplace_back(std::vector<Pad>());
    auto &asd = psp->at(2);
    asd.reserve(9);
    for (char c : std::string("ASDFGHJKL")) {
        asd.emplace_back(Pad(c, mixer));
    }
    psp->emplace_back(std::vector<Pad>());
    auto &zxc = psp->at(3);
    zxc.reserve(7);
    for (char c : std::string("ZXCVBNM")) {
        zxc.emplace_back(Pad(c, mixer));
    }
    return psp;
}

const int ctrl = 1, shift = 2, alt = 4, playing = 8;

SoundPad *loadSoundPad(std::filesystem::path &path, MIX_Mixer *mixer) {
    SDL_Log("Loading soundpad config from %s", path.u8string().c_str());
    std::ifstream cfg(path);
    if (!cfg.is_open()) {
        SDL_Log("Failed to open pad config %s", path.u8string().c_str());
        return createDefault(mixer);
    }

    std::string line;

    // Load layout
    std::vector<std::string> rows;
    rows.reserve(4);
    SDL_Log("Loading layout:");
    while (std::getline(cfg, line)) {
        if (line.empty()) break;
        bool allSpace = true;
        for (auto &c : line) {
            c = toupper(c);
            if (isalnum(c)) allSpace = false;
        }
        if (allSpace) break;
        rows.push_back(line);
        SDL_Log("\t'%s'", line.c_str());
    }
    SDL_Log("Loaded %ld rows", rows.size());
    if (rows.empty()) {
        SDL_Log("No rows in config %s, creating default", path.u8string().c_str());
        return createDefault(mixer);
    }

    // Init soundpad
    SoundPad *pad = new SoundPad();
    std::unordered_map<char, Pad *> padMap;
    pad->reserve(rows.size());
    for (auto &row : rows) {
        pad->emplace_back(std::vector<Pad>());
        auto &padRow = pad->back();
        padRow.reserve(row.size());
        for (auto c : row) {
            if (isspace(c)) continue;
            padRow.emplace_back(Pad(c, mixer));
            padMap[c] = &padRow.back();
        }
    }

    // Read keys
    std::filesystem::path base = path.parent_path() / path.stem();
    while (std::getline(cfg, line)) {
        if (line.empty()) continue;
        char c = toupper(line[0]);
        auto pp = padMap[c];
        if (!pp) {
            SDL_Log("No pad for key %c in config %s, while its config exists", c, path.u8string().c_str());
            while (std::getline(cfg, line) && !line.empty()); // skip to next
            continue;
        }
        auto songPath = line.substr(2);
        if (!songPath.empty() && pp->loadSound((base / songPath).u8string())) {
            SDL_Log("Loaded sound %s on pad %c", pp->name.c_str(), c);
        } else {
            SDL_Log("Failed to load sound %s on pad %c", songPath.c_str(), c);
        }
        if (!std::getline(cfg, line) || line.empty()) continue;
        // Loading transitions
        for (unsigned i = 0; i < line.size() && i < 16; ++i) {
            c = tolower(line[i]);
            PadStateRequest r = NONE;
            switch (c) {
            case 'o':
                r = ONE_SHOT;
                break;
            case 's':
                r = STOP;
                break;
            case 'p':
                r = PAUSE;
                break;
            case 'r':
                r = RESUME;
                break;
            case 'l':
                r = LOOP;
                break;
            case 'h':
                r = HELD;
                break;
            case 'n':
            case ' ':
                break;
            default:
                SDL_Log("Unknown request char %c for pad %c in config %s", c, pp->letter, path.u8string().c_str());
                break;
            }
            pp->table[(i & ctrl)][(i & shift) >> 1][(i & alt) >> 2][(i & playing) >> 3] = r;
        }
        // loading volume
        if (!std::getline(cfg, line) || line.empty()) continue;
        float volume;
        std::stringstream vars(line);
        vars >> volume;
        pp->volume(volume);
        SDL_Log("Volume of %c is %.3f", pp->letter, volume);
    }

    return pad;
}

bool saveSoundPad(std::filesystem::path &path, SoundPad *pad) {
    std::ofstream cfg(path);
    if (!cfg.is_open()) {
        SDL_Log("Failed to open pad config %s for writing", path.u8string().c_str());
        return false;
    }

    // Write layout
    for (auto &row : *pad) {
        for (auto &p : row) {
            cfg << p.letter;
        }
        cfg << std::endl;
    }
    cfg << std::endl;

    // Write keys
    for (auto &row : *pad) {
        for (auto &p : row) {
            cfg << p.letter << " " << p.name << std::endl;
            for (int i = 0; i < 16; ++i) {
                PadStateRequest r = p.table[(i & ctrl)][(i & shift) >> 1][(i & alt) >> 2][(i & playing) >> 3];
                char c = ' ';
                switch (r) {
                case NONE:
                    c = 'n';
                    break;
                case ONE_SHOT:
                    c = 'o';
                    break;
                case STOP:
                    c = 's';
                    break;
                case PAUSE:
                    c = 'p';
                    break;
                case RESUME:
                    c = 'r';
                    break;
                case LOOP:
                    c = 'l';
                    break;
                case HELD:
                    c = 'h';
                    break;
                default:
                    SDL_Log("Unknown request %d for pad %c in config %s", r, p.letter, path.u8string().c_str());
                    break;
                }
                cfg << c;
            }
            cfg << std::endl 
                << p.volume()
                << std::endl << std::endl;
        }
    }

    return true;
}

bool saveAppConfig(AppConfig *cfg) {
    auto &root = cfg->appdir;
    std::ofstream app(root / "config.ini");
    app << "autosave=" << cfg->autosave << std::endl;
    app << "baseroot=" << cfg->baseRoot.u8string() << std::endl;
    app << "font=" << cfg->fontFiles.first << std::endl;
    app << "monofont=" << cfg->fontFiles.second << std::endl;
    app.close();
    return true;
}
