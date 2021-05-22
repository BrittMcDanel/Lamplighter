#include "map.h"
#include "game.h"
#include "game_events.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "tween.h"
#include "utils.h"
#include "utils_game.h"
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Map::Map() {
  tiles = vector<Tile>();
  layers = vector<vector<Sprite>>();
  for (size_t i = 0; i < MAX_LAYERS; i++) {
    layers.push_back(vector<Sprite>());
  }
  rows = 0;
  cols = 0;
}

// deserialize calls this constructor
Map::Map(Game &game) {
  tiles = vector<Tile>();
  layers = vector<vector<Sprite>>();
  for (size_t i = 0; i < MAX_LAYERS; i++) {
    layers.push_back(vector<Sprite>());
  }
  rows = 0;
  cols = 0;
  move_point_icons = MovePointIcons(game);
  battle_move_action_point_text = Text(game, 10, FontColor::WhiteShadow, "",
                                       Vec2(0, 0), 150, TextAlignment::Left);
  battle_action_point_text = Text(game, 10, FontColor::WhiteShadow, "",
                                  Vec2(0, 0), 150, TextAlignment::Left);
  battle_action_point_description_text =
      Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
           TextAlignment::Left);
}

Map::Map(Game &game, int _rows, int _cols) {
  tiles = vector<Tile>();
  layers = vector<vector<Sprite>>();
  for (size_t i = 0; i < MAX_LAYERS; i++) {
    layers.push_back(vector<Sprite>());
  }
  move_point_icons = MovePointIcons(game);
  battle_move_action_point_text = Text(game, 10, FontColor::WhiteShadow, "",
                                       Vec2(0, 0), 150, TextAlignment::Left);
  battle_action_point_text = Text(game, 10, FontColor::WhiteShadow, "",
                                  Vec2(0, 0), 150, TextAlignment::Left);
  battle_action_point_description_text =
      Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
           TextAlignment::Left);
  rows = _rows;
  cols = _cols;
  rows_move_grid = _rows * MOVE_GRID_RATIO;
  cols_move_grid = _cols * MOVE_GRID_RATIO;

  // create a default map;
  auto s = rows * cols;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      auto world_point = tile_point_to_world_point(Vec2(i, j));
      auto tile = Tile();
      tile.set_to_default_tile(game);
      tile.sprite.dst.x = world_point.x;
      tile.sprite.dst.y = world_point.y;
      tiles.push_back(tile);

      // populate all layers (leaves empty data for tiles that don't
      // have layers but its more performant for rendering later, not sure
      // if this approach is correct or not)
      for (auto &layer_vec : layers) {
        auto sprite = Sprite();
        sprite.dst.x = world_point.x;
        sprite.dst.y = world_point.y;
        layer_vec.push_back(sprite);
      }
    }
  }

  add_all_player_units(game);

  auto u = Unit(game);
  u.set_tile_point(Vec2(10, 10));
  unit_dict[u.guid] = u;
}

void Map::process_game_events(Game &game) {
  auto &acting_unit = unit_dict[player_unit_guids.at(0)];
  while (!game_events.empty()) {
    GameEvent event = game_events.front();
    game_events.pop();
    switch (event.m_event_type) {
    case GameEventType::Move: {
      unit_dict[event.m_unit_guid].move_to(
          game, event.m_tile_point, 0,
          event.m_allow_units_to_path_through_each_other);
      break;
    }
    case GameEventType::CollectItemRequest: {
      fmt::print("Collect item request: {} -> {}\n",
                 to_string(event.m_item_guid),
                 item_dict.contains(event.m_item_guid));
      if (game.player.is_host && item_dict.contains(event.m_item_guid) &&
          !item_dict[event.m_item_guid].being_sent_to_player) {
        fmt::print("Collect item request: {}\n", to_string(event.m_item_guid));
        item_dict[event.m_item_guid].being_sent_to_player = true;
        game.game_client.SendMessage(GameEvent::collect_item_respond(
            game, event.m_unit_guid, event.m_item_guid));
      }
      break;
    }
    case GameEventType::CollectItemRespond: {
      fmt::print("Collect item: {}\n", to_string(event.m_item_guid));
      if (unit_dict.contains(event.m_unit_guid)) {
        unit_dict[event.m_unit_guid].send_item_to_player(game,
                                                         event.m_item_guid);
      }
      break;
    }
    default: {
      fmt::print("Map::process_game_event: Invalid m_event_type: {}\n",
                 event.m_event_type);
      abort();
    }
    }
  }
}

