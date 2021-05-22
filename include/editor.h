#ifndef EDITOR_H
#define EDITOR_H

#include <GL/glew.h>

#include "engine.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "imgui_internal.h"
#include "map.h"
#include "spritesheet.h"
#include "status_effect.h"
#include "ui/ui.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
// linux only - couldn't get <filesystem> to compile
#include <dirent.h>
using namespace std;

struct Game;

enum class EditorInspectMode {
  None,
  Tile,
  Unit,
  TreasureChest,
  Item,
  Ability,
  StatusEffect,
};

enum class EditorSpawnMode {
  None,
  Unit,
  Tile,
  TileObstacle,
  TileWarpPointToMap,
  TileWarpPoint,
  AiWalkPath,
  TreasureChest,
  Item,
  ItemInTreasureChest,
  ItemInUnitInventory,
  StatusEffectInAbility,
};

enum class EditorFileMode {
  Maps = 0,
  Units = 1,
  TreasureChest = 2,
  Item = 3,
  Ability = 4,
  StatusEffect = 5,
};

struct Editor {
  Map prev_map = Map();
  Camera prev_camera = Camera();
  UI prev_ui = UI();
  EditorInspectMode editor_inspect_mode = EditorInspectMode::None;
  EditorSpawnMode editor_spawn_mode = EditorSpawnMode::None;
  EditorFileMode editor_file_mode = EditorFileMode::Maps;
  vector<string> editor_file_modes_as_strings = vector<string>{
      "Maps",  "Units",     "Treasure chests",
      "Items", "Abilities", "Status Effects",
  };
  vector<string> item_types_as_strings = vector<string>{
      "None",          "Money", "Misc", "Useable", "PrimaryHand",
      "SecondaryHand", "Armor", "Head", "Boots",
  };
  vector<string> factions_as_strings = vector<string>{
      "Neutral",
      "Ally",
      "Enemy",
  };
  vector<string> ability_types_as_strings = vector<string>{
      "None",
      "Damage",
      "Heal",
      "StatusEffect",
  };
  vector<string> ability_elements_as_strings = vector<string>{
      "None",  "Physical",  "Fire",  "Water",
      "Earth", "Lightning", "Light", "Shadow",
  };
  vector<string> status_effect_types_as_strings = vector<string>{
      "Buff",
      "Nerf",
  };
  string current_editor_mode = "";
  boost::uuids::uuid inspecting_prefab_guid;
  string prefab_file_path = "";
  string formatted_prefab_file_name = "";
  string save_prefab_file_name = "";
  string save_map_file_name = "";
  SpriteSheet tilesheet_sprite;
  ImVec2 left_menu_pos;
  ImVec2 left_menu_size;
  Rect tilesheet_rect = Rect(0, 0, 0, 0);
  Rect tilesheet_src = Rect(0, 0, 0, 0);
  Rect selector_rect = Rect(0, 0, 20, 20);
  bool mouse_over_tilesheet_rect = false;
  int tile_layer = -1;
  vector<Sprite> tile_obstacle_sprites = vector<Sprite>();
  vector<Sprite> tile_warp_point_to_map_sprites = vector<Sprite>();
  vector<Sprite> tile_warp_point_sprites = vector<Sprite>();
  bool show_obstacles_always = false;
  bool show_warp_points_always = false;
  bool in_obstacle_mode = false;
  bool in_warp_point_to_map_mode = false;
  bool in_warp_point_mode = false;
  vector<string> game_flags_as_strings = vector<string>();
  vector<string> item_names_as_strings = vector<string>();
  vector<string> ability_names_as_strings = vector<string>();
  vector<string> unit_names_as_strings = vector<string>();
  vector<string> treasure_chest_names_as_strings = vector<string>();
  vector<string> status_effect_names_as_strings = vector<string>();
  string warp_to_map_file_path = "";
  Vec2 warp_to_map_tile_point = Vec2(0, 0);
  int inspecting_tile_idx = -1;
  bool inspect_prefab_mode = false;
  Ability inspect_ability_prefab_from_file = Ability();
  Unit inspect_unit_prefab_from_file = Unit();
  TreasureChest inspect_treasure_chest_prefab_from_file = TreasureChest();
  Item inspect_item_prefab_from_file = Item();
  StatusEffect inspect_status_effect_prefab_from_file = StatusEffect();
  vector<string> sorted_item_files = vector<string>();
  vector<string> sorted_treasure_chest_files = vector<string>();
  vector<string> sorted_ability_files = vector<string>();
  vector<string> sorted_unit_files = vector<string>();
  vector<string> sorted_map_files = vector<string>();
  vector<string> sorted_status_effect_files = vector<string>();
  void start(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_theme(Game &game);
  void add_obstacle_and_warp_point_sprites(Game &game);
  void handle_input_events(Game &game);
  void update_nav_window(Game &game);
  void update_tilesheet_window(Game &game);
  void update_file_window(Game &game);
  void update_inspect_window(Game &game);
  void update_inspect_none(Game &game);
  void update_inspect_tile(Game &game);
  void update_inspect_unit(Game &game);
  void update_inspect_treasure_chest(Game &game);
  void update_inspect_item(Game &game);
  void update_inspect_ability(Game &game);
  void update_inspect_status_effect(Game &game);
  void display_sprite_srcs(Game &game, Image image, vector<SpriteSrc> &srcs,
                           string base_label);
  Unit &get_inspected_unit(Game &game);
  Item &get_inspected_item(Game &game);
  TreasureChest &get_inspected_treasure_chest(Game &game);
  void destroy();
};

#endif // EDITOR_H