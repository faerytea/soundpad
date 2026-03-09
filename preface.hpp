#ifndef PREFACE_HPP
#define PREFACE_HPP

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_image/SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

inline SDL_Renderer *renderer = nullptr;

#endif // PREFACE_HPP
