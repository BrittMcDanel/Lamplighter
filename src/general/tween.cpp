#include "tween.h"

#include <math.h>

#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

TweenCompletion::TweenCompletion() {
  started = false;
  completed = false;
}

void TweenCallback::set_as_unit_move_callback(boost::uuids::uuid _unit_guid,
                                              Vec2 _tile_point,
                                              bool _is_final_point_in_path) {
  cb_type = TweenCallbackType::UnitMove;
  unit_guid = _unit_guid;
  tile_point = _tile_point;
  is_final_point_in_path = _is_final_point_in_path;
}

void TweenCallback::set_as_send_item_to_unit_callback(
    boost::uuids::uuid _unit_guid, boost::uuids::uuid _item_guid) {
  cb_type = TweenCallbackType::SendItemToUnit;
  unit_guid = _unit_guid;
  item_guid = _item_guid;
}

void TweenCallback::set_as_item_pickup_display_complete(
    boost::uuids::uuid _panel_guid) {
  cb_type = TweenCallbackType::ItemPickupDisplayComplete;
  panel_guid = _panel_guid;
}

void TweenCallback::set_as_battle_text(boost::uuids::uuid _unit_guid,
                                       int _handle) {
  cb_type = TweenCallbackType::BattleText;
  unit_guid = _unit_guid;
  handle = _handle;
}

void TweenCallback::set_as_use_ability(
    boost::uuids::uuid _acting_unit_guid,
    vector<boost::uuids::uuid> _receiving_unit_guids, int _ability_handle,
    int _damage, Vec2 _target_dst, PerformAbilityContext _ability_context) {
  cb_type = TweenCallbackType::UseAbility;
  unit_guid = _acting_unit_guid;
  receiving_unit_guids = _receiving_unit_guids;
  handle = _ability_handle;
  damage = _damage;
  target_dst = _target_dst;
  ability_context = _ability_context;
}

void TweenCallback::set_as_use_ability_timeout(
    boost::uuids::uuid _acting_unit_guid,
    vector<boost::uuids::uuid> _receiving_unit_guids, AbilityName _ability_name,
    int _damage, Vec2 _target_dst, PerformAbilityContext _ability_context) {
  cb_type = TweenCallbackType::UseAbilityTimeout;
  unit_guid = _acting_unit_guid;
  receiving_unit_guids = _receiving_unit_guids;
  ability_name = _ability_name;
  damage = _damage;
  target_dst = _target_dst;
  ability_context = _ability_context;
}

void Tweens::update(Game &game, Rect &val) {
  for (int i = (int)tween_xys.size() - 1; i >= 0; i--) {
    auto &tween = tween_xys.at(i);
    auto tween_completion = tween.update(game, val);
    if (tween_completion.started) {
      tween.on_start();
      handle_tween_on_start(game, tween.callback);
    }
    if (tween_completion.completed) {
      auto on_complete = tween.on_complete;
      on_complete();
      handle_tween_on_complete(game, tween.callback);
      tween_xys.erase(tween_xys.begin() + i);
    }
  }

  for (int i = (int)tween_xys_constant_speed.size() - 1; i >= 0; i--) {
    auto &tween = tween_xys_constant_speed.at(i);
    auto tween_completion = tween.update(game, val);
    if (tween_completion.started) {
      tween.on_start();
      handle_tween_on_start(game, tween.callback);
    }
    if (tween_completion.completed) {
      auto on_complete = tween.on_complete;
      on_complete();
      handle_tween_on_complete(game, tween.callback);
      tween_xys_constant_speed.erase(tween_xys_constant_speed.begin() + i);
    }
  }

  // iterate backwards because tweens can be erased while iterating
  for (int i = (int)tween_xys_speed_moving_target.size() - 1; i >= 0; i--) {
    auto &tween = tween_xys_speed_moving_target.at(i);
    auto target_val = Rect(0, 0, 0, 0);
    GAME_ASSERT(tween.moving_target_type != TweenMovingTargetType::None);
    if (tween.moving_target_type == TweenMovingTargetType::Unit) {
      target_val = game.map.unit_dict[tween.moving_target_guid].sprite.dst;
    }
    auto tween_completion = tween.update(game, val, target_val);
    if (tween_completion.started) {
      tween.on_start();
      handle_tween_on_start(game, tween.callback);
    }
    if (tween_completion.completed) {
      auto on_complete = tween.on_complete;
      on_complete();
      handle_tween_on_complete(game, tween.callback);
      tween_xys_speed_moving_target.erase(
          tween_xys_speed_moving_target.begin() + i);
    }
  }
}

