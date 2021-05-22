#include "editor.h"
#include "game.h"
#include "utils.h"
#include <SDL2/SDL_image.h>

int InputTextCallback(ImGuiInputTextCallbackData *data) {
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    // Resize string callback
    std::string *str = (std::string *)data->UserData;
    IM_ASSERT(data->Buf == str->c_str());
    str->resize(data->BufTextLen);
    data->Buf = (char *)str->c_str();
  }
}

bool InputTextString(const char *label, std::string *str,
                     ImGuiInputTextFlags flags) {
  flags |= ImGuiInputTextFlags_CallbackResize;
  return ImGui::InputText(label, (char *)str->c_str(), str->capacity() + 1,
                          flags, InputTextCallback, (void *)str);
}

void Editor::start(Game &game) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
  // Controls
  // stop imgui from changing the cursor
  io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

  ImGui_ImplSDL2_InitForOpenGL(game.engine.window, game.engine.context);
  ImGui_ImplOpenGL3_Init(game.engine.glsl_version.c_str());

  // Setup style
  ImGui::StyleColorsDark();
  set_theme(game);

  // max prefab len
  for (size_t i = 0; i < 500; i++) {
    prefab_file_path.push_back('a');
  }
  prefab_file_path.clear();
  auto tile_image = game.engine.get_image(ImageName::Tiles);
  auto misc_image = game.engine.get_image(ImageName::Misc);
  tilesheet_sprite =
      SpriteSheet(game, tile_image, Vec2(0, 0),
                  vector<SpriteSrc>{SpriteSrc(ImageLocation(ImageLocation(
                      tile_image, Rect(0, 0, tile_image.image_dims.x,
                                       tile_image.image_dims.y))))},
                  100);
  add_obstacle_and_warp_point_sprites(game);

  game_flags_as_strings = game.get_game_flags_as_strings();
  item_names_as_strings = game.get_item_names_as_strings();
  ability_names_as_strings = game.get_ability_names_as_strings();
  unit_names_as_strings = game.get_unit_names_as_strings();
  treasure_chest_names_as_strings = game.get_treasure_chest_names_as_strings();
  status_effect_names_as_strings = game.get_status_effect_names_as_strings();
}

void Editor::set_theme(Game &game) {
  ImGuiStyle *style = &ImGui::GetStyle();
  ImVec4 *colors = style->Colors;

  colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.150f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.30f);
  colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.45f);
  colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
  colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
  colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingHighlight] =
      ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

  style->ChildRounding = 4.0f;
  style->FrameBorderSize = 1.0f;
  style->FrameRounding = 2.0f;
  style->GrabMinSize = 7.0f;
  style->PopupRounding = 2.0f;
  style->ScrollbarRounding = 12.0f;
  style->ScrollbarSize = 13.0f;
  style->TabBorderSize = 1.0f;
  style->TabRounding = 0.0f;
}

void Editor::add_obstacle_and_warp_point_sprites(Game &game) {
  auto tile_image = game.engine.get_image(ImageName::Tiles);
  // clear old obstacle sprites
  tile_obstacle_sprites.clear();
  tile_warp_point_to_map_sprites.clear();
  for (auto &tile : game.map.tiles) {
    auto tile_obstacle_sprite =
        Sprite(game, tile_image, Vec2(tile.sprite.dst.x, tile.sprite.dst.y),
               vector<SpriteSrc>{SpriteSrc(ImageLocation(
                   ImageLocation(tile_image, Rect(0, 220, 20, 20))))},
               100);
    tile_obstacle_sprites.push_back(tile_obstacle_sprite);
    // warp points to other maps
    auto tile_warp_point_to_map_sprite =
        Sprite(game, tile_image, Vec2(tile.sprite.dst.x, tile.sprite.dst.y),
               vector<SpriteSrc>{SpriteSrc(ImageLocation(
                   ImageLocation(tile_image, Rect(20, 220, 20, 20))))},
               100);
    tile_warp_point_to_map_sprites.push_back(tile_warp_point_to_map_sprite);
    // warp points in the same map
    auto tile_warp_point_sprite =
        Sprite(game, tile_image, Vec2(tile.sprite.dst.x, tile.sprite.dst.y),
               vector<SpriteSrc>{SpriteSrc(ImageLocation(
                   ImageLocation(tile_image, Rect(40, 220, 20, 20))))},
               100);
    tile_warp_point_sprites.push_back(tile_warp_point_sprite);
  }
}

void Editor::update(Game &game) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(game.engine.window);
  ImGui::NewFrame();

  for (size_t i = 0; i < game.map.tiles.size(); i++) {
    auto &tile_obstacle_sprite = tile_obstacle_sprites[i];
    auto &tile_warp_point_to_map_sprite = tile_warp_point_to_map_sprites[i];
    auto &tile_warp_point_sprite = tile_warp_point_sprites[i];
    tile_obstacle_sprite.is_hidden = !game.map.tiles[i].is_obstacle;
    // always hidden until these are used again
    tile_warp_point_to_map_sprite.is_hidden = true;
    tile_warp_point_sprite.is_hidden = !game.map.tiles[i].is_warp_point;
    tile_obstacle_sprite.update(game);
    tile_warp_point_to_map_sprite.update(game);
    tile_warp_point_sprite.update(game);
  }

  handle_input_events(game);
  update_nav_window(game);
  update_tilesheet_window(game);
  update_file_window(game);
  update_inspect_window(game);

  ImGui::Render();
}

