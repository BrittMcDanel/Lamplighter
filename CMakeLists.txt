cmake_minimum_required(VERSION 3.0)

project(engine-test VERSION 0.0.1)

set(CMAKE_GENERATOR "Ninja")

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} -Wextra -ggdb -lm")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} /W3")
endif()

find_package(Threads REQUIRED)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} )

# SDL2
add_subdirectory(third-party/SDL2-2.0.12)
set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

# imgui
set(IMGUI_SOURCE_FILES 
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_impl_sdl.cpp
    third-party/imgui/imgui_impl_opengl3.cpp
    third-party/imgui/imgui_demo.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_tables.cpp
    third-party/imgui/imgui_widgets.cpp
)
include_directories(third-party/imgui)

# asio
include_directories(third-party/asio-1.18.0/include)

# steam networking stack
add_subdirectory(third-party/steam)
include_directories(third-party/steam/include/)

# stb
include_directories(third-party/stb/include)
set(STB 
   third-party/stb/src/stb_image.c
   third-party/stb/src/stb_image_write.c
   third-party/stb/src/stb_truetype.c
)

# fmt
add_subdirectory(third-party/fmt-7.1.3)
include_directories(third-party/fmt-7.1.3/include)

# our files
include_directories(include)
include_directories(include/ui)
include_directories(include/rapidjson)
include_directories(include/uuid)

# main
set(SOURCE_FILES
    src/general/backward.cpp
    src/general/utils.cpp
    src/general/engine.cpp
    src/general/sprite.cpp
    src/general/game.cpp
    src/general/editor.cpp
    src/general/camera.cpp
    src/general/tile.cpp
    src/general/map.cpp
    src/general/network.cpp
    src/general/unit_sprite.cpp
    src/general/unit.cpp
    src/general/ability.cpp
    src/general/input_events.cpp
    src/general/tween.cpp
    src/general/pathfinder.cpp
    src/general/serializer.cpp
    src/general/spritesheet.cpp
    src/general/ai_walk_path.cpp
    src/general/game_events.cpp
    src/general/text.cpp
    src/general/fixed_sprite.cpp
    src/general/dialogue.cpp
    src/general/item.cpp
    src/general/tweenable_sprite.cpp
    src/general/treasure_chest.cpp
    src/general/inventory.cpp
    src/general/utils_game.cpp
    src/general/assets.cpp
    src/general/battle.cpp
    src/general/ability_targets.cpp
    src/general/status_effect.cpp
    src/general/ui/battle_text.cpp
    src/general/ui/dialogue_box.cpp
    src/general/ui/unit_ui.cpp
    src/general/ui/expandable_sprite.cpp
    src/general/ui/ui_sprite.cpp
    src/general/ui/ui.cpp
    src/general/ui/item_pickup_list.cpp
    src/general/ui/inventory_window.cpp
    src/general/ui/equip_window.cpp
    src/general/ui/shop_window.cpp
    src/general/ui/bottom_navbar.cpp
    src/general/ui/utils_ui.cpp
    src/general/ui/tooltip.cpp
    src/general/ui/alert.cpp
    src/general/ui/portraits.cpp
    src/general/ui/move_point_icons.cpp
    src/general/ui/action_point_display.cpp
    src/general/ui/crosshairs.cpp
    src/general/ui/turn_order_display.cpp
    src/general/ui/skill_tree_window.cpp
)

set(CMAKE_CXX_STANDARD 17)

add_executable(main src/main.cpp ${SOURCE_FILES} ${STB} ${IMGUI_SOURCE_FILES})
set_property(TARGET main PROPERTY CMAKE_CXX_STANDARD 17)

target_link_libraries(main ${OPENGL_LIBRARIES} SDL2-static SDL2main -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lGLEW GameNetworkingSockets::GameNetworkingSockets fmt::fmt)

add_executable(run src/run.cpp ${SOURCE_FILES} ${STB} ${IMGUI_SOURCE_FILES})
set_property(TARGET run PROPERTY CMAKE_CXX_STANDARD 17)

target_link_libraries(run ${OPENGL_LIBRARIES} SDL2-static SDL2main -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lGLEW GameNetworkingSockets::GameNetworkingSockets fmt::fmt)