void Tweens::clear() {
  tween_xys.clear();
  tween_xys_constant_speed.clear();
  tween_xys_speed_moving_target.clear();
}

TweenXY::TweenXY(Rect _start_val, Rect _target_val, Uint32 current_time,
                 Uint32 _duration, Uint32 _delay, TweenCallback _callback,
                 std::function<void()> _on_start,
                 std::function<void()> _on_complete) {
  callback = _callback;
  start_val = _start_val;
  target_val = _target_val;
  spawn_time = current_time;
  duration = _duration;
  delay = _delay;
  current_time_spawn_time_delta = 0;
  has_started = false;
  on_start = _on_start;
  on_complete = _on_complete;
  double_point = DoublePoint((double)_start_val.x, (double)_start_val.y);
}

TweenXYConstantSpeed::TweenXYConstantSpeed(Rect _start_val, Rect _target_val,
                                           Uint32 current_time, Uint32 _delay,
                                           double _speed,
                                           TweenCallback _callback,
                                           std::function<void()> _on_start,
                                           std::function<void()> _on_complete) {
  callback = _callback;
  start_val = _start_val;
  target_val = _target_val;
  speed = _speed;
  spawn_time = current_time;
  delay = _delay;
  current_time_spawn_time_delta = 0;
  has_started = false;
  on_start = _on_start;
  on_complete = _on_complete;
  double_point = DoublePoint((double)_start_val.x, (double)_start_val.y);
}

TweenXYSpeedMovingTarget::TweenXYSpeedMovingTarget() {
  moving_target_type = TweenMovingTargetType::None;
}

TweenXYSpeedMovingTarget::TweenXYSpeedMovingTarget(
    Rect _start_val, Rect _target_val, TweenInterpType _tween_interp_type,
    Uint32 current_time, Uint32 _delay, double _speed,
    TweenMovingTargetType _moving_target_type,
    boost::uuids::uuid _moving_target_guid, TweenCallback _callback,
    std::function<void()> _on_start, std::function<void()> _on_complete) {
  callback = _callback;
  start_val = _start_val;
  target_val = _target_val;
  tween_interp_type = _tween_interp_type;
  speed = _speed;
  is_constant_speed = true;
  spawn_time = current_time;
  delay = _delay;
  current_time_spawn_time_delta = 0;
  has_started = false;
  on_start = _on_start;
  on_complete = _on_complete;
  double_point = DoublePoint((double)_start_val.x, (double)_start_val.y);
  moving_target_type = _moving_target_type;
  moving_target_guid = _moving_target_guid;
}

// tweens handle removing individual tweens when the tween is completed
// so you do not have to do that in the on complete function.
TweenCompletion TweenXY::update(Game &game, Rect &val) {
  auto tween_completion = TweenCompletion();
  auto current_time = game.engine.current_time;
  current_time_spawn_time_delta = current_time - spawn_time;
  val.x = start_val.x;
  val.y = start_val.y;
  // used to be <=, but I made it < so that delay of 0 occurr immediately.
  // this fixes a graphical glitch where stuff gets drawn one frame at a
  // different position. Change back if necessary or add a check for just delay
  // of 0.
  if (current_time < spawn_time + delay) {
    return tween_completion;
  }

  if (!has_started) {
    tween_completion.started = true;
  }

  auto frac = (current_time - (spawn_time + delay)) / (float)duration;
  if (frac > 1) {
    frac = 1;
  }
  auto diff_x = target_val.x - start_val.x;
  auto diff_y = target_val.y - start_val.y;
  auto x = diff_x * frac;
  auto y = diff_y * frac;

  val.x = start_val.x + (int)x;
  val.y = start_val.y + (int)y;

  if (current_time >= spawn_time + delay + duration) {
    tween_completion.completed = true;
  }

  return tween_completion;
}