void Map::update(Game &game) {
  GAME_ASSERT(player_unit_guids.size() > 0);
  item_guids_to_remove_at_end_of_frame.clear();
  battle_guids_to_remove_at_end_of_frame.clear();
  ability_handles_to_release_at_end_of_frame.clear();
  process_game_events(game);
  // updates
  // clear some generic data (used by the editor)
  treasure_chest_input.clear();
  unit_input.clear();
  item_input.clear();

  for (auto &tile : tiles) {
    tile.update(game);
  }
  for (auto &layer_vec : layers) {
    for (auto &sprite : layer_vec) {
      sprite.update(game);
    }
  }
  for (auto &entry : treasure_chest_dict) {
    // cout << "treasure chest guid " << entry.first << "\n";
    auto key = entry.first;
    auto &treasure_chest = entry.second;
    treasure_chest.update(game);
    if (!treasure_chest_input.is_mouse_over) {
      treasure_chest_input.guid = key;
      treasure_chest_input.is_mouse_over =
          treasure_chest.sprite.input_events.is_mouse_over ||
          treasure_chest.opened_sprite.input_events.is_mouse_over;
    }
    if (treasure_chest.sprite.input_events.is_click ||
        treasure_chest.opened_sprite.input_events.is_click) {
      treasure_chest_input.guid = key;
      treasure_chest_input.clicked = true;
    }
    if (treasure_chest.sprite.input_events.is_right_click ||
        treasure_chest.opened_sprite.input_events.is_right_click) {
      treasure_chest_input.guid = key;
      treasure_chest_input.right_clicked = true;
    }
  }
  // sort the units, result is in the set_sorted_unit_guid vec
  set_sorted_unit_guids();
  for (auto &guid : sorted_unit_guids) {
    auto &unit = unit_dict[guid];
    unit.update(game);
    // set is mouse over if not already set ( can be set from true to false
    // otherwise)
    if (!unit_input.is_mouse_over) {
      unit_input.guid = guid;
      unit_input.is_mouse_over = unit.sprite.input_events.is_mouse_over;
    }
    if (unit.sprite.input_events.is_click) {
      unit_input.guid = guid;
      unit_input.clicked = true;
    }
    if (unit.sprite.input_events.is_right_click) {
      unit_input.guid = guid;
      unit_input.right_clicked = true;
    }
  }

  for (auto &entry : item_dict) {
    auto key = entry.first;
    auto &item = entry.second;
    item.update(game);
    if (!item_input.is_mouse_over) {
      item_input.guid = key;
      item_input.is_mouse_over = item.sprite.input_events.is_mouse_over;
    }
    if (item.sprite.input_events.is_click) {
      item_input.guid = key;
      item_input.clicked = true;
    }
    if (item.sprite.input_events.is_right_click) {
      item_input.guid = key;
      item_input.right_clicked = true;
    }
  }
  for (auto &ability : abilities.items) {
    if (ability.in_use_in_pool) {
      ability.update(game);
    }
  }
  for (auto &battle : battle_dict) {
    battle.second.update(game);
  }
  // this rect is not needed as the timeout tweens are just time based
  auto tmp_rect = Rect(0, 0, 0, 0);
  ability_timeout_tweens.update(game, tmp_rect);

  auto &acting_unit = unit_dict[player_unit_guids.at(0)];
  if (!acting_unit.in_battle) {
    if (treasure_chest_input.is_mouse_over || unit_input.is_mouse_over) {
      game.engine.set_cursor(CursorType::Hand);
    }
  }

  // user input
  if (game.engine.mouse_in_game_rect) {
    auto camera_move_dist = 3;
    auto wasd_pressed = false;
    if (game.engine.keys_held_down[(int)SDLK_w]) {
      wasd_pressed = true;
      game.engine.camera.dst.y += camera_move_dist;
    }
    if (game.engine.keys_held_down[(int)SDLK_a]) {
      wasd_pressed = true;
      game.engine.camera.dst.x -= camera_move_dist;
    }
    if (game.engine.keys_held_down[(int)SDLK_s]) {
      wasd_pressed = true;
      game.engine.camera.dst.y -= camera_move_dist;
    }
    if (game.engine.keys_held_down[(int)SDLK_d]) {
      wasd_pressed = true;
      game.engine.camera.dst.x += camera_move_dist;
    }
    if (game.engine.keys_down[(int)SDLK_i]) {
      game.ui.equip_window.is_hidden = !game.ui.equip_window.is_hidden;
    }

    if (wasd_pressed) {
      game.engine.camera.camera_mode = CameraMode::FreeControl;
    }
  }

  if (game.editor_state.no_editor_or_editor_and_in_play_mode()) {
    pickup_nearby_items(game, acting_unit);
    check_if_battle_start(game, acting_unit);
    // make sure the mouse is in the game rect
    if (game.engine.mouse_in_game_rect) {
      if (acting_unit.in_battle) {
        update_battle_input(game, acting_unit);
      } else {
        update_non_battle_input(game, acting_unit);
      }
    }

    if (game.engine.camera.camera_mode == CameraMode::FollowPlayer) {
      game.engine.camera.center_on_rect(acting_unit.sprite.dst,
                                        acting_unit.sprite.hitbox_dims,
                                        game.engine.base_resolution);
    }
  }

  game.engine.camera.keep_in_map_bounds(game, rows, cols);

  // handle any tween callbacks
  for (auto item_guid : item_guids_to_remove_at_end_of_frame) {
    item_dict.erase(item_guid);
  }
  for (auto ability_handle : ability_handles_to_release_at_end_of_frame) {
    abilities.release_handle(ability_handle);
  }
  for (auto battle_guid : battle_guids_to_remove_at_end_of_frame) {
    battle_dict.erase(battle_guid);
  }
}

void Map::draw(Game &game) {
  for (auto &tile : tiles) {
    tile.draw(game);
  }
  range_ability_target.draw(game);
  turn_order_ability_target.draw(game);
  displayed_ability_target.draw(game);
  for (auto &layer_vec : layers) {
    // layer draw order
    for (int j = cols - 1; j >= 0; j--) {
      for (int i = 0; i < rows; i++) {
        auto idx = twod_to_oned_idx(Vec2(i, j), rows);
        auto &sprite = layer_vec[idx];
        sprite.draw(game);
      }
    }
  }
  for (auto &entry : treasure_chest_dict) {
    auto key = entry.first;
    auto &treasure_chest = entry.second;
    treasure_chest.draw(game);
  }
  for (auto &guid : sorted_unit_guids) {
    auto &unit = unit_dict[guid];
    unit.unit_ui_before_unit.draw(game);
  }
  for (auto &guid : sorted_unit_guids) {
    auto &unit = unit_dict[guid];
    unit.draw(game);
  }
  move_point_icons.draw(game);

  for (auto &entry : item_dict) {
    auto key = entry.first;
    auto &item = entry.second;
    item.draw(game);
  }
  for (auto &guid : sorted_unit_guids) {
    auto &unit = unit_dict[guid];
    unit.unit_ui_after_unit.draw(game);
  }
  for (auto &guid : sorted_unit_guids) {
    auto &unit = unit_dict[guid];
    unit.unit_ui_last_layer.draw(game);
  }
  battle_move_action_point_text.draw(game);
  battle_action_point_text.draw(game);
  battle_action_point_description_text.draw(game);
  for (auto &ability : abilities.items) {
    if (ability.in_use_in_pool) {
      ability.draw(game);
    }
  }
}

