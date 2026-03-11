cmake_minimum_required(VERSION 3.12)
project(soundpad
    LANGUAGES C CXX
    VERSION 1.3.0
    DESCRIPTION "Simple soundboard application built with SDL3 and ImGui."
    HOMEPAGE_URL "https://github.com/faerytea/soundpad/"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.3)
    set(PROJECT_SPDX_LICENSE "GPL-3.0-or-later")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 3.4.0 REQUIRED)
find_package(SDL3_image 3.4.0 REQUIRED)
find_package(SDL3_mixer 3.2.0 REQUIRED)

# icon & links
find_package(ImageMagick COMPONENTS convert) # for icon set generation

if(WIN32)
    set(PLATFORM_SPECIFIC "WIN32")
elseif(APPLE)
    set(PLATFORM_SPECIFIC "MACOSX_BUNDLE")
else()
    set(PLATFORM_SPECIFIC "")
endif()

if(ImageMagick_convert_FOUND)
    set(SRC_IMAGE "${PROJECT_SOURCE_DIR}/res/soundpad icon 800.png")
    #get_target_property(APP_OUTPUT_NAME ${PROJECT_NAME} OUTPUT_NAME)
    if(WIN32)
        enable_language("RC")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.ico
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -write mpr:iconimg +delete
                    "(" mpr:iconimg -resize 256x256 ")"
                    "(" mpr:iconimg -resize 128x128 ")"
                    "(" mpr:iconimg -resize 64x64 ")"
                    "(" mpr:iconimg -resize 48x48 ")"
                    "(" mpr:iconimg -resize 32x32 ")"
                    "(" mpr:iconimg -resize 16x16 ")"
                    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.ico
            DEPENDS ${SRC_IMAGE}
            COMMENT "Generating icon set for ${PROJECT_NAME}"
        )
        install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.ico DESTINATION bin)
        set(APP_ICON "\"${PROJECT_NAME}.ico\"")
        set(ICONS ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.rc ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.ico)
    elseif(APPLE)
    else() # assume linux
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/")
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/32x32/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/32x32/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 32x32 ${PROJECT_BINARY_DIR}/share/icons/hicolor/32x32/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 32x32 icon for ${PROJECT_NAME}"
        )
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/48x48/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/48x48/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 48x48 ${PROJECT_BINARY_DIR}/share/icons/hicolor/48x48/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 48x48 icon for ${PROJECT_NAME}"
        )
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/64x64/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 64x64 ${PROJECT_BINARY_DIR}/share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 64x64 icon for ${PROJECT_NAME}"
        )
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/128x128/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/128x128/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 128x128 ${PROJECT_BINARY_DIR}/share/icons/hicolor/128x128/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 128x128 icon for ${PROJECT_NAME}"
        )
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/256x256/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/256x256/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 256x256 ${PROJECT_BINARY_DIR}/share/icons/hicolor/256x256/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 256x256 icon for ${PROJECT_NAME}"
        )
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/512x512/apps/")
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png
            COMMAND ${ImageMagick_convert_EXECUTABLE} ${SRC_IMAGE} -resize 512x512 ${PROJECT_BINARY_DIR}/share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png
            MAIN_DEPENDENCY ${SRC_IMAGE}
            COMMENT "Generating 512x512 icon for ${PROJECT_NAME}"
        )
        configure_file(${PROJECT_SOURCE_DIR}/res/soundpad.desktop.in ${PROJECT_BINARY_DIR}/share/applications/${PROJECT_NAME}.desktop @ONLY)
        install(FILES ${PROJECT_BINARY_DIR}/share/applications/${PROJECT_NAME}.desktop DESTINATION share/applications/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/32x32/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/32x32/apps/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/48x48/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/48x48/apps/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/64x64/apps/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/128x128/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/128x128/apps/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/256x256/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/256x256/apps/)
        install(FILES ${PROJECT_BINARY_DIR}/share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png DESTINATION share/icons/hicolor/512x512/apps/)
        set(ICONS 
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/32x32/apps/${PROJECT_NAME}.png
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/48x48/apps/${PROJECT_NAME}.png
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/128x128/apps/${PROJECT_NAME}.png
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/256x256/apps/${PROJECT_NAME}.png
            ${PROJECT_BINARY_DIR}/share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png
        )
        set(APP_ICON "\"../share/icons/hicolor/64x64/apps/${PROJECT_NAME}.png\"")
        set(APP_ICON_ALTER "\"../share/icons/hicolor/512x512/apps/${PROJECT_NAME}.png\"")
    endif()
else()
    message(WARNING "No ImageMagick (aka 'magick') found, icons won't be generated")
    set(ICONS "")
endif()

include_directories(
    vendored/imgui/
    vendored/imgui/backends/
)

add_executable(${PROJECT_NAME} ${PLATFORM_SPECIFIC}
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
    ${ICONS}
)

if(APP_ICON)
    # tell app that we have an icon
    target_compile_definitions(${PROJECT_NAME} PRIVATE "APP_ICON=${APP_ICON}" "APP_ICON_ALTER=${APP_ICON_ALTER}")
    if(APP_ICON_ALTER)
        # if we have an alternate icon, tell app about it too
        target_compile_definitions(${PROJECT_NAME} PRIVATE "APP_ICON_ALTER=${APP_ICON_ALTER}")
    endif()
endif()

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(UNIX AND NOT APPLE)
    find_package(Fontconfig REQUIRED)
    target_link_libraries(${PROJECT_NAME} Fontconfig::Fontconfig)
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
set(CPACK_PACKAGE_CONTACT "faerytea@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})

set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})

set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/COPYING)
install(FILES COPYING DESTINATION share/doc/${PROJECT_NAME})

# debian specific
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl3-0 (>= 3.4.0), libsdl3-mixer0 (>= 3.2.0), libsdl3-image0 (>= 3.4.0), fontconfig")
set(CPACK_DEBIAN_PACKAGE_SECTION sound)

include(CPack)