// tweens handle removing individual tweens when the tween is completed
// so you do not have to do that in the on complete function.
TweenCompletion TweenXYConstantSpeed::update(Game &game, Rect &val) {
  auto tween_completion = TweenCompletion();
  auto current_time = game.engine.current_time;
  current_time_spawn_time_delta = current_time - spawn_time;
  val.x = start_val.x;
  val.y = start_val.y;
  // used to be <=, but I made it < so that delay of 0 occurr immediately.
  // this fixes a graphical glitch where stuff gets drawn one frame at a
  // different position. Change back if necessary or add a check for just delay
  // of 0.
  if (current_time < spawn_time + delay) {
    return tween_completion;
  }

  if (!has_started) {
    tween_completion.started = true;
  }
  // target transform tweens use Float_Point, a floating point Point struct.
  // this is because the x and y updates are fractional, and casting them to
  // int will result in never making progress towards the target. So,
  // the Float_Point is updated and the normal transform point is just an
  // int cast of the Float_Point.
  auto delta_x = target_val.x - double_point.x;
  auto delta_y = target_val.y - double_point.y;
  auto delta_dist = sqrt(delta_x * delta_x + delta_y * delta_y);
  float frac = speed / delta_dist;
  auto x = frac * delta_x;
  auto y = frac * delta_y;
  double_point.x += x;
  double_point.y += y;
  val.x = (int)double_point.x;
  val.y = (int)double_point.y;
  // some min dist to check for
  if (delta_dist < 4.0) {
    tween_completion.completed = true;
  }

  return tween_completion;
}

// tweens handle removing individual tweens when the tween is completed
// so you do not have to do that in the on complete function.
TweenCompletion TweenXYSpeedMovingTarget::update(Game &game, Rect &val,
                                                 Rect &_target_val) {
  // set target val to the moving _target_val
  target_val = _target_val;
  auto tween_completion = TweenCompletion();
  auto current_time = game.engine.current_time;
  current_time_spawn_time_delta = current_time - spawn_time;
  auto elapsed = current_time_spawn_time_delta + delay;
  double tween_time = std::min(1.0, elapsed / 1000.0);
  val.x = start_val.x;
  val.y = start_val.y;
  // used to be <=, but I made it < so that delay of 0 occurr immediately.
  // this fixes a graphical glitch where stuff gets drawn one frame at a
  // different position. Change back if necessary or add a check for just delay
  // of 0.
  if (current_time < spawn_time + delay) {
    return tween_completion;
  }

  if (!has_started) {
    tween_completion.started = true;
  }
  // target transform tweens use Float_Point, a floating point Point struct.
  // this is because the x and y updates are fractional, and casting them to
  // int will result in never making progress towards the target. So,
  // the Float_Point is updated and the normal transform point is just an
  // int cast of the Float_Point.
  auto delta_x = target_val.x - double_point.x;
  auto delta_y = target_val.y - double_point.y;
  auto delta_dist = sqrt(delta_x * delta_x + delta_y * delta_y);
  // cout << tween_time << endl;
  auto curr_speed = (1 - tween_time) * speed +
                    tween_time * tween_interp_update(speed, tween_interp_type);
  auto frac = curr_speed / delta_dist;
  auto x = frac * delta_x;
  auto y = frac * delta_y;
  double_point.x += x;
  double_point.y += y;
  val.x = (int)double_point.x;
  val.y = (int)double_point.y;
  // some min dist to check for
  if (delta_dist < 4.0) {
    tween_completion.completed = true;
  }

  return tween_completion;
}

