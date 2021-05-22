#ifndef UNIT_H
#define UNIT_H
#include "ability.h"
#include "ai_walk_path.h"
#include "dialogue.h"
#include "inventory.h"
#include "rapidjson/document.h"
#include "robin_hood.h"
#include "sprite.h"
#include "status_effect.h"
#include "ui/unit_ui.h"
#include "unit_sprite.h"
#include "utils.h"
#include "utils_game.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct Unit {
  boost::uuids::uuid guid;
  UnitName unit_name = UnitName::None;
  UnitSprite sprite;
  Faction faction;
  string display_name;
  Item coin;
  Item cash;
  Inventory inventory = Inventory();
  Stats stats = Stats();
  EquippedAbilities equipped_abilities = EquippedAbilities();
  UnitUIBeforeUnit unit_ui_before_unit;
  UnitUIAfterUnit unit_ui_after_unit;
  UnitUILastLayer unit_ui_last_layer;
  vector<Ability> abilities;
  vector<vector<Ability>> ability_slots;
  vector<StatusEffect> status_effects;
  vector<Dialogue> dialogues;
  vector<AIWalkPath> ai_walk_paths;
  Equipment equipment;
  int num_moves_this_turn = 0;
  int move_indexes_this_turn = 0;
  int ai_walk_path_idx;
  int dialogue_idx;
  pair<int, int> selected_ability_idx = make_pair(0, 0);
  boost::uuids::uuid in_dialogue_with_unit_guid;
  boost::uuids::uuid battle_guid;
  bool in_dialogue = false;
  bool is_ai_walking = false;
  bool is_moving = false;
  bool is_shop = false;
  bool in_battle = false;
  bool is_battle_acting = false;
  bool is_ability_selected = false;
  Unit() = default;
  Unit(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void do_next_ai_walk(Game &game);
  bool show_next_dialogue(Game &game,
                          boost::uuids::uuid _talking_to_unit_handle);
  void move_to(Game &game, Vec2 target, Uint32 _delay,
               bool allow_units_to_path_through_each_other);
  void send_item_to_player(Game &game, boost::uuids::uuid item_guid);
  void stop_moving(Game &game);
  Vec2 get_tile_point();
  void set_tile_point(Vec2 tile_point);
  void set_tile_point_move_grid(Vec2 tile_point_move_grid);
  void add_battle_text(Game &game, string &_text);
  void add_status_effect(Game &game, const StatusEffect &_status_effect);
  void dec_status_effects(Game &game);
  void remove_all_battle_status_effects(Game &game);
  bool has_ability(AbilityName _ability_name);
  void add_ability(const Ability &ability);
  const Ability &get_selected_ability(Game &game);
};

void unit_serialize(Game &game, Unit &unit);
void unit_serialize_into_file(Game &game, Unit &unit, const char *file_path);
Unit unit_deserialize_from_file(Game &game, const char *file_path,
                                bool use_static_asset_data = true);
Unit unit_deserialize(Game &game, GenericObject<false, Value> &obj,
                      bool use_static_asset_data = true);

#endif // UNIT_H