void Map::update_battle_input(Game &game, Unit &acting_unit) {
  if (!game.ui.is_mouse_over_ui && acting_unit.is_ability_selected &&
      game.engine.is_right_mouse_up) {
    acting_unit.is_ability_selected = false;
  }
  update_ability_selected(game, acting_unit,
                          acting_unit.get_selected_ability(game));

  auto &battle = battle_dict[acting_unit.battle_guid];
  auto &ability = acting_unit.get_selected_ability(game);
  auto is_player_turn =
      game.map.is_guid_in_all_player_units(battle.acting_unit_guid);
  game.ui.action_point_display.is_hidden = !is_player_turn;
  displayed_ability_target.is_hidden =
      game.ui.is_mouse_over_ui ||
      (!acting_unit.is_ability_selected ||
       (acting_unit.is_ability_selected &&
        acting_unit.get_selected_ability(game).stats.aoe.current == 1));
  range_ability_target =
      game.ability_targets.get_target(ability, AbilityTargetDims::Range);
  range_ability_target.set_dst(game, acting_unit.sprite.dst.get_xy(),
                               acting_unit.sprite.hitbox_dims);
  range_ability_target.is_hidden =
      !is_player_turn || ability.is_melee ||
      !(acting_unit.is_ability_selected ||
        (!acting_unit.is_ability_selected && unit_input.is_mouse_over));
  battle_action_point_text.is_hidden = true;
  battle_move_action_point_text.is_hidden = true;
  battle_action_point_description_text.is_hidden = true;
  battle_action_point_text.font_color = FontColor::WhiteShadow;
  battle_move_action_point_text.font_color = FontColor::WhiteShadow;
  battle_action_point_description_text.font_color = FontColor::WhiteShadow;
  move_point_icons.update(game);
  battle_move_action_point_text.update(game);
  battle_action_point_text.update(game);
  battle_action_point_description_text.update(game);
  turn_order_ability_target.update(game);
  range_ability_target.update(game);

  auto mouse_tile_point_move_grid = world_point_to_tile_point_move_grid(
      Vec2(game.engine.mouse_point_game_rect_scaled_camera));
  auto start = acting_unit.sprite.tile_point_hit_box.get_xy();
  auto target = mouse_tile_point_move_grid;
  if (unit_input.is_mouse_over) {
    auto &receiving_unit = unit_dict[unit_input.guid];
    target =
        get_unit_directional_target(acting_unit.sprite.tile_point_hit_box,
                                    receiving_unit.sprite.tile_point_hit_box);
  }

  battle_show_and_check_for_move(game, acting_unit, start, target);
}

void Map::battle_show_and_check_for_move(Game &game, Unit &acting_unit,
                                         Vec2 start, Vec2 target) {
  GAME_ASSERT(battle_dict.contains(acting_unit.battle_guid));

  auto &battle = battle_dict[acting_unit.battle_guid];
  auto &ability = acting_unit.get_selected_ability(game);
  auto ability_in_range =
      is_ability_in_range(ability, acting_unit.sprite.dst.get_xy(),
                          game.engine.mouse_point_game_rect_scaled_camera);
  if (game.ui.is_mouse_over_ui || acting_unit.is_battle_acting ||
      !game.map.is_guid_in_all_player_units(battle.acting_unit_guid)) {
    move_point_icons.hide_all();
    if (game.ui.is_mouse_over_ui && !acting_unit.is_ability_selected) {
      game.ui.action_point_display.active = 0;
    }
    if (acting_unit.is_ability_selected) {
      update_battle_ap_text(game, acting_unit, ability,
                            ability.stats.action_points.current);
    }
    return;
  }

  auto ap_cost = 0;
  auto move_ap = 0;
  if (acting_unit.is_ability_selected) {
    ap_cost = ability.stats.action_points.current;
  } else {
    auto ap_cost_pair = get_ap_of_move(game, acting_unit, start, target);
    ap_cost = ap_cost_pair.first;
    move_ap = ap_cost_pair.first;
    if (unit_input.is_mouse_over) {
      ap_cost += ability.stats.action_points.current;
    }
  }

  auto has_enough_ap = ap_cost <= acting_unit.stats.action_points.current;

  if (!game.ui.is_mouse_over_ui) {
    // default ability is ranged but unit is not in range
    if (!acting_unit.is_ability_selected && !ability.is_melee &&
        unit_input.is_mouse_over && !ability_in_range) {
      game.engine.set_cursor(CursorType::Invalid);
    }
    // selected ability is ranged and cursor is out of range
    else if (acting_unit.is_ability_selected && !ability.is_melee &&
             !ability_in_range) {
      game.engine.set_cursor(CursorType::Invalid);
    }
    // ability default or selected doesn't have enough ap to move and attack
    // or just attack
    else if (ability.is_melee && !has_enough_ap) {
      if (unit_input.is_mouse_over) {
        game.engine.set_cursor(CursorType::SwordInvalid);
      } else {
        game.engine.set_cursor(CursorType::Invalid);
      }
    } else if (unit_input.is_mouse_over) {
      game.engine.set_cursor(CursorType::Sword);
    }
  }

  update_battle_path(game, acting_unit, ability, start, target);
  update_battle_ap_text(game, acting_unit, ability, ap_cost);

  if (!game.ui.is_mouse_over_ui && has_enough_ap) {
    if (!acting_unit.is_moving) {
      game.ui.action_point_display.active = ap_cost;
    }
    if (game.engine.is_mouse_up) {
      // get new path as the user clicked while the unit was moving
      if (acting_unit.is_moving) {
        battle.clear_action_queue();
        move_point_icons.show_path(game, game.path_finder.path);
      }
      // alway dec an action point if this is the unit's first move.
      if (ap_cost == 0 && acting_unit.num_moves_this_turn == 0) {
        acting_unit.stats.action_points.dec_current(1);
      }
      acting_unit.num_moves_this_turn += 1;

      auto charge_ability_used = false;
      // regular move
      if (!acting_unit.is_ability_selected && !unit_input.is_mouse_over) {
        game.ui.action_point_display.active = 0;
        auto battle_action = BattleAction();
        battle_action.set_as_unit_move(
            acting_unit.guid,
            game.path_finder.path.at(game.path_finder.path.size() - 1));
        battle.add_battle_action(game, battle_action);
      }
      // ability is melee, move to get into range if necessary
      else if (unit_input.is_mouse_over && ability.is_melee) {
        if (ability.stats.cast_time.current > 0) {
          charge_ability_used = true;
        }
        acting_unit.is_ability_selected = false;
        game.ui.action_point_display.active = 0;

        // melee ability not in range, move to get into range
        if (!ability_in_range) {
          auto battle_action = BattleAction();
          battle_action.set_as_unit_move(
              acting_unit.guid,
              game.path_finder.path.at(game.path_finder.path.size() - 1));
          battle.add_battle_action(game, battle_action);
        }

        auto receiving_unit_guids = get_receiving_unit_guids(
            game, ability, game.engine.mouse_point_game_rect_scaled_camera);
        auto use_ability_battle_action = BattleAction();
        use_ability_battle_action.set_as_use_ability(
            acting_unit.guid, receiving_unit_guids, ability,
            game.engine.mouse_point_game_rect_scaled);
        battle.add_battle_action(game, use_ability_battle_action);
      }
      // non-melee ability
      else if (!ability.is_melee && ability_in_range) {
        // non-aoe ability but a unit wasn't clicked on, so return.
        if (ability.stats.aoe.current == 1 && !unit_input.is_mouse_over) {
          return;
        }
        if (ability.stats.cast_time.current > 0) {
          charge_ability_used = true;
        }
        acting_unit.is_ability_selected = false;
        game.ui.action_point_display.active = 0;
        auto receiving_unit_guids = get_receiving_unit_guids(
            game, ability, game.engine.mouse_point_game_rect_scaled_camera);
        auto use_ability_battle_action = BattleAction();
        use_ability_battle_action.set_as_use_ability(
            acting_unit.guid, receiving_unit_guids, ability,
            game.engine.mouse_point_game_rect_scaled);
        battle.add_battle_action(game, use_ability_battle_action);
      }

      // end turn if a charge ability was used
      if (charge_ability_used) {
        auto end_turn_battle_action = BattleAction();
        end_turn_battle_action.set_as_end_turn(acting_unit.guid);
        battle.add_battle_action(game, end_turn_battle_action);
      }

      // perform the actions
      battle.perform_next_battle_action(game);
    }
  }
}

