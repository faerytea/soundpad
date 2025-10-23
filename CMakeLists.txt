cmake_minimum_required(VERSION 3.21)
project(soundpad LANGUAGES C CXX)

if(NOT DEFINED STATIC) 
    set(STATIC OFF CACHE BOOL "Link SDL statically (default OFF)")
endif()

if(STATIC)
    set(SUFFIX "-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(SUFFIX "-shared${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

if(SDL3_DIR)
    message(STATUS "Using SDL3_DIR: ${SDL3_DIR}")
    set(SDL3_LIB ${SDL3_DIR}/lib/SDL3${SUFFIX})
    set(SDL3_HEADERS ${SDL3_DIR}/include/)
    if(NOT DEFINED SDL3_MIXER_DIR)
        set(SDL3_MIXER_DIR ${SDL3_DIR} CACHE PATH "Path to SDL_mixer 3 installation")
    endif()
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-${SUFFIX})
    set(SDL3_LIB SDL3::SDL3)
    set(SDL3_HEADERS ${SDL3_LIB}/include/)
    message(STATUS "Using default SDL3_DIR: ${SDL3_DIR}")
endif()

if(SDL3_MIXER_DIR)
    message(STATUS "Using SDL3_MIXER_DIR: ${SDL3_MIXER_DIR}")
    set(SDL3_MIXER_LIB ${SDL3_MIXER_DIR}/lib/SDL3_mixer${SUFFIX})
    set(SDL3_MIXER_HEADERS ${SDL3_MIXER_DIR}/include/)
else()
    find_package(SDL3_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL3_mixer-${SUFFIX})
    set(SDL3_MIXER_LIB SDL3_mixer::SDL3_mixer)
    set(SDL3_MIXER_HEADERS ${SDL3_MIXER_LIB}/include/)
    message(STATUS "Using default SDL3_MIXER_DIR: ${SDL3_MIXER_DIR}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Static link: ${STATIC}")

include_directories(
    ${SDL3_HEADERS}
    ${SDL3_MIXER_HEADERS}
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
    ${SDL3_LIB}
    ${SDL3_MIXER_LIB}
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
