cmake_minimum_required(VERSION 3.21)
project(soundpad LANGUAGES C CXX)

# ============================================================
# НАСТРОЙКИ ПОЛЬЗОВАТЕЛЯ
# ============================================================
# Пример вызова:
#   cmake -DTARGET=win-release -DSTATIC_LINK=ON ..
#
# TARGET может быть одним из:
#   linux-debug | linux-release | win-debug | win-release
#
# ============================================================

if(NOT DEFINED TARGET)
    set(TARGET linux-debug CACHE STRING "Target preset: linux-debug | linux-release | win-debug | win-release")
endif()

if(NOT DEFINED STATIC_LINK)
    set(STATIC_LINK OFF CACHE BOOL "Link SDL statically (default OFF)")
endif()

# ------------------------------------------------------------
# НАСТРОЙКА КОМПИЛЯТОРА И ПЛАТФОРМЫ
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(TOLOWER "${TARGET}" TARGET_LOWER)

#if(TARGET_LOWER MATCHES "^win")
#    message(STATUS "Configuring for Windows (cross-compile via MinGW)")
#
#    set(CMAKE_SYSTEM_NAME Windows)
#    set(CMAKE_SYSTEM_VERSION 1)
#    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
#    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
#    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
#
#    set(IS_WINDOWS TRUE)
#    set(IS_LINUX FALSE)
#else()
#    message(STATUS "Configuring for native Linux build")
#    set(IS_WINDOWS FALSE)
#    set(IS_LINUX TRUE)
#endif()

# ------------------------------------------------------------
# РЕЖИМ СБОРКИ (Debug / Release)
# ------------------------------------------------------------

if(TARGET_LOWER MATCHES "debug")
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Static link: ${STATIC_LINK}")

# ============================================================
# ПУТИ К SDL3 и SDL_mixer
# ============================================================


set(SDL3_INCLUDE_DIR "${SDL3_DIR}/include")
set(SDL3MIXER_INCLUDE_DIR "${SDL3MIXER_DIR}/include")

if(STATIC_LINK)
    if(IS_WINDOWS)
        set(SDL3_LIBRARY "${SDL3_DIR}/lib/SDL3-static.lib")
        set(SDL3MIXER_LIBRARY "${SDL3MIXER_DIR}/lib/SDL3_mixer-static.lib")
    else()
        set(SDL3_LIBRARY "${SDL3_DIR}/lib/libSDL3.a")
        set(SDL3MIXER_LIBRARY "${SDL3MIXER_DIR}/lib/libSDL3_mixer.a")
    endif()
else()
    if(IS_WINDOWS)
        set(SDL3_LIBRARY "${SDL3_DIR}/lib/SDL3.dll")
        set(SDL3MIXER_LIBRARY "${SDL3MIXER_DIR}/lib/SDL3_mixer.dll")
    else()
        set(SDL3_LIBRARY "${SDL3_DIR}/lib/libSDL3.so")
        set(SDL3MIXER_LIBRARY "${SDL3MIXER_DIR}/lib/libSDL3_mixer.so")
    endif()
endif()

include_directories(
    ${SDL3_INCLUDE_DIR}
    ${SDL3MIXER_INCLUDE_DIR}
    vendored/imgui/
    vendored/imgui/backends/
)

# ============================================================
# СБОРКА ПРОЕКТА
# ============================================================

add_executable(${PROJECT_NAME} 
    main.cpp 
    Pad.hpp Pad.cpp
    Utils.hpp Utils.cpp
    Config.hpp Config.cpp
    Font.hpp Font.cpp
    vendored/imgui/imgui.cpp 
    vendored/imgui/imgui_demo.cpp
    vendored/imgui/imgui_draw.cpp
    vendored/imgui/imgui_tables.cpp
    vendored/imgui/imgui_widgets.cpp
    vendored/imgui/backends/imgui_impl_sdl3.cpp
    vendored/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_link_libraries(${PROJECT_NAME}
    ${SDL3_LIBRARY}
    ${SDL3MIXER_LIBRARY}
)

if(IS_LINUX)
    target_link_libraries(${PROJECT_NAME} pthread dl fontconfig)
elseif(IS_WINDOWS)
    target_link_libraries(${PROJECT_NAME} winmm imm32 version)
endif()

# ============================================================
# ОТЛАДОЧНЫЕ СООБЩЕНИЯ
# ============================================================
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "SDL3 include: ${SDL3_INCLUDE_DIR}")
message(STATUS "SDL3_mixer include: ${SDL3MIXER_INCLUDE_DIR}")
message(STATUS "SDL3 lib: ${SDL3_LIBRARY}")
message(STATUS "SDL3_mixer lib: ${SDL3MIXER_LIBRARY}")