// returns the ap cost of the move, as well as the closest point the unit
// could path to the target. This can be the target if the target could be
// pathed to.
pair<int, Vec2> Map::get_ap_of_move(Game &game, Unit &acting_unit, Vec2 start,
                                    Vec2 target) {
  auto actual_target = target;
  game.path_finder.set_path(game, *this, acting_unit, start, target, false);
  if (!game.path_finder.path_found) {
    game.path_finder.set_path(game, *this, acting_unit, start,
                              game.path_finder.closest_point_to_target, false);
    actual_target = game.path_finder.closest_point_to_target;
  }
  return make_pair(get_path_ap_cost(game.path_finder.path.size(),
                                    acting_unit.move_indexes_this_turn,
                                    acting_unit.num_moves_this_turn),
                   actual_target);
}

void Map::update_ability_selected(Game &game, Unit &acting_unit,
                                  const Ability &ability) {
  if (game.ui.is_mouse_over_ui) {
    return;
  }
  if (ability.ability_name == AbilityName::None) {
    return;
  }

  if (ability.stats.aoe.current != 1) {
    displayed_ability_target = game.ability_targets.get_target(ability);
    displayed_ability_target.set_dst(
        game, game.engine.mouse_point_game_rect_scaled_camera);
    displayed_ability_target.update(game);
  }

  auto receiving_unit_guids = get_receiving_unit_guids(
      game, ability, game.engine.mouse_point_game_rect_scaled_camera);
  for (auto unit_guid : receiving_unit_guids) {
    auto &u = unit_dict[unit_guid];
    u.unit_ui_after_unit.crosshairs.set_is_hidden(false);
  }
}

void Map::update_battle_path(Game &game, Unit &acting_unit,
                             const Ability &ability, Vec2 start, Vec2 target) {
  game.path_finder.set_path(game, *this, acting_unit, start, target, false);
  if (!game.path_finder.path_found) {
    game.path_finder.set_path(game, *this, acting_unit, start,
                              game.path_finder.closest_point_to_target, false);
  }
  // if you have 5 ap, you can move a full 5 ap + 1 full move ap - 1 pixel
  // so 5 ap means you can move 5 * MOVE_INDEXES_PER_ACTION_POINT +
  // (MOVE_INDEXES_PER_ACTION_POINT - 1)
  auto available_move_size = (acting_unit.stats.action_points.current *
                              MOVE_INDEXES_PER_ACTION_POINT) +
                             (MOVE_INDEXES_PER_ACTION_POINT - 1) -
                             acting_unit.move_indexes_this_turn;
  // can be < 0 becuase of the sub at the end
  if (available_move_size < 0) {
    available_move_size = 0;
  }

  // not enough action points to the move the full path,
  // erase everything that is greater than the action points allow.
  if (available_move_size >= 0 &&
      available_move_size < (int)game.path_finder.path.size()) {
    game.path_finder.path.erase(game.path_finder.path.begin() +
                                    available_move_size,
                                game.path_finder.path.end());
  }
  if (!ability.is_melee) {
    move_point_icons.hide_all();
  } else if (!acting_unit.is_moving) {
    move_point_icons.show_path(game, game.path_finder.path);
  }
}

