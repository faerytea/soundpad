# Soundpad

Simple soundpad application. 

Sounds are loaded through SDL_mixer, so format support may vary,
but usually include wav, mp3 and flac.

## Usage

Right-click on pad to change its settings (file, volume and state transitions).

Left-click on pad (or press a corresponding key) to activate it. 
Ctrl, alt and shift modifiers may change behavior.

Can play a sound, pause/resume, loop, stop and play-while-pressed.

## Building

You'll need [SDL 3.3+](https://github.com/libsdl-org/SDL) and [SDL_mixer 3+](https://github.com/libsdl-org/SDL_mixer)
build as static or shared libs. Since SDL_mixer 3 is not released yet _and_ requires SDL 3.3+
(which is not a stable release), you probably need to build them.
Building SDL3 isn't hard, almost same process can be applied to SDL_mixer 3.

Dear ImGui provided as a submodule, since it is preffered way to include it,
so just ensure that submodule is downloaded.