void handle_tween_on_start(Game &game, TweenCallback &cb) {
  // nones are fine, some tweens don't want a callback
  switch (cb.cb_type) {
  case TweenCallbackType::None: {
    break;
  }
  case TweenCallbackType::UnitMove: {
    auto &unit = game.map.unit_dict[cb.unit_guid];
    unit.is_moving = true;
    unit.sprite.tile_point_hit_box.x = cb.tile_point.x;
    unit.sprite.tile_point_hit_box.y = cb.tile_point.y;
    break;
  }
  case TweenCallbackType::SendItemToUnit: {
    break;
  }
  case TweenCallbackType::ItemPickupDisplayComplete: {
    break;
  }
  case TweenCallbackType::BattleAlert: {
    break;
  }
  case TweenCallbackType::BattleText: {
    break;
  }
  case TweenCallbackType::UseAbility: {
    break;
  }
  case TweenCallbackType::UseAbilityTimeout: {
    break;
  }
  default: {
    cout << "Tween::handle_tween_on_start. cb_type not handled "
         << (int)cb.cb_type << "\n";
    abort();
  }
  }
}

void handle_tween_on_complete(Game &game, TweenCallback &cb) {
  // nones are fine, some tweens don't want a callback
  switch (cb.cb_type) {
  case TweenCallbackType::None: {
    break;
  }
  case TweenCallbackType::UnitMove: {
    auto &unit = game.map.unit_dict[cb.unit_guid];
    auto tile_point = move_grid_point_to_tile_point(cb.tile_point);
    auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
    auto &tile = game.map.tiles[tile_idx];
    if (tile.is_warp_point) {
      // clear move tweens
      unit.sprite.tweens.clear();
      unit.is_moving = false;
      unit.is_ai_walking = false;
      unit.unit_ui_before_unit.move_icon.is_hidden = true;
      unit.set_tile_point(tile.warps_to_tile_point);
    }
    if (unit.in_battle) {
      GAME_ASSERT(game.map.battle_dict.contains(unit.battle_guid));
      auto &battle = game.map.battle_dict[unit.battle_guid];
      unit.move_indexes_this_turn += 1;
      if (unit.move_indexes_this_turn == MOVE_INDEXES_PER_ACTION_POINT) {
        unit.stats.action_points.dec_current(1);
        unit.move_indexes_this_turn = 0;
      }
    }
    if (cb.is_final_point_in_path) {
      unit.is_moving = false;
      unit.is_ai_walking = false;
      unit.unit_ui_before_unit.move_icon.is_hidden = true;
      if (unit.in_battle) {
        GAME_ASSERT(game.map.battle_dict.contains(unit.battle_guid));
        auto &battle = game.map.battle_dict[unit.battle_guid];
        battle.perform_next_battle_action(game);
      }
    }
    break;
  }
  case TweenCallbackType::SendItemToUnit: {
    auto &unit = game.map.unit_dict[cb.unit_guid];
    auto &item = game.map.item_dict[cb.item_guid];
    // if the unit is in the player's control add the item to
    // their inventory.
    if (is_player_controlled_unit_guid(game.map.player_unit_guids,
                                       cb.unit_guid)) {
      if (item.item_type == ItemType::Money) {
        unit.coin.quantity += item.quantity;
      } else {
        unit.inventory.add_item(item);
      }
      // add item to ui item pickup list for display
      game.ui.item_pickup_list.add_item(game, item);
    }
    // erase item
    GAME_ASSERT(game.map.item_dict.contains(item.guid));
    game.map.remove_item_guid_at_end_of_frame(item.guid);
    break;
  }
  case TweenCallbackType::ItemPickupDisplayComplete: {
    game.ui.item_pickup_list.remove_panel_with_guid(game, cb.panel_guid);
    break;
  }
  case TweenCallbackType::BattleAlert: {
    game.ui.battle_start_alert.is_hidden = true;
    break;
  }
  case TweenCallbackType::BattleText: {
    GAME_ASSERT(game.map.unit_dict.contains(cb.unit_guid));
    auto &unit = game.map.unit_dict[cb.unit_guid];
    unit.unit_ui_last_layer.battle_texts.battle_texts.release_handle(cb.handle);
    break;
  }
  case TweenCallbackType::UseAbility: {
    // store all ability data thats needed then add it to the vec
    // to be released at the end of the frame
    auto &ability = game.map.abilities.get_item(cb.handle);
    auto ability_action_points = ability.stats.action_points.current;
    game.map.ability_handles_to_release_at_end_of_frame.push_back(cb.handle);

    // get the receiving units for aoe spells just when they go off
    // as units can move if its a charge spell.
    if (ability.stats.aoe.current != 1) {
      cb.receiving_unit_guids =
          game.map.get_receiving_unit_guids(game, ability, cb.target_dst);
    }

    for (auto receiving_unit_guid : cb.receiving_unit_guids) {
      GAME_ASSERT(game.map.unit_dict.contains(receiving_unit_guid));
      auto &receiving_unit = game.map.unit_dict[receiving_unit_guid];
      switch (ability.ability_type) {
      case AbilityType::None: {
        cout << "TweenCallback::UseAbility ability type of None was used.\n";
        abort();
        break;
      }
      case AbilityType::Damage: {
        receiving_unit.stats.hp.dec_current(cb.damage);
        auto text = to_string(cb.damage);
        receiving_unit.add_battle_text(game, text);
        break;
      }
      case AbilityType::Heal: {
        receiving_unit.stats.hp.inc_current(cb.damage);
        auto text = to_string(cb.damage);
        receiving_unit.add_battle_text(game, text);
        break;
      }
      case AbilityType::StatusEffect: {
        for (auto &status_effect_pct : ability.status_effect_pcts) {
          auto rnd_pct = game.engine.get_random_double(0, 1.0);
          if (rnd_pct <= status_effect_pct.pct) {
            receiving_unit.add_status_effect(game,
                                             status_effect_pct.status_effect);
            auto text = ability.display_name;
            receiving_unit.add_battle_text(game, text);
          }
        }
        break;
      }
      default: {
        cout << "TweenCallback::UseAbility ability type not handled "
             << (int)ability.ability_type << "\n";
        abort();
        break;
      }
      }
    }

    GAME_ASSERT(game.map.unit_dict.contains(cb.unit_guid));
    auto &acting_unit = game.map.unit_dict[cb.unit_guid];
    if (acting_unit.in_battle) {
      GAME_ASSERT(game.map.battle_dict.contains(acting_unit.battle_guid));
      acting_unit.is_battle_acting = false;
      auto &battle = game.map.battle_dict[acting_unit.battle_guid];

      // check for counters, can only counter normal or chargeing abilities
      // can't counter counter abilities (infinite cycle)
      if (cb.ability_context != PerformAbilityContext::Counter) {
        auto someone_countered = false;
        for (auto receiving_unit_guid : cb.receiving_unit_guids) {
          GAME_ASSERT(game.map.unit_dict.contains(receiving_unit_guid));
          // can't counter an ability you hit yourself with
          if (receiving_unit_guid == acting_unit.guid) {
            continue;
          }
          auto &receiving_unit = game.map.unit_dict[receiving_unit_guid];
          if (receiving_unit.equipped_abilities.counter.ability_name !=
              AbilityName::None) {
            someone_countered = true;
            auto battle_action = BattleAction();
            battle_action.set_as_use_ability(
                receiving_unit.guid,
                vector<boost::uuids::uuid>{acting_unit.guid},
                receiving_unit.equipped_abilities.counter, Vec2(0, 0));
            battle.counter_queue.push(battle_action);
          }
        }
        if (someone_countered) {
          battle.perform_next_counter(game);
          return;
        }
      }

      if (cb.ability_context == PerformAbilityContext::Counter) {
        battle.perform_next_counter(game);
      } else if (cb.ability_context == PerformAbilityContext::Charging) {
        battle.perform_next_charge_action(game);
      } else if (cb.ability_context == PerformAbilityContext::Default) {
        battle.perform_next_battle_action(game);
      } else {
        cout << "tween_handle_on_complete - TweenCallbackType::UseAbility - "
                "unexpected perform ability context: "
             << static_cast<int>(cb.ability_context) << "\n";
        abort();
      }
    }
    break;
  }
  case TweenCallbackType::UseAbilityTimeout: {
    GAME_ASSERT(game.map.unit_dict.contains(cb.unit_guid));
    auto &acting_unit = game.map.unit_dict[cb.unit_guid];
    acting_unit.unit_ui_after_unit.ability_box.is_hidden = true;
    game.map.perform_ability(game, cb.unit_guid, cb.receiving_unit_guids,
                             game.assets.get_ability(cb.ability_name),
                             cb.target_dst, cb.ability_context);
    break;
  }
  default: {
    cout << "Tween::handle_tween_on_complete. cb_type not handled "
         << (int)cb.cb_type << "\n";
    abort();
  }
  }
}