void Map::update_battle_ap_text(Game &game, Unit &acting_unit,
                                const Ability &ability, int ap_cost) {
  auto ability_in_range =
      is_ability_in_range(ability, acting_unit.sprite.dst.get_xy(),
                          game.engine.mouse_point_game_rect_scaled_camera);
  battle_action_point_text.str = to_string(ap_cost) + " AP";

  battle_action_point_text.is_hidden = false;
  battle_action_point_description_text.is_hidden = false;
  battle_move_action_point_text.str =
      to_string(acting_unit.stats.action_points.current) + " AP";
  battle_action_point_description_text.str = "";
  if (!ability.is_melee && !ability_in_range) {
    battle_action_point_description_text.font_color = FontColor::RedShadow;
    battle_action_point_description_text.str = "Out of range";
  }
  if (ap_cost > acting_unit.stats.action_points.current) {
    battle_move_action_point_text.is_hidden = false;
    battle_action_point_description_text.font_color = FontColor::RedShadow;
    battle_action_point_description_text.str = "Not enough AP";
    battle_action_point_text.font_color = FontColor::RedShadow;
  }
  auto path_last_point =
      game.path_finder.path[game.path_finder.path.size() - 1];
  auto path_last_point_world_point =
      tile_point_to_world_point_move_grid(path_last_point);
  battle_move_action_point_text.dst.set_xy(path_last_point_world_point.x,
                                           path_last_point_world_point.y + 16);
  battle_action_point_text.dst.set_xy(
      Vec2(game.engine.mouse_point_game_rect_scaled_camera.x + 10,
           game.engine.mouse_point_game_rect_scaled_camera.y - 16));
  battle_action_point_description_text.dst.set_xy(Vec2(
      battle_action_point_text.dst.x, battle_action_point_text.dst.y - 12));

  battle_move_action_point_text.update(game);
  battle_action_point_text.update(game);
  battle_action_point_description_text.update(game);
}

vector<boost::uuids::uuid> Map::get_receiving_unit_guids(Game &game,
                                                         const Ability &ability,
                                                         Vec2 target_dst) {
  if (ability.stats.aoe.current != 1) {
    return get_units_in_aoe(game, ability, target_dst);
  } else if (game.map.unit_input.is_mouse_over) {
    if (game.map.unit_input.is_mouse_over) {
      return vector<boost::uuids::uuid>{game.map.unit_input.guid};
    } else {
      return vector<boost::uuids::uuid>();
    }
  }

  return vector<boost::uuids::uuid>();
}

vector<boost::uuids::uuid>
Map::get_units_in_aoe(Game &game, const Ability &ability, Vec2 target_dst) {
  auto receiving_units = vector<boost::uuids::uuid>();
  get_receiving_units_ability_target = game.ability_targets.get_target(ability);
  get_receiving_units_ability_target.set_dst(game, target_dst);
  get_receiving_units_ability_target.update(game);
  for (auto &entry : unit_dict) {
    auto unit_guid = entry.first;
    auto &unit = game.map.unit_dict[unit_guid];
    if (rect_contains_circle(
            get_receiving_units_ability_target.sprite.dst.get_center(),
            get_receiving_units_ability_target.sprite.dst.w / 2,
            unit.sprite.dst)) {
      receiving_units.push_back(unit_guid);
    }
  }
  return receiving_units;
}

void Map::start_ability_timeout(
    Game &game, boost::uuids::uuid acting_unit_guid,
    vector<boost::uuids::uuid> &receiving_unit_guids, const Ability &ability,
    Vec2 target_point, PerformAbilityContext _ability_context) {
  GAME_ASSERT(unit_dict.contains(acting_unit_guid));
  auto &acting_unit = unit_dict[acting_unit_guid];
  Uint32 duration = 500;
  auto start = Rect(0, 0, 0, 0);
  auto target = Rect(0, 0, 0, 0);
  auto callback = TweenCallback();
  callback.set_as_use_ability_timeout(
      acting_unit_guid, receiving_unit_guids, ability.ability_name,
      ability.stats.damage.current, target_point, _ability_context);
  ability_timeout_tweens.tween_xys.push_back(TweenXY(
      start, target, game.engine.current_time, duration, 0, callback, []() {},
      []() {}));
  // ui stuff
  acting_unit.unit_ui_after_unit.ability_box.is_hidden = false;
  acting_unit.unit_ui_after_unit.ability_box.set_text_str(game, acting_unit,
                                                          ability.display_name);
}