void Editor::draw(Game &game) {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // only draw obstacle tiles if in obstacle spawn mode
  if (show_obstacles_always ||
      editor_spawn_mode == EditorSpawnMode::TileObstacle) {
    for (auto &tile_obstacle_sprite : tile_obstacle_sprites) {
      tile_obstacle_sprite.draw(game);
    }
    // send obstacle sprite draw calls
    game.engine.present_render_buffer();
  }
  if (show_warp_points_always ||
      editor_spawn_mode == EditorSpawnMode::TileWarpPointToMap ||
      editor_spawn_mode == EditorSpawnMode::TileWarpPoint) {
    for (auto &tile_warp_point_to_map_sprite : tile_warp_point_to_map_sprites) {
      tile_warp_point_to_map_sprite.draw(game);
    }
    for (auto &tile_warp_point_sprite : tile_warp_point_sprites) {
      tile_warp_point_sprite.draw(game);
    }
    // send obstacle sprite draw calls
    game.engine.present_render_buffer();
  }

  // just drawing the spritesheet at the bottom left corner for now, its not
  // tied to the window at all.
  tilesheet_rect = Rect(0, 0, tilesheet_sprite.image.image_dims.x,
                        tilesheet_sprite.image.image_dims.y);
  glViewport(0, 0, game.engine.game_resolution.x,
             game.engine.game_resolution.y);
  tilesheet_sprite.update(game);
  tilesheet_sprite.draw(game);

  // send tilesheet draw calls
  game.engine.present_render_buffer();

  glColor4f(1.0, 1.0, 1.0, 1.0);
  glLineWidth(4);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glBegin(GL_POLYGON);
  // not sure why this math works for the rect
  auto x1 = selector_rect.x;
  auto y1 = selector_rect.y - selector_rect.h + TILE_SIZE;
  auto x2 = selector_rect.x + selector_rect.w;
  auto y2 = selector_rect.y + TILE_SIZE;
  glVertex2f(x1, y1);
  glVertex2f(x2, y1);
  glVertex2f(x2, y2);
  glVertex2f(x1, y2);
  glEnd();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Editor::destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void Editor::handle_input_events(Game &game) {
  if (rect_contains_point(tilesheet_rect,
                          game.engine.mouse_point_opengl_origin)) {
    mouse_over_tilesheet_rect = true;
    int diff_x = game.engine.mouse_point_opengl_origin.x - tilesheet_rect.x;
    int diff_y = tilesheet_rect.h - game.engine.mouse_point_opengl_origin.y;
    diff_x /= TILE_SIZE;
    diff_y /= TILE_SIZE;
    selector_rect.x = diff_x * TILE_SIZE;
    selector_rect.y =
        tilesheet_sprite.image.image_dims.y - ((diff_y + 1) * TILE_SIZE);
    if (game.engine.is_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::Tile;
      tilesheet_src = Rect(diff_x * TILE_SIZE, diff_y * TILE_SIZE,
                           selector_rect.w, selector_rect.h);
      /*cout << "src " << tilesheet_src.x << " " << tilesheet_src.y << " " <<
         tilesheet_src.w << " "
           << tilesheet_src.h << "\n";*/
    }
  } else {
    mouse_over_tilesheet_rect = false;
  }
  if (editor_spawn_mode == EditorSpawnMode::Unit) {
    if (game.engine.mouse_in_game_rect && game.engine.is_mouse_down) {
      auto tile_point = world_point_to_tile_point_move_grid(
          Vec2(game.engine.mouse_point_game_rect_scaled_camera));
      auto unit = unit_deserialize_from_file(game, prefab_file_path.c_str());
      unit.set_tile_point_move_grid(tile_point);
      game.map.unit_dict[unit.guid] = unit;
    } else if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::TreasureChest) {
    if (game.engine.mouse_in_game_rect && game.engine.is_mouse_down) {
      auto tile_point = world_point_to_tile_point(
          Vec2(game.engine.mouse_point_game_rect_scaled_camera));
      auto treasure_chest =
          treasure_chest_deserialize_from_file(game, prefab_file_path.c_str());
      treasure_chest.set_tile_point(game, tile_point);
      game.map.treasure_chest_dict[treasure_chest.guid] = treasure_chest;
    } else if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::Item) {
    if (game.engine.mouse_in_game_rect && game.engine.is_mouse_down) {
      auto tile_point = world_point_to_tile_point(
          Vec2(game.engine.mouse_point_game_rect_scaled_camera));
      auto item = item_deserialize_from_file(game, prefab_file_path.c_str());
      set_world_point_from_tile_point(tile_point, item.sprite.dst);
      game.map.item_dict[item.guid] = item;
    } else if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::ItemInTreasureChest) {
    if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::ItemInUnitInventory) {
    if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::StatusEffectInAbility) {
    if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::Tile) {
    // if right click outside of the game rect set spawn mode to none
    if (!game.engine.mouse_in_game_rect) {
      if (game.engine.is_right_mouse_down) {
        editor_spawn_mode = EditorSpawnMode::None;
      }
    }
    if (game.engine.mouse_in_game_rect) {
      auto tile_point = world_point_to_tile_point(
          Vec2(game.engine.mouse_point_game_rect_scaled_camera));
      auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
      // sometimes the mouse can go out of the game rect so the tile idx would
      // be out of bounds.
      if (tile_idx >= 0 && tile_idx <= (int)game.map.tiles.size() - 1) {
        auto image = tilesheet_sprite.image;
        auto sprite = &game.map.tiles.at(tile_idx).sprite;
        if (tile_layer != -1) {
          assert(tile_layer <= (int)game.map.layers.size() - 1);
          sprite = &game.map.layers.at(tile_layer).at(tile_idx);
        }
        if (game.engine.is_mouse_held_down) {
          sprite->image = image;
          sprite->srcs.clear();
          sprite->srcs.push_back(
              SpriteSrc(ImageLocation(image, tilesheet_src)));
        } else if (game.engine.is_right_mouse_held_down) {
          sprite->srcs.clear();
          if (tile_layer == -1) {
            sprite->srcs.push_back(
                SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))));
          }
        }
      }
    }
  } else if (editor_spawn_mode == EditorSpawnMode::TileObstacle) {
    auto tile_point = world_point_to_tile_point(
        Vec2(game.engine.mouse_point_game_rect_scaled_camera));
    auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
    auto &tile = game.map.tiles.at(tile_idx);
    auto &tile_obstacle_sprite = tile_obstacle_sprites.at(tile_idx);
    // sometimes the mouse can go out of the game rect so the tile idx would
    // be out of bounds.
    if (game.engine.mouse_in_game_rect && tile_idx >= 0 &&
        tile_idx <= (int)game.map.tiles.size() - 1) {
      if (game.engine.is_mouse_held_down) {
        tile.is_obstacle = true;
      } else if (game.engine.is_right_mouse_held_down) {
        tile.is_obstacle = false;
      }
    } else if (!game.engine.mouse_in_game_rect) {
      if (game.engine.is_right_mouse_down) {
        editor_spawn_mode = EditorSpawnMode::None;
        in_obstacle_mode = false;
      }
    }
  } else if (editor_spawn_mode == EditorSpawnMode::TileWarpPointToMap) {
    /*auto tile_point = world_point_to_tile_point(
        Vec2(game.engine.mouse_point_game_rect_scaled_camera));
    auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
    auto &tile = game.map.tiles.at(tile_idx);
    auto &tile_warp_point_to_map_sprite =
        tile_warp_point_to_map_sprites.at(tile_idx);
    // sometimes the mouse can go out of the game rect so the tile idx would
    // be out of bounds.
    if (game.engine.mouse_in_game_rect && tile_idx >= 0 &&
        tile_idx <= (int)game.map.tiles.size() - 1) {
      if (game.engine.is_mouse_held_down) {
        tile.is_warp_point_to_map = true;
        string formatted_map_file_path = "../assets/prefabs/maps/";
        formatted_map_file_path += warp_to_map_file_path;
        formatted_map_file_path += ".json";
        tile.warps_to_map_file_path = formatted_map_file_path;
        tile.warps_to_tile_point = warp_to_map_tile_point;
      } else if (game.engine.is_right_mouse_held_down) {
        tile.is_warp_point_to_map = false;
      }
    } else if (!game.engine.mouse_in_game_rect) {
      if (game.engine.is_right_mouse_down) {
        editor_spawn_mode = EditorSpawnMode::None;
        in_warp_point_to_map_mode = false;
      }
    }*/
  } else if (editor_spawn_mode == EditorSpawnMode::TileWarpPoint) {
    auto tile_point = world_point_to_tile_point(
        Vec2(game.engine.mouse_point_game_rect_scaled_camera));
    auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
    auto &tile = game.map.tiles.at(tile_idx);
    auto &tile_warp_point_sprite = tile_warp_point_sprites.at(tile_idx);
    // sometimes the mouse can go out of the game rect so the tile idx would
    // be out of bounds.
    if (game.engine.mouse_in_game_rect && tile_idx >= 0 &&
        tile_idx <= (int)game.map.tiles.size() - 1) {
      if (game.engine.is_mouse_held_down) {
        tile.is_warp_point = true;
        tile.warps_to_tile_point = warp_to_map_tile_point;
      } else if (game.engine.is_right_mouse_held_down) {
        tile.is_warp_point = false;
      }
    } else if (!game.engine.mouse_in_game_rect) {
      if (game.engine.is_right_mouse_down) {
        editor_spawn_mode = EditorSpawnMode::None;
        in_warp_point_mode = false;
      }
    }
  } else if (editor_spawn_mode == EditorSpawnMode::AiWalkPath) {
    auto &unit = game.map.unit_dict.at(inspecting_prefab_guid);
    if (game.engine.mouse_in_game_rect) {
      if (game.engine.is_mouse_down) {
        auto tile_point = world_point_to_tile_point_move_grid(
            Vec2(game.engine.mouse_point_game_rect_scaled_camera));
        unit.ai_walk_paths.push_back(AIWalkPath(tile_point, 0));
      }
    }
    if (game.engine.is_right_mouse_down) {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  } else if (editor_spawn_mode == EditorSpawnMode::None) {
    if (game.engine.mouse_in_game_rect) {
      if (game.map.unit_input.clicked) {
        inspecting_prefab_guid = game.map.unit_input.guid;
        auto &unit = game.map.unit_dict.at(inspecting_prefab_guid);
        editor_inspect_mode = EditorInspectMode::Unit;
      } else if (game.map.treasure_chest_input.clicked) {
        inspecting_prefab_guid = game.map.treasure_chest_input.guid;
        editor_inspect_mode = EditorInspectMode::TreasureChest;
      } else if (game.map.item_input.clicked) {
        inspecting_prefab_guid = game.map.item_input.guid;
        editor_inspect_mode = EditorInspectMode::Item;
      } else if (game.engine.is_mouse_up) {
        editor_inspect_mode = EditorInspectMode::Tile;
        auto tile_point = world_point_to_tile_point(
            Vec2(game.engine.mouse_point_game_rect_scaled_camera));
        auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
        inspecting_tile_idx = tile_idx;
      }

      // only allow removing entities if not playing
      if (!game.editor_state.in_play_mode) {
        if (game.map.unit_input.right_clicked) {
          game.map.erase_unit_guid(game, game.map.unit_input.guid);
          editor_inspect_mode = EditorInspectMode::None;
        } else if (game.map.treasure_chest_input.right_clicked) {
          game.map.treasure_chest_dict.erase(
              game.map.treasure_chest_input.guid);
          editor_inspect_mode = EditorInspectMode::None;
        } else if (game.map.item_input.right_clicked) {
          game.map.item_dict.erase(game.map.item_input.guid);
          editor_inspect_mode = EditorInspectMode::None;
        }
      }
    }
  }
}

