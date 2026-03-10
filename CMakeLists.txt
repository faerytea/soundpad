cmake_minimum_required(VERSION 3.12)
if(CMAKE_VERSION VERSION_LESS 4.3)
    project(soundpad 
        LANGUAGES C CXX
        VERSION 1.2.5
        HOMEPAGE_URL "https://github.com/faerytea/soundpad"
    )
else()
    project(soundpad 
        LANGUAGES C CXX
        VERSION 1.2.5
        HOMEPAGE_URL "https://github.com/faerytea/soundpad/"
        SPDX_LICENSE "GPL-3.0-or-later"
    )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 3.4.0 REQUIRED)
find_package(SDL3_image 3.4.0 REQUIRED)
find_package(SDL3_mixer 3.2.0 REQUIRED)

include_directories(
    vendored/imgui/
    vendored/imgui/backends/
)

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

if(MSVC)
    set(WIN32_EXECUTABLE ON)
endif()

if(UNIX AND NOT APPLE)
    find_package(Fontconfig)
    if(Fontconfig_FOUND)
        target_link_libraries(${PROJECT_NAME} Fontconfig::Fontconfig)
    endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_compile_definitions(${PROJECT_NAME} PRIVATE "SOUNDPAD_VERSION=\"${PROJECT_VERSION}\"")

target_link_libraries(${PROJECT_NAME}
    SDL3::SDL3
    SDL3_mixer::SDL3_mixer
    SDL3_image::SDL3_image
)

# install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
if(WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION bin)
endif()

# пакет
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "faerytea")
set(CPACK_PACKAGE_CONTACT "t.me/faerytea")
set(CPACK_PACKAGE_DESCRIPTION "Soundpad is a simple soundboard application built with SDL3 and ImGui.")

set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})

set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/COPYING)
install(FILES COPYING DESTINATION share/doc/${PROJECT_NAME})

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl3-0 (>= 3.4.0), libsdl3-mixer0 (>= 3.2.0), libsdl3-image0 (>= 3.4.0), fontconfig")
set(CPACK_DEBIAN_PACKAGE_SECTION sound)

include(CPack)