void Map::perform_ability(Game &game, boost::uuids::uuid acting_unit_guid,
                          vector<boost::uuids::uuid> &_receiving_unit_guids,
                          const Ability &ability, Vec2 target_dst,
                          PerformAbilityContext _ability_context) {
  GAME_ASSERT(unit_dict.contains(acting_unit_guid));
  auto &acting_unit = unit_dict[acting_unit_guid];
  if (_receiving_unit_guids.size() == 0) {
    return;
  }
  if (acting_unit.in_battle) {
    acting_unit.stats.action_points.dec_current(
        ability.stats.action_points.current);
  }
  acting_unit.is_battle_acting = true;
  auto ability_handle = abilities.get_handle(ability);
  abilities.items[ability_handle].guid = game.engine.get_guid();
  abilities.items[ability_handle].sprite.spawn_time = game.engine.current_time;
  abilities.items[ability_handle].tweens.clear();
  auto start = acting_unit.sprite.dst;
  auto target = acting_unit.sprite.dst;
  if (ability.stats.aoe.current == 1) {
    GAME_ASSERT(_receiving_unit_guids.size() > 0);
    GAME_ASSERT(unit_dict.contains(_receiving_unit_guids.at(0)));
    auto &receiving_unit_guid = _receiving_unit_guids.at(0);
    auto &receiving_unit = unit_dict[receiving_unit_guid];
    start = receiving_unit.sprite.dst;
    target = receiving_unit.sprite.dst;
  }
  if (abilities.items[ability_handle].is_projectile) {
    start = acting_unit.sprite.dst;
    if (ability.stats.aoe.current != 1) {
      target.set_xy(target_dst);
    }
  }

  auto receiving_unit_guids = _receiving_unit_guids;

  // if spawning multiple abilities, make sure only one of them is the final
  // ability, if you don't then advance turn will be called multiple times
  // in complete. We are not spawning multiple abilities yet however.
  auto callback = TweenCallback();
  callback.set_as_use_ability(
      acting_unit_guid, receiving_unit_guids, ability_handle,
      abilities.items[ability_handle].stats.damage.current, target_dst,
      _ability_context);
  if (ability.is_projectile) {
    abilities.items[ability_handle].tweens.tween_xys_constant_speed.push_back(
        TweenXYConstantSpeed(
            start, target, game.engine.current_time, ability.delay,
            ability.projectile_speed, callback, []() {}, []() {}));
  } else {
    auto duration = ability.sprite.srcs.size() * ability.sprite.anim_speed;
    abilities.items[ability_handle].tweens.tween_xys.push_back(TweenXY(
        start, target, game.engine.current_time, duration, ability.delay,
        callback, []() {}, []() {}));
  }
}

void Map::update_non_battle_input(Game &game, Unit &acting_unit) {
  if (game.ui.is_mouse_over_ui) {
    return;
  }
  if (acting_unit.in_dialogue) {
    handle_in_dialogue(game, acting_unit);
  } else if (unit_input.clicked && !acting_unit.is_ability_selected) {
    handle_on_unit_click(game, acting_unit, unit_input.guid);
  } else if (treasure_chest_input.clicked && !acting_unit.is_ability_selected) {
    handle_on_treasure_chest_click(game, acting_unit,
                                   treasure_chest_input.guid);
  } else if (!(treasure_chest_input.is_mouse_over || unit_input.is_mouse_over ||
               acting_unit.is_ability_selected)) {
    auto mouse_tile_point_move_grid = world_point_to_tile_point_move_grid(
        Vec2(game.engine.mouse_point_game_rect_scaled_camera));
    if (game.engine.is_mouse_down) {
      // acting_unit.move_to(game, mouse_tile_point_move_grid, 0, [](){});
      game.game_client.SendMessage(GameEvent::create_move_unit(
          game, acting_unit.guid, mouse_tile_point_move_grid, true));
    }
  }
}

void Map::check_if_battle_start(Game &game, Unit &acting_unit) {
  if (acting_unit.in_battle) {
    return;
  }
  for (auto &entry : unit_dict) {
    auto &unit = entry.second;
    if (unit.guid == acting_unit.guid) {
      continue;
    }
    if (unit.faction != Faction::Enemy) {
      continue;
    }
    if (unit.stats.hp.current_equals_lower_bound()) {
      continue;
    }
    auto delta_x = (double)(unit.sprite.dst.x - acting_unit.sprite.dst.x);
    auto delta_y = (double)(unit.sprite.dst.y - acting_unit.sprite.dst.y);
    auto delta_dist = sqrt(delta_x * delta_x + delta_y * delta_y);
    if (delta_dist < 100) {
      create_battle(game, acting_unit);
    }
  }
}

void Map::create_battle(Game &game, Unit &acting_unit) {
  auto battle = Battle(game);
  // this loop will add the acting unit as well
  for (auto &entry : unit_dict) {
    auto &unit = entry.second;
    if (unit.stats.hp.current_equals_lower_bound()) {
      continue;
    }
    auto delta_x = (double)(unit.sprite.dst.x - acting_unit.sprite.dst.x);
    auto delta_y = (double)(unit.sprite.dst.y - acting_unit.sprite.dst.y);
    auto delta_dist = sqrt(delta_x * delta_x + delta_y * delta_y);
    if (delta_dist < 400) {
      unit.in_battle = true;
      unit.battle_guid = battle.guid;
      unit.stop_moving(game);
      battle.unit_guids.push_back(unit.guid);
    }
  }
  battle.start(game);
  battle_dict[battle.guid] = battle;
  game.ui.battle_start_alert.show(game);
}

void Map::pickup_nearby_items(Game &game, Unit &acting_unit) {
  // automatically pick up nearby items
  for (auto &entry : item_dict) {
    auto &item = entry.second;
    auto delta_x = (double)(item.sprite.dst.x - acting_unit.sprite.dst.x);
    auto delta_y = (double)(item.sprite.dst.y - acting_unit.sprite.dst.y);
    auto delta_dist = sqrt(delta_x * delta_x + delta_y * delta_y);
    if (delta_dist < 50 && !item.being_sent_to_player) {
      game.game_client.SendMessage(
          GameEvent::collect_item_request(game, acting_unit.guid, item.guid));
    }
  }
}

// unit is in dialogue with another unit if this function is called
void Map::handle_in_dialogue(Game &game, Unit &acting_unit) {
  if (game.engine.is_mouse_up) {
    auto &dialogue_unit = unit_dict[acting_unit.in_dialogue_with_unit_guid];
    auto dialogue_over =
        dialogue_unit.show_next_dialogue(game, acting_unit.guid);
    if (dialogue_over) {
      dialogue_unit.in_dialogue = false;
      acting_unit.in_dialogue = false;
    }
  }
}