void Editor::update_nav_window(Game &game) {
  // left hand side window begin
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(game.engine.window_resolution.x,
                                  game.engine.game_rect_top_padding));
  ImGui::Begin("Nav", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  if (ImGui::Button(game.editor_state.in_play_mode ? "Stop playing" : "Play")) {
    game.editor_state.in_play_mode = !game.editor_state.in_play_mode;
    if (game.editor_state.in_play_mode) {
      prev_map = game.map;
      prev_ui = game.ui;
      prev_camera = game.engine.camera;
    } else {
      game.map = prev_map;
      game.ui = prev_ui;
      game.engine.camera = prev_camera;
    }
  }

  current_editor_mode = "Editor mode: none";
  if (editor_spawn_mode == EditorSpawnMode::Unit) {
    current_editor_mode = "Editor mode: spawn unit (" + prefab_file_path + ")";
  } else if (editor_spawn_mode == EditorSpawnMode::TreasureChest) {
    current_editor_mode =
        "Editor mode: spawn treasure chest (" + prefab_file_path + ")";
  } else if (editor_spawn_mode == EditorSpawnMode::Item) {
    current_editor_mode = "Editor mode: spawn item (" + prefab_file_path + ")";
  } else if (editor_spawn_mode == EditorSpawnMode::ItemInTreasureChest) {
    current_editor_mode =
        "Editor mode: spawn item in treasure chest - click on an item prefab "
        "button to add it to the treasure chest.";
  } else if (editor_spawn_mode == EditorSpawnMode::ItemInUnitInventory) {
    current_editor_mode =
        "Editor mode: spawn item in unit's inventory - click on an item prefab "
        "button to add it to the unit's inventory.";
  } else if (editor_spawn_mode == EditorSpawnMode::StatusEffectInAbility) {
    current_editor_mode = "Editor mode: spawn status effect in ability - click "
                          "on a status effect prefab "
                          "button to add it to the ability.";
  } else if (editor_spawn_mode == EditorSpawnMode::Tile) {
    current_editor_mode =
        "Editor mode: place tile - src rect = (" + to_string(tilesheet_src.x) +
        ", " + to_string(tilesheet_src.y) + ", " + to_string(tilesheet_src.w) +
        ", " + to_string(tilesheet_src.h) + ")";
  } else if (editor_spawn_mode == EditorSpawnMode::TileObstacle) {
    current_editor_mode = "Editor mode: set tiles as obstacle tiles";
  } else if (editor_spawn_mode == EditorSpawnMode::TileWarpPointToMap) {
    current_editor_mode = "Editor mode: set tiles as warp points to other maps";
  } else if (editor_spawn_mode == EditorSpawnMode::TileWarpPoint) {
    current_editor_mode = "Editor mode: set tiles as warp points";
  } else if (editor_spawn_mode == EditorSpawnMode::AiWalkPath) {
    current_editor_mode = "Editor mode: create ai walk path";
  }
  ImGui::SameLine();
  ImGui::Text(current_editor_mode.c_str());

  ImGui::SameLine(game.engine.window_resolution.x - 580);
  ImGui::Text("Current map");
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  InputTextString("Current map file name", &save_map_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  ImGui::SameLine();
  if (ImGui::Button("Save current map")) {
    if (save_map_file_name.size() > 0) {
      formatted_prefab_file_name = "../assets/prefabs/maps/";
      formatted_prefab_file_name += save_map_file_name;
      formatted_prefab_file_name += ".json";
      map_serialize_into_file(game, game.map,
                              formatted_prefab_file_name.c_str());
      game.assets.update_all_assets_from_files(game);
      formatted_prefab_file_name = "";
    }
  }

  // left hand side window end
  ImGui::End();
}

void Editor::update_tilesheet_window(Game &game) {
  // left hand side window begin
  left_menu_pos = ImVec2(0, game.engine.game_rect_top_padding);
  left_menu_size =
      ImVec2(game.engine.game_rect.x, game.engine.window_resolution.y -
                                          game.engine.game_rect_top_padding);
  ImGui::SetNextWindowPos(left_menu_pos);
  ImGui::SetNextWindowSize(left_menu_size);
  ImGui::Begin("Tiles", nullptr,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize);
  // left hand side window content
  ImGui::Checkbox("Show obstacles always", &show_obstacles_always);
  ImGui::Checkbox("Show warp points always", &show_warp_points_always);
  if (ImGui::Checkbox("Obstacle mode##tiles", &in_obstacle_mode)) {
    if (in_obstacle_mode) {
      editor_spawn_mode = EditorSpawnMode::TileObstacle;
    } else {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  }
  if (ImGui::Checkbox("Warp point mode##tiles", &in_warp_point_mode)) {
    if (in_warp_point_mode) {
      editor_spawn_mode = EditorSpawnMode::TileWarpPoint;
    } else {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  }
  /* InputTextString("Warp to map", &warp_to_map_file_path,
                   ImGuiInputTextFlags_CallbackResize);*/
  if (ImGui::InputInt("Warp to tile x", &warp_to_map_tile_point.x, 1)) {
    if (warp_to_map_tile_point.x < 0) {
      warp_to_map_tile_point.x = 0;
    }
  }
  if (ImGui::InputInt("Warp to tile y", &warp_to_map_tile_point.y, 1)) {
    if (warp_to_map_tile_point.y < 0) {
      warp_to_map_tile_point.y = 0;
    }
  }
  /*if (ImGui::Checkbox("Warp point to other map mode##tiles",
                      &in_warp_point_to_map_mode)) {
    if (in_warp_point_to_map_mode) {
      editor_spawn_mode = EditorSpawnMode::TileWarpPointToMap;
    } else {
      editor_spawn_mode = EditorSpawnMode::None;
    }
  }*/
  ImGui::Text("Tiles");
  if (ImGui::InputInt("Layer", &tile_layer, 1)) {
    if (tile_layer < -1) {
      tile_layer = -1;
    } else if (tile_layer > MAX_LAYERS - 1) {
      tile_layer = MAX_LAYERS - 1;
    }
  }
  ImGui::InputInt("Selector rect w", &selector_rect.w, 20);
  ImGui::InputInt("Selector rect h", &selector_rect.h, 20);
  // left hand side window end
  ImGui::End();
}

void Editor::update_file_window(Game &game) {
  // bottom window begin
  ImGui::SetNextWindowPos(
      ImVec2(game.engine.game_rect.x,
             game.engine.game_rect.h + game.engine.game_rect_top_padding));
  ImGui::SetNextWindowSize(
      ImVec2(game.engine.game_rect.w,
             (game.engine.window_resolution.y - game.engine.game_rect.h) -
                 game.engine.game_rect_top_padding));
  ImGui::Begin("Prefabs", nullptr,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize);
  // bottom window content
  if (ImGui::BeginCombo(
          "Select prefab type",
          editor_file_modes_as_strings[(int)editor_file_mode].c_str())) {
    bool unnecessary_selected = false;
    for (int i = 0; i < (int)editor_file_modes_as_strings.size(); i++) {
      auto &editor_file_mode_str = editor_file_modes_as_strings[i];
      auto i_str = to_string(i);
      auto label = editor_file_mode_str + "##file_mode" + i_str;
      bool is_selected = i == (int)editor_file_mode;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        editor_file_mode = (EditorFileMode)i;
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("inspect prefab##files", &inspect_prefab_mode)) {
    editor_spawn_mode = EditorSpawnMode::None;
    if (!inspect_prefab_mode) {
      editor_inspect_mode = EditorInspectMode::None;
    }
  }
  ImGui::SameLine();
  // if a field is added to a prefab and you want to quickly serialize it
  // across all prefabs
  if (ImGui::Button("reserialize all assets##file window")) {
    game.assets.reserialize_all_assets(game);
  }
  if (editor_file_mode == EditorFileMode::Units) {
    sorted_unit_files.clear();
    if (auto dir = opendir("../assets/prefabs/units")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_unit_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_unit_files.begin(), sorted_unit_files.end());
    for (auto &file_name : sorted_unit_files) {
      if (ImGui::Button(file_name.c_str())) {
        prefab_file_path = "../assets/prefabs/units/" + file_name;
        if (inspect_prefab_mode) {
          editor_inspect_mode = EditorInspectMode::Unit;
          inspect_unit_prefab_from_file =
              unit_deserialize_from_file(game, prefab_file_path.c_str());
          save_prefab_file_name = file_name;
          string_replace_all(save_prefab_file_name, ".json", "");
        } else {
          editor_spawn_mode = EditorSpawnMode::Unit;
        }
      }
    }
  } else if (editor_file_mode == EditorFileMode::Maps) {
    sorted_map_files.clear();
    if (auto dir = opendir("../assets/prefabs/maps")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_map_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_map_files.begin(), sorted_map_files.end());
    for (auto &file_name : sorted_map_files) {
      if (ImGui::Button(file_name.c_str())) {
        // editor_spawn_mode = EditorSpawnMode::Unit;
        prefab_file_path = "../assets/prefabs/maps/" + file_name;
        game.map = map_deserialize_from_file(game, prefab_file_path.c_str());
        // add player units to map
        game.map.add_all_player_units(game);
        prefab_file_path = "";
        save_prefab_file_name = "";
        editor_inspect_mode = EditorInspectMode::None;
        editor_spawn_mode = EditorSpawnMode::None;
      }
    }
  } else if (editor_file_mode == EditorFileMode::TreasureChest) {
    sorted_treasure_chest_files.clear();
    if (auto dir = opendir("../assets/prefabs/treasurechests")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_treasure_chest_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_treasure_chest_files.begin(),
         sorted_treasure_chest_files.end());
    for (auto &file_name : sorted_treasure_chest_files) {
      if (ImGui::Button(file_name.c_str())) {
        prefab_file_path = "../assets/prefabs/treasurechests/" + file_name;
        if (inspect_prefab_mode) {
          editor_inspect_mode = EditorInspectMode::TreasureChest;
          inspect_treasure_chest_prefab_from_file =
              treasure_chest_deserialize_from_file(game,
                                                   prefab_file_path.c_str());
          save_prefab_file_name = file_name;
          string_replace_all(save_prefab_file_name, ".json", "");
        } else {
          editor_spawn_mode = EditorSpawnMode::TreasureChest;
        }
      }
    }
  } else if (editor_file_mode == EditorFileMode::Item) {
    sorted_item_files.clear();
    if (auto dir = opendir("../assets/prefabs/items")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_item_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_item_files.begin(), sorted_item_files.end());
    for (auto &file_name : sorted_item_files) {
      if (ImGui::Button(file_name.c_str())) {
        prefab_file_path = "../assets/prefabs/items/" + file_name;
        if (editor_spawn_mode == EditorSpawnMode::ItemInTreasureChest) {
          auto &treasure_chest = get_inspected_treasure_chest(game);
          auto item =
              item_deserialize_from_file(game, prefab_file_path.c_str());
          treasure_chest.inventory.add_item(item);
          prefab_file_path = "";
        } else if (editor_spawn_mode == EditorSpawnMode::ItemInUnitInventory) {
          auto &unit = get_inspected_unit(game);
          auto item =
              item_deserialize_from_file(game, prefab_file_path.c_str());
          unit.inventory.add_item(item);
          prefab_file_path = "";
        }
        // if adding an item to a chest when you click on the item prefab
        // json button it will add it to the chest else the mode gets switched
        // to spawn item.
        else if (inspect_prefab_mode) {
          editor_inspect_mode = EditorInspectMode::Item;
          inspect_item_prefab_from_file =
              item_deserialize_from_file(game, prefab_file_path.c_str());
          save_prefab_file_name = file_name;
          string_replace_all(save_prefab_file_name, ".json", "");
        } else {
          editor_spawn_mode = EditorSpawnMode::Item;
        }
      }
    }
  } else if (editor_file_mode == EditorFileMode::Ability) {
    sorted_ability_files.clear();
    if (auto dir = opendir("../assets/prefabs/abilities")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_ability_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_ability_files.begin(), sorted_ability_files.end());
    for (auto &file_name : sorted_ability_files) {
      if (ImGui::Button(file_name.c_str())) {
        prefab_file_path = "../assets/prefabs/abilities/" + file_name;
        // can't spawn an ability
        editor_spawn_mode = EditorSpawnMode::None;
        if (inspect_prefab_mode) {
          editor_inspect_mode = EditorInspectMode::Ability;
          inspect_ability_prefab_from_file =
              ability_deserialize_from_file(game, prefab_file_path.c_str());
          save_prefab_file_name = file_name;
          string_replace_all(save_prefab_file_name, ".json", "");
        }
      }
    }
  } else if (editor_file_mode == EditorFileMode::StatusEffect) {
    sorted_status_effect_files.clear();
    if (auto dir = opendir("../assets/prefabs/statuseffects")) {
      while (auto f = readdir(dir)) {
        if (!f->d_name || f->d_name[0] == '.') {
          continue; // Skip everything that starts with a dot
        }
        sorted_status_effect_files.push_back(f->d_name);
      }
      closedir(dir);
    }
    sort(sorted_status_effect_files.begin(), sorted_status_effect_files.end());
    for (auto &file_name : sorted_status_effect_files) {
      if (ImGui::Button(file_name.c_str())) {
        prefab_file_path = "../assets/prefabs/statuseffects/" + file_name;
        if (editor_spawn_mode == EditorSpawnMode::StatusEffectInAbility) {
          auto &ability = inspect_ability_prefab_from_file;
          inspect_status_effect_prefab_from_file =
              status_effect_deserialize_from_file(game,
                                                  prefab_file_path.c_str());
          ability.status_effect_pcts.push_back(
              StatusEffectPct(inspect_status_effect_prefab_from_file, 0.0));
          prefab_file_path = "";
        } else if (inspect_prefab_mode) {
          editor_inspect_mode = EditorInspectMode::StatusEffect;
          inspect_status_effect_prefab_from_file =
              status_effect_deserialize_from_file(game,
                                                  prefab_file_path.c_str());
          save_prefab_file_name = file_name;
          string_replace_all(save_prefab_file_name, ".json", "");
        }
      }
    }
  }
  // bottom window end
  ImGui::End();
}

void Editor::update_inspect_window(Game &game) {
  // right hand side window (inspect) begin
  auto inspect_x = game.engine.game_rect.x + game.engine.game_rect.w;
  ImGui::SetNextWindowPos(ImVec2(inspect_x, game.engine.game_rect_top_padding));
  ImGui::SetNextWindowSize(ImVec2(game.engine.window_resolution.x - inspect_x,
                                  game.engine.window_resolution.y -
                                      game.engine.game_rect_top_padding));
  ImGui::Begin("Inspect", nullptr,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize);

  // right hand side window content
  if (editor_inspect_mode == EditorInspectMode::None) {
    update_inspect_none(game);
  } else if (editor_inspect_mode == EditorInspectMode::Tile) {
    update_inspect_tile(game);
  } else if (editor_inspect_mode == EditorInspectMode::Unit) {
    update_inspect_unit(game);
  } else if (editor_inspect_mode == EditorInspectMode::TreasureChest) {
    update_inspect_treasure_chest(game);
  } else if (editor_inspect_mode == EditorInspectMode::Item) {
    update_inspect_item(game);
  } else if (editor_inspect_mode == EditorInspectMode::Ability) {
    update_inspect_ability(game);
  } else if (editor_inspect_mode == EditorInspectMode::StatusEffect) {
    update_inspect_status_effect(game);
  }

  // right hand side window end
  ImGui::End();
}

void Editor::update_inspect_none(Game &game) {
  ImGui::Text("Click on something to inspect it");
}

void Editor::update_inspect_tile(Game &game) {
  auto &tile = game.map.tiles.at(inspecting_tile_idx);
  auto twod_idx = oned_to_twod_idx(inspecting_tile_idx, game.map.rows);
  ImGui::Text(("Inspecting tile at idx: (" + to_string(twod_idx.x) + ", " +
               to_string(twod_idx.y) + +")")
                  .c_str());
  ImGui::Text(("Is obstacle: " + to_string(tile.is_obstacle)).c_str());
  ImGui::Text(("Is warp point: " + to_string(tile.is_warp_point)).c_str());
  if (!tile.is_warp_point) {
    ImGui::Text("warps to tile x: not a warp tile");
    ImGui::Text("warps to tile y: not a warp tile");
  } else {
    ImGui::Text(
        ("warps to tile x: " + to_string(tile.warps_to_tile_point.x)).c_str());
    ImGui::Text(
        ("warps to tile y: " + to_string(tile.warps_to_tile_point.y)).c_str());
  }
}

void Editor::update_inspect_unit(Game &game) {
  auto &unit = get_inspected_unit(game);

  InputTextString("prefab file name##unit", &save_prefab_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::Button("Save unit prefab")) {
    cout << "save unit prefab " << save_prefab_file_name << "\n";
    formatted_prefab_file_name = "../assets/prefabs/units/";
    formatted_prefab_file_name += save_prefab_file_name;
    formatted_prefab_file_name += ".json";
    unit_serialize_into_file(game, unit, formatted_prefab_file_name.c_str());
    game.assets.update_all_assets_from_files(game);
  }
  ImGui::Text(("Inspecting unit with handle: " + to_string(unit.guid)).c_str());
  ImGui::Text(("is unit moving: " + to_string(unit.is_moving)).c_str());
  ImGui::Text(("is unit ai walking: " + to_string(unit.is_ai_walking)).c_str());
  ImGui::Text(("in dialog with unit handle: " +
               to_string(unit.in_dialogue_with_unit_guid))
                  .c_str());
  ImGui::Text(("Money: " + to_string(unit.coin.quantity)).c_str());
  if (ImGui::BeginCombo("Unit name##unit",
                        unit_names_as_strings[(int)unit.unit_name].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < unit_names_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = unit_names_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)unit.unit_name;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        unit.unit_name = static_cast<UnitName>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  bool unnecessary_selected = false;
  if (ImGui::BeginCombo("Faction##unit",
                        factions_as_strings[(int)unit.faction].c_str())) {
    for (size_t i = 0; i < factions_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = factions_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)unit.faction;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        unit.faction = static_cast<Faction>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::CollapsingHeader("anim speeds")) {
    ImGui::InputInt("Portrait anim speed##unit",
                    (int *)&unit.sprite.portrait_anim_speed, 10);
    ImGui::InputInt("Idle anim speed", (int *)&unit.sprite.idle_anim_speed, 10);
    ImGui::InputInt("Walk anim speed", (int *)&unit.sprite.walk_anim_speed, 10);
    ImGui::InputInt("Attack anim speed", (int *)&unit.sprite.attack_anim_speed,
                    10);
    ImGui::InputInt("Cast anim speed", (int *)&unit.sprite.cast_anim_speed, 10);
    ImGui::InputInt("Hit anim speed", (int *)&unit.sprite.hit_anim_speed, 10);
    ImGui::InputInt("Dead anim speed", (int *)&unit.sprite.dead_anim_speed, 10);
  }

  ImGui::Text("anim src rects / portrait");
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.portrait,
                      string("unit portrait"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.idle_down,
                      string("idle down"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.walk_down,
                      string("walk down"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.attack_down,
                      string("attack down"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.cast_down,
                      string("cast down"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.hit_down,
                      string("hit down"));
  display_sprite_srcs(game, unit.sprite.image, unit.sprite.dead_down,
                      string("dead down"));
  ImGui::Text("hitbox x, y, width and height (for pathing)");
  ImGui::Text(
      ("hitbox x: " + to_string(unit.sprite.tile_point_hit_box.x)).c_str());
  ImGui::Text(
      ("hitbox y: " + to_string(unit.sprite.tile_point_hit_box.y)).c_str());
  if (ImGui::InputInt("hitbox width##unit", &unit.sprite.hitbox_dims.x)) {
    if (unit.sprite.hitbox_dims.x < 0) {
      unit.sprite.hitbox_dims.x = 0;
    }
  }
  if (ImGui::InputInt("hitbox height##unit", &unit.sprite.hitbox_dims.y)) {
    if (unit.sprite.hitbox_dims.y < 0) {
      unit.sprite.hitbox_dims.y = 0;
    }
  }
  ImGui::Text("hitbox sizes for gui stuff");
  if (ImGui::InputInt("hitbox width##unit input events",
                      &unit.sprite.hitbox_dims_input_events.x)) {
    if (unit.sprite.hitbox_dims_input_events.x < 0) {
      unit.sprite.hitbox_dims_input_events.x = 0;
    }
  }
  if (ImGui::InputInt("hitbox height##unit input events",
                      &unit.sprite.hitbox_dims_input_events.y)) {
    if (unit.sprite.hitbox_dims_input_events.y < 0) {
      unit.sprite.hitbox_dims_input_events.y = 0;
    }
  }
  ImGui::Text("dialogues");
  if (ImGui::CollapsingHeader("unit dialogues")) {
    for (size_t i = 0; i < unit.dialogues.size(); i++) {
      auto &dialogue = unit.dialogues[i];
      auto i_str = to_string(i);
      auto section_header = "Dialogue info " + i_str;
      auto del_label = "X##Del dialogue " + i_str;
      auto up_label = "Move up##Up dialogue " + i_str;
      auto down_label = "Move down##Up dialogue " + i_str;
      ImGui::Text(section_header.c_str());
      if (i > 0) {
        ImGui::SameLine();
        if (ImGui::Button(up_label.c_str())) {
          auto tmp = unit.dialogues[i - 1];
          unit.dialogues[i - 1] = unit.dialogues[i];
          unit.dialogues[i] = tmp;
          // iteration maybe invalidated, break for next frmae
          break;
        }
      }
      if (i < unit.dialogues.size() - 1) {
        ImGui::SameLine();
        if (ImGui::Button(down_label.c_str())) {
          auto tmp = unit.dialogues[i + 1];
          unit.dialogues[i + 1] = unit.dialogues[i];
          unit.dialogues[i] = tmp;
          // iteration maybe invalidated, break for next frame
          break;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button(del_label.c_str())) {
        unit.dialogues.erase(unit.dialogues.begin() + i);
      }

      bool unnecessary_selected = false;
      auto select_label = "condition##dialogue" + to_string(i);
      if (ImGui::BeginCombo(
              select_label.c_str(),
              game_flags_as_strings[(int)dialogue.game_flag_condition]
                  .c_str())) {
        for (size_t j = 0; j < game_flags_as_strings.size(); j++) {
          auto j_str = to_string(j);
          auto ij_str = i_str + j_str;
          auto label = game_flags_as_strings[j] + "##" + ij_str;
          bool is_selected = (int)j == (int)dialogue.game_flag_condition;
          if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
            dialogue.game_flag_condition = static_cast<GameFlag>(j);
          }
          if (is_selected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      for (size_t j = 0; j < dialogue.strs.size(); j++) {
        auto j_str = to_string(j);
        auto ij_str = i_str + j_str;
        auto label = "dialogue " + j_str + "##" + ij_str;
        auto &str = dialogue.strs[j];
        auto del_dialogue_btn_label = "X##" + ij_str;
        InputTextString(label.c_str(), &str,
                        ImGuiInputTextFlags_CallbackResize);
        ImGui::SameLine();
        if (ImGui::Button(del_dialogue_btn_label.c_str())) {
          dialogue.strs.erase(dialogue.strs.begin() + j);
        }
      }
      auto add_dialogue_btn_label = "+##Add unit dialogue text " + to_string(i);
      if (ImGui::Button(add_dialogue_btn_label.c_str())) {
        auto str = string("");
        // make sure str has enough size (reserve doesnt work here for some
        // reason)
        for (size_t i = 0; i < 500; i++) {
          str.push_back('a');
        }
        str.clear();
        dialogue.strs.push_back(str);
      }
    }
    if (ImGui::Button("Add unit dialogue")) {
      auto dialogue = Dialogue();
      // reserve enough space for 50 strings
      dialogue.strs.reserve(50);
      unit.dialogues.push_back(dialogue);
    }
  }
  ImGui::Text("ai walk paths");
  string target_point_x_label = "";
  string target_point_y_label = "";
  string delay_label = "";
  if (ImGui::CollapsingHeader("ai walk paths for unit")) {
    for (size_t i = 0; i < unit.ai_walk_paths.size(); i++) {
      auto &ai_walk_path = unit.ai_walk_paths.at(i);
      target_point_x_label = "target point x" + to_string(i);
      target_point_y_label = "target point y" + to_string(i);
      delay_label = "delay" + to_string(i);
      if (i != 0) {
        if (ImGui::Button(("Del ai walk path " + to_string(i)).c_str())) {
          // every unit has an ai walk path of size 1, even allies. Ai only does
          // walks if the ai walk path size is > 1.
          if (i != 0 && unit.ai_walk_paths.size() > 1) {
            unit.ai_walk_paths.erase(unit.ai_walk_paths.begin() + i);
          }
        }
      }
      if (ImGui::InputInt(target_point_x_label.c_str(),
                          (int *)&ai_walk_path.target_point.x, 1)) {
      }
      if (ImGui::InputInt(target_point_y_label.c_str(),
                          (int *)&ai_walk_path.target_point.y, 1)) {
      }
      if (ImGui::InputInt(delay_label.c_str(), (int *)&ai_walk_path.delay, 1)) {
      }
    }
    if (ImGui::Button("Add ai walk path")) {
      unit.ai_walk_paths.push_back(AIWalkPath());
    }
    if (ImGui::Button("Click to add ai walk path")) {
      editor_spawn_mode = editor_spawn_mode != EditorSpawnMode::AiWalkPath
                              ? EditorSpawnMode::AiWalkPath
                              : EditorSpawnMode::None;
    }
  }

  ImGui::Text("Inventory");
  ImGui::Checkbox("is shop##unit", &unit.is_shop);
  if (ImGui::CollapsingHeader("inventory##unit")) {
    ImGui::Text(
        "If an item prefab has been updated, reload \nthe map for those "
        "changes to take effect \nin the shop.");
    for (size_t i = 0; i < unit.inventory.items.size(); i++) {
      auto &item = unit.inventory.items[i];
      if (item.item_name == ItemName::None) {
        continue;
      }
      auto i_str = to_string(i);
      ImGui::Text(("Item: " + i_str).c_str());
      ImGui::SameLine();
      if (ImGui::Button(("Del item##unit inventory " + i_str).c_str())) {
        unit.inventory.items.erase(unit.inventory.items.begin() + i);
        // stop iterating to avoid iterator invalidation. Next frame the gui
        // will re-render.
        break;
      }
      ImGui::Text(
          ("Item name: " + item_names_as_strings[(int)item.item_name]).c_str());
      ImGui::Text(("Display name: " + item.display_name).c_str());
      ImGui::Text(("Description: " + item.description).c_str());
      ImGui::Text(("Cost: " + to_string(item.cost)).c_str());
      if (ImGui::InputInt(("item quantity##unit inventory " + i_str).c_str(),
                          &item.quantity)) {
        if (item.quantity < 1) {
          // items in treasure chests must have at least quantity 1
          item.quantity = 1;
        } else if (item.quantity > ITEM_MAX_QUANTITY) {
          item.quantity = ITEM_MAX_QUANTITY;
        }
      }
    }
    if (ImGui::Button("Add item to unit's inventory")) {
      editor_spawn_mode = EditorSpawnMode::ItemInUnitInventory;
    }
  }
}

void Editor::update_inspect_treasure_chest(Game &game) {
  auto &treasure_chest = get_inspected_treasure_chest(game);

  InputTextString("prefab file name##treasure chest", &save_prefab_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::Button("Save treasure chest prefab")) {
    cout << "save unit prefab " << save_prefab_file_name << "\n";
    formatted_prefab_file_name = "../assets/prefabs/treasurechests/";
    formatted_prefab_file_name += save_prefab_file_name;
    formatted_prefab_file_name += ".json";
    treasure_chest_serialize_into_file(game, treasure_chest,
                                       formatted_prefab_file_name.c_str());
    game.assets.update_all_assets_from_files(game);
  }
  ImGui::Text(("Inspecting treasure chest with handle: " +
               to_string(treasure_chest.guid))
                  .c_str());
  ImGui::Checkbox("treasure chest opened", &treasure_chest.is_opened);
  if (ImGui::BeginCombo(
          "treasure chest name##treasure chest",
          treasure_chest_names_as_strings[(int)treasure_chest
                                              .treasure_chest_name]
              .c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < treasure_chest_names_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = treasure_chest_names_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)treasure_chest.treasure_chest_name;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        treasure_chest.treasure_chest_name = static_cast<TreasureChestName>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::CollapsingHeader("treasure chest closed sprite##treasure chest")) {
    ImGui::InputInt("closed anim speed##treasure chest",
                    (int *)&treasure_chest.sprite.anim_speed, 10);
    ImGui::Text("closed src rects");
    display_sprite_srcs(game, treasure_chest.sprite.image,
                        treasure_chest.sprite.srcs,
                        string("closed treasure chest srcs"));
  }
  if (ImGui::CollapsingHeader("treasure chest opened sprite##treasure chest")) {
    ImGui::InputInt("opened anim speed##treasure chest",
                    (int *)&treasure_chest.opened_sprite.anim_speed, 10);
    ImGui::Text("opened src rects");
    display_sprite_srcs(game, treasure_chest.opened_sprite.image,
                        treasure_chest.opened_sprite.srcs,
                        string("opened treasure chest srcs"));
  }
  if (ImGui::CollapsingHeader("inventory##treasure chest")) {
    for (size_t i = 0; i < treasure_chest.inventory.items.size(); i++) {
      auto &item = treasure_chest.inventory.items[i];
      if (item.item_name == ItemName::None) {
        continue;
      }
      auto i_str = to_string(i);
      ImGui::Text(("Item: " + i_str).c_str());
      ImGui::SameLine();
      if (ImGui::Button(
              ("Del item##treasure chest inventory " + i_str).c_str())) {
        treasure_chest.inventory.items.erase(
            treasure_chest.inventory.items.begin() + i);
        // stop iterating to avoid iterator invalidation. Next frame the gui
        // will re-render.
        break;
      }
      ImGui::Text(
          ("Item name: " + item_names_as_strings[(int)item.item_name]).c_str());
      ImGui::Text(("Display name: " + item.display_name).c_str());
      ImGui::Text(("Description: " + item.description).c_str());
      ImGui::Text(("Cost: " + to_string(item.cost)).c_str());
      if (ImGui::InputInt(
              ("item quantity##treasure chest item " + i_str).c_str(),
              &item.quantity)) {
        if (item.quantity < 1) {
          // items in treasure chests must have at least quantity 1
          item.quantity = 1;
        } else if (item.quantity > ITEM_MAX_QUANTITY) {
          item.quantity = ITEM_MAX_QUANTITY;
        }
      }
    }
    if (ImGui::Button("Add item to treasure chest")) {
      editor_spawn_mode = EditorSpawnMode::ItemInTreasureChest;
    }
  }
}

void Editor::update_inspect_item(Game &game) {
  auto &item = get_inspected_item(game);

  InputTextString("prefab file name##item", &save_prefab_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::Button("Save item prefab")) {
    cout << "save unit prefab " << save_prefab_file_name << "\n";
    formatted_prefab_file_name = "../assets/prefabs/items/";
    formatted_prefab_file_name += save_prefab_file_name;
    formatted_prefab_file_name += ".json";
    item_serialize_into_file(game, item, formatted_prefab_file_name.c_str());
    game.assets.update_all_assets_from_files(game);
  }
  ImGui::Text(("Inspecting item with handle: " + to_string(item.guid)).c_str());
  ImGui::Text("sprite");
  ImGui::InputInt("anim speed##item", (int *)&item.sprite.anim_speed, 10);
  display_sprite_srcs(game, item.sprite.image, item.sprite.srcs,
                      string("item srcs"));
  ImGui::Text("item values");
  if (ImGui::BeginCombo("Item name##item",
                        item_names_as_strings[(int)item.item_name].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < item_names_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = item_names_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)item.item_name;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        item.item_name = static_cast<ItemName>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::BeginCombo("Item type##item",
                        item_types_as_strings[(int)item.item_type].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < item_types_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = item_types_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)item.item_type;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        item.item_type = static_cast<ItemType>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  InputTextString("display name##item", &item.display_name,
                  ImGuiInputTextFlags_CallbackResize);
  InputTextString("description##item", &item.description,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::InputInt("cost##item", (int *)&item.cost, 1)) {
    if (item.quantity < 1) {
      item.quantity = 1;
    } else if (item.quantity > ITEM_MAX_COST) {
      item.quantity = ITEM_MAX_COST;
    }
  }

  if (ImGui::InputInt("item quantity##item", &item.quantity)) {
    // items should always have at least 1
    if (item.quantity < 1) {
      item.quantity = 1;
    } else if (item.quantity > ITEM_MAX_QUANTITY) {
      item.quantity = ITEM_MAX_QUANTITY;
    }
  }
}

void Editor::update_inspect_ability(Game &game) {
  auto &ability = inspect_ability_prefab_from_file;
  InputTextString("prefab file name##ability", &save_prefab_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::Button("Save ability prefab")) {
    cout << "save ability prefab " << save_prefab_file_name << "\n";
    formatted_prefab_file_name = "../assets/prefabs/abilities/";
    formatted_prefab_file_name += save_prefab_file_name;
    formatted_prefab_file_name += ".json";
    ability_serialize_into_file(game, ability,
                                formatted_prefab_file_name.c_str());
    game.assets.update_all_assets_from_files(game);
  }
  InputTextString("display name##ability", &ability.display_name,
                  ImGuiInputTextFlags_CallbackResize);
  InputTextString("description##ability", &ability.description,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::BeginCombo(
          "Ability name##ability",
          ability_names_as_strings[(int)ability.ability_name].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < ability_names_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = ability_names_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)ability.ability_name;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        ability.ability_name = static_cast<AbilityName>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::BeginCombo(
          "Ability type##ability",
          ability_types_as_strings[(int)ability.ability_type].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < ability_types_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = ability_types_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)ability.ability_type;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        ability.ability_type = static_cast<AbilityType>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::BeginCombo(
          "Ability element##ability",
          ability_elements_as_strings[(int)ability.ability_element].c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < ability_elements_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = ability_elements_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)ability.ability_element;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        ability.ability_element = static_cast<AbilityElement>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::Checkbox("is melee##ability", &ability.is_melee);
  if (ImGui::InputInt("damage##ability", &ability.stats.damage.current)) {
    if (ability.stats.damage.current <= 1) {
      ability.stats.damage.current = 1;
    } else if (ability.stats.damage.current > MAX_ABILITY_DAMAGE) {
      ability.stats.damage.current = MAX_ABILITY_DAMAGE;
    }
  }
  if (ImGui::InputInt("range##ability", &ability.stats.range.current)) {
    if (ability.stats.range.current <= 1) {
      ability.stats.range.current = 1;
    } else if (ability.stats.range.current > RANGE_MAX) {
      ability.stats.range.current = RANGE_MAX;
    }
  }
  if (ImGui::InputInt("aoe##ability", &ability.stats.aoe.current)) {
    if (ability.stats.aoe.current <= 1) {
      ability.stats.aoe.current = 1;
    } else if (ability.stats.aoe.current > AOE_MAX) {
      ability.stats.aoe.current = AOE_MAX;
    }
  }
  if (ImGui::InputInt("cast time##ability", &ability.stats.cast_time.current)) {
    if (ability.stats.cast_time.current <= 0) {
      ability.stats.cast_time.current = 0;
    } else if (ability.stats.cast_time.current > CAST_TIME_MAX) {
      ability.stats.cast_time.current = CAST_TIME_MAX;
    }
  }
  ImGui::Checkbox("is projectile##ability", &ability.is_projectile);
  if (ImGui::InputDouble("projectile speed##ability",
                         &ability.projectile_speed)) {
    if (ability.projectile_speed < 1) {
      ability.projectile_speed = 1;
    }
  }
  ImGui::InputInt("delay##ability", (int *)&ability.delay);
  if (ImGui::InputInt("anim speed##ability",
                      (int *)&ability.sprite.anim_speed)) {
    if (ability.sprite.anim_speed < 1) {
      ability.sprite.anim_speed = 1;
    }
  }
  if (ImGui::CollapsingHeader("status effects##ability status effect vec")) {
    for (size_t i = 0; i < ability.status_effect_pcts.size(); i++) {
      auto &status_effect_pct = ability.status_effect_pcts.at(i);
      auto i_as_str = to_string(i);
      if (ImGui::Button(
              ("Del##ability status effect pct " + i_as_str).c_str())) {
        if (ability.status_effect_pcts.size() > 0) {
          ability.status_effect_pcts.erase(ability.status_effect_pcts.begin() +
                                           i);
          return;
        }
      }
      ImGui::Text(status_effect_pct.status_effect.display_name.c_str());
      if (ImGui::InputDouble(
              ("Pct##ability status effect pct input " + i_as_str).c_str(),
              &status_effect_pct.pct)) {
        if (status_effect_pct.pct < 0) {
          status_effect_pct.pct = 0;
        }
      }
    }
    if (ImGui::Button("Add status effect from file##ability status effect")) {
      editor_spawn_mode = EditorSpawnMode::StatusEffectInAbility;
    }
  }
  display_sprite_srcs(game, ability.sprite.image, ability.sprite.srcs,
                      string("ability srcs"));
  if (ImGui::InputInt("portrait anim speed##ability",
                      (int *)&ability.portrait.anim_speed)) {
    if (ability.portrait.anim_speed < 1) {
      ability.portrait.anim_speed = 1;
    }
  }
  display_sprite_srcs(game, ability.portrait.image, ability.portrait.srcs,
                      string("ability portrait srcs"));
}

void Editor::update_inspect_status_effect(Game &game) {
  auto &status_effect = inspect_status_effect_prefab_from_file;
  InputTextString("prefab file name##status_effect", &save_prefab_file_name,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::Button("Save status effect prefab")) {
    cout << "save status effect prefab " << save_prefab_file_name << "\n";
    formatted_prefab_file_name = "../assets/prefabs/statuseffects/";
    formatted_prefab_file_name += save_prefab_file_name;
    formatted_prefab_file_name += ".json";
    status_effect_serialize_into_file(game, status_effect,
                                      formatted_prefab_file_name.c_str());
    game.assets.update_all_assets_from_files(game);
  }
  InputTextString("display name##status_effect", &status_effect.display_name,
                  ImGuiInputTextFlags_CallbackResize);
  InputTextString("description##status_effect", &status_effect.description,
                  ImGuiInputTextFlags_CallbackResize);
  if (ImGui::BeginCombo(
          "Status effect name##status_effect",
          status_effect_names_as_strings[(int)status_effect.status_effect_name]
              .c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < status_effect_names_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = status_effect_names_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)status_effect.status_effect_name;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        status_effect.status_effect_name = static_cast<StatusEffectName>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (ImGui::BeginCombo(
          "Status effect type##status_effect",
          status_effect_types_as_strings[(int)status_effect.status_effect_type]
              .c_str())) {
    bool unnecessary_selected = false;
    for (size_t i = 0; i < status_effect_types_as_strings.size(); i++) {
      auto i_str = to_string(i);
      auto label = status_effect_types_as_strings[i] + "##" + i_str;
      bool is_selected = (int)i == (int)status_effect.status_effect_type;
      if (ImGui::Selectable(label.c_str(), &unnecessary_selected)) {
        status_effect.status_effect_type = static_cast<StatusEffectType>(i);
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::InputInt("turn duration##status_effect",
                  (int *)&status_effect.turns_remaining);
  if (ImGui::InputInt("anim speed##status_effect",
                      (int *)&status_effect.sprite.anim_speed)) {
    if (status_effect.sprite.anim_speed < 1) {
      status_effect.sprite.anim_speed = 1;
    }
  }
  display_sprite_srcs(game, status_effect.sprite.image,
                      status_effect.sprite.srcs, string("status_effect srcs"));
  if (ImGui::InputInt("portrait anim speed##status_effect",
                      (int *)&status_effect.portrait.anim_speed)) {
    if (status_effect.portrait.anim_speed < 1) {
      status_effect.portrait.anim_speed = 1;
    }
  }
  display_sprite_srcs(game, status_effect.portrait.image,
                      status_effect.portrait.srcs,
                      string("status_effect portrait srcs"));
}

void Editor::display_sprite_srcs(Game &game, Image image,
                                 vector<SpriteSrc> &srcs, string base_label) {
  // imgui is using labels as ids
  // vecs reference stability should be fine here as its updated every frame.
  string index_str = "";
  string label_x_str = "";
  string label_y_str = "";
  string label_w_str = "";
  string label_h_str = "";
  string button_text = "Add " + base_label + " rect";
  if (ImGui::CollapsingHeader(base_label.c_str())) {
    for (size_t i = 0; i < srcs.size(); i++) {
      auto &src = srcs.at(i);
      index_str = "Del " + base_label + " idx: " + to_string(i);
      label_x_str = base_label + " x" + to_string(i);
      label_y_str = base_label + " y" + to_string(i);
      label_w_str = base_label + " w" + to_string(i);
      label_h_str = base_label + " h" + to_string(i);
      if (ImGui::Button(index_str.c_str())) {
        // only delete if there would be a src left, zero srcs will crash.
        if (srcs.size() > 1) {
          srcs.erase(srcs.begin() + i);
        }
        return;
      }
      if (ImGui::InputInt(label_x_str.c_str(), (int *)&src.image_location.src.x,
                          1)) {
        src = SpriteSrc(ImageLocation(image, src.image_location.src));
      }
      if (ImGui::InputInt(label_y_str.c_str(), (int *)&src.image_location.src.y,
                          1)) {
        src = SpriteSrc(ImageLocation(image, src.image_location.src));
      }
      if (ImGui::InputInt(label_w_str.c_str(), (int *)&src.image_location.src.w,
                          1)) {
        src = SpriteSrc(ImageLocation(image, src.image_location.src));
      }
      if (ImGui::InputInt(label_h_str.c_str(), (int *)&src.image_location.src.h,
                          1)) {
        src = SpriteSrc(ImageLocation(image, src.image_location.src));
      }
    }
    // will invalidate the iterator above but it should be fine for next frame
    if (ImGui::Button(button_text.c_str())) {
      srcs.push_back(SpriteSrc(ImageLocation(image, Rect(0, 0, 0, 0))));
    }
  }
}

Unit &Editor::get_inspected_unit(Game &game) {
  // the item that was being inspected has been picked up by the player
  // and removed from the item_dict.
  if (!inspect_prefab_mode) {
    if (game.map.unit_dict.contains(inspecting_prefab_guid)) {
      return game.map.unit_dict.at(inspecting_prefab_guid);
    }
  }

  return inspect_unit_prefab_from_file;
}

Item &Editor::get_inspected_item(Game &game) {
  // the item that was being inspected has been picked up by the player
  // and removed from the item_dict.
  if (!inspect_prefab_mode) {
    if (game.map.item_dict.contains(inspecting_prefab_guid)) {
      return game.map.item_dict.at(inspecting_prefab_guid);
    }
  }

  return inspect_item_prefab_from_file;
}

TreasureChest &Editor::get_inspected_treasure_chest(Game &game) {
  // the item that was being inspected has been picked up by the player
  // and removed from the item_dict.
  if (!inspect_prefab_mode) {
    if (game.map.treasure_chest_dict.contains(inspecting_prefab_guid)) {
      return game.map.treasure_chest_dict.at(inspecting_prefab_guid);
    }
  }

  return inspect_treasure_chest_prefab_from_file;
}