double tween_interp_update(double speed, TweenInterpType tween_interp_type) {
  switch (tween_interp_type) {
  case TweenInterpType::Linear: {
    return speed;
  }
  case TweenInterpType::QuadraticIn: {
    return speed * speed;
  }
  default:
    cout << "Error: tween_interp_update TweenInterpType not handled: "
         << (int)tween_interp_type << endl;
    abort();
  }
}

void tweens_serialize(Game &game, Tweens &tweens) {
  game.serializer.writer.StartObject();
  game.serializer.writer.String("tween_xys");
  game.serializer.writer.StartArray();
  for (auto &tween_xy : tweens.tween_xys) {
    game.serializer.writer.StartObject();
    game.serializer.serialize_rect("start_val", tween_xy.start_val);
    game.serializer.serialize_rect("target_val", tween_xy.start_val);
    game.serializer.serialize_uint("delay", tween_xy.delay);
    game.serializer.serialize_uint("duration", tween_xy.duration);
    // no need to serialize spawn time as engine current times will differ
    // just need to serialize the delta time
    game.serializer.serialize_uint("current_time_spawn_time_delta",
                                   tween_xy.current_time_spawn_time_delta);
    game.serializer.serialize_double_point("double_point",
                                           tween_xy.double_point);
    game.serializer.serialize_bool("has_started", tween_xy.has_started);
    game.serializer.writer.String("callback");
    game.serializer.serialize_tween_callback(tween_xy.callback);
    game.serializer.writer.EndObject();
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Tweens tweens_deserialize(Game &game, GenericObject<false, Value> &obj) {
  Tweens tweens = Tweens();
  auto tween_xy_array = obj["tween_xys"].GetArray();
  for (auto &tween_xy_obj : tween_xy_array) {
    auto tween_xy = TweenXY();
    game.serializer.deserialize_rect(obj, "start_val", tween_xy.start_val);
    game.serializer.deserialize_rect(obj, "target_val", tween_xy.target_val);
    game.serializer.deserialize_double_point(obj, "double_point",
                                             tween_xy.double_point);
    tween_xy.delay = tween_xy_obj["delay"].GetUint();
    tween_xy.duration = tween_xy_obj["duration"].GetUint();
    tween_xy.current_time_spawn_time_delta =
        tween_xy_obj["current_time_spawn_time_delta"].GetUint();
    tween_xy.has_started = tween_xy_obj["has_started"].GetBool();
    tween_xy.spawn_time =
        game.engine.current_time - tween_xy.current_time_spawn_time_delta;
    auto cb_obj = tween_xy_obj["callback"].GetObject();
    game.serializer.deserialize_tween_callback(game, cb_obj, tween_xy.callback);
    tweens.tween_xys.push_back(tween_xy);
  }
  return tweens;
}

void tweens_serialize_into_file(Game &game, Tweens &tweens,
                                const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  tweens_serialize(game, tweens);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "tweens_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Tweens tweens_deserialize_from_file(Game &game, const char *file_path) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "tweens_deserialize_from_file. File error " << file_path << "\n";
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
  return tweens_deserialize(game, obj);
}