void Map::handle_on_unit_click(Game &game, Unit &acting_unit,
                               boost::uuids::uuid unit_guid) {
  auto &clicked_unit = unit_dict[unit_guid];
  if (clicked_unit.is_shop) {
    game.ui.shop_window.show_shop(unit_guid);
    acting_unit.stop_moving(game);
  } else if (clicked_unit.guid != acting_unit.guid &&
             clicked_unit.dialogues.size() > 0) {
    // have the unit move again towards the target when the dialogue
    // is over by subtracting path idx, it will do path idx + 1 again
    // when it starts to walk.
    clicked_unit.is_ai_walking = false;
    clicked_unit.ai_walk_path_idx -= 1;
    if (clicked_unit.ai_walk_path_idx < 0) {
      clicked_unit.ai_walk_path_idx = 0;
    }
    auto dialogue_over =
        clicked_unit.show_next_dialogue(game, acting_unit.guid);
    clicked_unit.in_dialogue = true;
    acting_unit.in_dialogue = true;
    acting_unit.in_dialogue_with_unit_guid = unit_input.guid;
    acting_unit.stop_moving(game);
  }
}

void Map::handle_on_treasure_chest_click(
    Game &game, Unit &acting_unit, boost::uuids::uuid treasure_chest_guid) {
  auto &treasure_chest = treasure_chest_dict[treasure_chest_guid];
  if (!treasure_chest.is_opened) {
    treasure_chest.is_opened = true;
    for (auto &item : treasure_chest.inventory.items) {
      // set item starting position to that of the tresaure chest
      auto item_cpy = item;
      item_cpy.guid = game.engine.get_guid();
      item_cpy.sprite.dst = treasure_chest.sprite.dst;
      item_dict[item_cpy.guid] = item_cpy;
      game.game_client.SendMessage(GameEvent::collect_item_request(
          game, acting_unit.guid, item_cpy.guid));
    }
  }
}

bool Map::point_in_bounds(Vec2 &p) {
  return p.x >= 0 && p.x <= rows - 1 && p.y >= 0 && p.y <= cols - 1;
}

bool Map::point_in_bounds_move_grid(Vec2 &p) {
  return p.x >= 0 && p.x <= rows_move_grid - 1 && p.y >= 0 &&
         p.y <= cols_move_grid - 1;
}

bool Map::unit_occupies_tile_point_move_grid(
    Game &game, const Rect &tile_point_hit_box,
    boost::uuids::uuid acting_unit_guid) {
  for (auto &entry : unit_dict) {
    auto &unit = entry.second;
    // skip the unit if it has the same tile point hit box as the skip
    // tile point hit box. This is because I am not using handles and need
    // a way to identify units.
    if (unit.guid == acting_unit_guid) {
      continue;
    }
    if (rect_contains_rect(tile_point_hit_box,
                           unit.sprite.tile_point_hit_box)) {
      return true;
    }
  }
  return false;
}

// if you remove while iterating through the item_dict item.update()
// calls sprite.update() calls tween.update() which can remove the item
// from the item_dict. This invalidates the item_dict while iterating.
// instead add it to a vec to be removed at the end of the frame.
void Map::remove_item_guid_at_end_of_frame(boost::uuids::uuid item_guid) {
  item_guids_to_remove_at_end_of_frame.push_back(item_guid);
}

void Map::set_sorted_unit_guids() {
  // clear previous unit guids
  sorted_unit_guids.clear();

  // put all the unit guids into the sorted unit guid vec
  for (auto &entry : unit_dict) {
    auto key = entry.first;
    sorted_unit_guids.push_back(key);
  }

  // sort the unit guids
  sort(sorted_unit_guids.begin(), sorted_unit_guids.end(),
       [&](boost::uuids::uuid guid1, boost::uuids::uuid guid2) -> bool {
         auto &unit1 = unit_dict[guid1];
         auto &unit2 = unit_dict[guid2];
         return unit1.sprite.dst.y > unit2.sprite.dst.y;
       });
}

Unit &Map::get_player_unit() {
  GAME_ASSERT(player_unit_guids.size() > 0);
  GAME_ASSERT(unit_dict.contains(player_unit_guids[0]));
  return unit_dict[player_unit_guids[0]];
}

bool Map::is_guid_in_all_player_units(boost::uuids::uuid unit_guid) {
  return find(all_player_unit_guids.begin(), all_player_unit_guids.end(),
              unit_guid) != all_player_unit_guids.end();
}

void Map::erase_unit_guid(Game &game, boost::uuids::uuid _unit_guid) {
  unit_dict.erase(_unit_guid);
  // remove from sorted unit guids so that [] doesn't add
  // an erased unit back into the unit dict
  for (int i = sorted_unit_guids.size() - 1; i >= 0; i--) {
    auto unit_guid = sorted_unit_guids[i];
    if (unit_guid == _unit_guid) {
      sorted_unit_guids.erase(sorted_unit_guids.begin() + i);
      break;
    }
  }
}

void Map::add_all_player_units(Game &game) {
  // each new game creates n total player controlled units that the host
  // and clients control. prefabs/maps json files do not contain these units
  // just the npcs / enemies. Save files serialize the map with the player
  // units as well.
  for (size_t i = 0; i < PLAYER_CONTROLLED_UNITS_SIZE; i++) {
    auto unit = Unit(game);
    unit_dict[unit.guid] = unit;
    all_player_unit_guids.push_back(unit.guid);
  }

  // add first unit to player unit guids
  auto player_guid = all_player_unit_guids.at(0);
  auto &player = unit_dict[player_guid];
  player_unit_guids.push_back(player_guid);
}

// can be made more efficient later if needed, focusing on geting it
// working for now.
void map_transition(Game &game, string &file_path,
                    Vec2 warp_to_map_tile_point) {
  if (file_path.size() == 0) {
    cout << "Map::map_transition - file path is empty. path: "
         << file_path.c_str() << "\n";
    abort();
  }
  // save persistent data before loading the new map
  auto player_unit_guids = game.map.player_unit_guids;
  auto all_player_unit_guids = game.map.all_player_unit_guids;
  auto player_controlled_units = vector<Unit>();
  for (auto &player_unit_guid : game.map.all_player_unit_guids) {
    auto &player_unit = game.map.unit_dict[player_unit_guid];
    // clear any move tweens as we are moving them into the new map
    player_unit.stop_moving(game);
    player_unit.is_moving = false;
    player_unit.in_dialogue = false;
    player_unit.set_tile_point(warp_to_map_tile_point);
    player_controlled_units.push_back(player_unit);
  }
  // get the new map
  game.map = map_deserialize_from_file(game, file_path.c_str());
  // add persistent data to the new map
  game.map.player_unit_guids = player_unit_guids;
  game.map.all_player_unit_guids = all_player_unit_guids;
  for (auto &player_unit : player_controlled_units) {
    game.map.unit_dict[player_unit.guid] = player_unit;
  }
}

void map_serialize(Game &game, Map &map, bool is_save_file) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_int("rows", map.rows);
  game.serializer.serialize_int("cols", map.cols);
  game.serializer.writer.String("tiles");
  game.serializer.writer.StartArray();
  for (auto &tile : map.tiles) {
    tile_serialize(game, tile);
  }
  game.serializer.writer.EndArray();
  for (int i = 0; i < MAX_LAYERS; i++) {
    auto layer_key = "layer_" + to_string(i);
    game.serializer.writer.String(layer_key.c_str());
    game.serializer.writer.StartArray();
    for (auto &sprite : map.layers[0]) {
      sprite_serialize(game, sprite);
    }
    game.serializer.writer.EndArray();
  }
  if (is_save_file) {
    game.serializer.writer.String("all_player_unit_guids");
    game.serializer.writer.StartArray();
    for (auto &player_unit_guid : map.all_player_unit_guids) {
      game.serializer.writer.String(to_string(player_unit_guid).c_str());
    }
    game.serializer.writer.EndArray();
  }
  game.serializer.writer.String("units");
  game.serializer.writer.StartArray();
  for (auto &entry : map.unit_dict) {
    auto &unit = entry.second;
    if (!is_save_file && map.is_guid_in_all_player_units(unit.guid)) {
      // skip serialization of this unit as its guid is a player guid
      continue;
    }
    unit_serialize(game, unit);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.String("treasure_chests");
  game.serializer.writer.StartArray();
  for (auto &entry : map.treasure_chest_dict) {
    auto &treasure_chest = entry.second;
    treasure_chest_serialize(game, treasure_chest);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.String("items");
  game.serializer.writer.StartArray();
  for (auto &entry : map.item_dict) {
    auto &item = entry.second;
    item_serialize(game, item);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Map map_deserialize(Game &game, GenericObject<false, Value> &obj,
                    bool is_save_file) {
  Map map = Map(game);
  map.rows = obj["rows"].GetInt();
  map.cols = obj["cols"].GetInt();
  map.rows_move_grid = map.rows * MOVE_GRID_RATIO;
  map.cols_move_grid = map.cols * MOVE_GRID_RATIO;
  auto tiles_array = obj["tiles"].GetArray();
  auto idx = 0;
  for (auto &tile_obj : tiles_array) {
    auto obj = tile_obj.GetObject();
    map.tiles.push_back(tile_deserialize(game, obj));
    idx += 1;
  }
  for (int i = 0; i < MAX_LAYERS; i++) {
    auto layer_key = "layer_" + to_string(i);
    auto layers_array = obj[layer_key.c_str()].GetArray();
    auto idx = 0;
    for (auto &sprite_obj : layers_array) {
      auto obj = sprite_obj.GetObject();
      map.layers[i].push_back(sprite_deserialize(game, obj));
      idx += 1;
    }
  }
  // save file expects all_player_guids to be present
  if (is_save_file) {
    if (!obj.HasMember("all_player_unit_guids")) {
      cout << "Map::desserialize - is_save_file, obj does not contain "
              "all_player_unit_guids.\n";
      abort();
    }
    // clear previous default unit guids placed in by maps constructor
    // as we are about to add the ones from the save file
    map.all_player_unit_guids.clear();
    auto all_player_unit_guids_array = obj["all_player_unit_guids"].GetArray();
    for (auto &player_unit_guid_obj : all_player_unit_guids_array) {
      auto guid = game.engine.string_gen(player_unit_guid_obj.GetString());
      map.all_player_unit_guids.push_back(guid);
    }
  }
  auto units_array = obj["units"].GetArray();
  for (auto &unit_obj : units_array) {
    auto obj = unit_obj.GetObject();
    auto unit = unit_deserialize(game, obj);
    map.unit_dict[unit.guid] = unit;
  }
  auto treasure_chests_array = obj["treasure_chests"].GetArray();
  for (auto &treasure_chest_obj : treasure_chests_array) {
    auto obj = treasure_chest_obj.GetObject();
    auto treasure_chest = treasure_chest_deserialize(game, obj);
    map.treasure_chest_dict[treasure_chest.guid] = treasure_chest;
  }
  auto items_array = obj["items"].GetArray();
  for (auto &item_obj : items_array) {
    auto obj = item_obj.GetObject();
    auto item = item_deserialize(game, obj);
    map.item_dict[item.guid] = item;
  }
  return map;
}

void map_serialize_into_file(Game &game, Map &map, const char *file_path,
                             bool is_save_file) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  map_serialize(game, map, is_save_file);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "map_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Map map_deserialize_from_file(Game &game, const char *file_path,
                              bool is_save_file) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "map_deserialize_from_file. File error " << file_path << "\n";
    abort();
  }
  stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  // cout << "buff " << buffer.str().c_str() << "\n";
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  // parse
  game.serializer.doc.Parse(buffer.str().c_str());
  // unit object
  auto obj = game.serializer.doc.GetObject();
  // deserialize unit_obj into a Unit
  return map_deserialize(game, obj, is_save_file);
}