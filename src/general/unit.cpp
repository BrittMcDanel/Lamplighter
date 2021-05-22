#include "unit.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils_game.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Unit::Unit(Game &game) {
  guid = game.engine.get_guid();
  faction = Faction::Ally;
  display_name = string("");
  inventory = Inventory(game);
  stats = Stats(Range(40), Range(6), Range(1), Range(1), Range(1), Range(1));
  abilities = vector<Ability>();
  abilities.push_back(game.assets.get_ability(AbilityName::Fire));
  abilities.push_back(game.assets.get_ability(AbilityName::FireBall));
  ability_slots = vector<vector<Ability>>();
  for (size_t i = 0; i < NUM_ABILITY_SLOT_ROWS; i++) {
    ability_slots.push_back(vector<Ability>());
    for (size_t j = 0; j < NUM_ABILITY_SLOTS_PER_ROW; j++) {
      ability_slots[i].push_back(Ability());
    }
  }
  ability_slots[0][0] = game.assets.get_ability(AbilityName::Bullet);
  ability_slots[0][1] = game.assets.get_ability(AbilityName::SwordSlash);
  ability_slots[0][2] = game.assets.get_ability(AbilityName::Fire);
  ability_slots[0][4] = game.assets.get_ability(AbilityName::FireBall);
  ability_slots[0][8] = game.assets.get_ability(AbilityName::Resolve);
  ability_slots[0][19] = game.assets.get_ability(AbilityName::FireBall);
  ability_slots[0][20] = game.assets.get_ability(AbilityName::FireBall);
  ability_slots[1][10] = game.assets.get_ability(AbilityName::FireBall);

  equipped_abilities = EquippedAbilities(game);
  equipment = Equipment(game);
  selected_ability_idx = make_pair(0, 0);
  unit_ui_before_unit = UnitUIBeforeUnit(game);
  unit_ui_after_unit = UnitUIAfterUnit(game);
  unit_ui_last_layer = UnitUILastLayer(game);
  sprite = UnitSprite(game);
  sprite.can_receive_input_events = true;
  dialogues = vector<Dialogue>();
  auto dialogue = Dialogue();
  dialogue.game_flag_condition = GameFlag::Always;
  dialogue.strs.push_back("Have you talked to Bob yet?");
  dialogue.strs.push_back("He is near the east side of town.");
  dialogue.strs.push_back("After you talk to him, tell me what he said.");
  dialogues.push_back(dialogue);
  ai_walk_paths = vector<AIWalkPath>();
  ai_walk_path_idx = 0;
  dialogue_idx = 0;
  in_dialogue_with_unit_guid;
  is_ai_walking = false;
  is_moving = false;
  coin = get_coin_item(game);
  cash = get_cash_item(game);
}

void Unit::set_tile_point(Vec2 tile_point) {
  auto _tile_point_move_grid = tile_point_to_tile_point_move_grid(tile_point);
  auto world_point = tile_point_to_world_point_move_grid(_tile_point_move_grid);
  sprite.dst.x = world_point.x;
  sprite.dst.y = world_point.y;
  sprite.tile_point_hit_box.x = _tile_point_move_grid.x;
  sprite.tile_point_hit_box.y = _tile_point_move_grid.y;
  // add the initial walk tile point to the units starting point
  auto ai_walk_path = AIWalkPath(_tile_point_move_grid, 0);
  if (ai_walk_paths.size() > 0) {
    ai_walk_paths[0] = ai_walk_path;
  } else {
    ai_walk_paths.push_back(ai_walk_path);
  }
}

void Unit::set_tile_point_move_grid(Vec2 _tile_point_move_grid) {
  auto world_point = tile_point_to_world_point_move_grid(_tile_point_move_grid);
  sprite.dst.x = world_point.x;
  sprite.dst.y = world_point.y;
  sprite.tile_point_hit_box.x = _tile_point_move_grid.x;
  sprite.tile_point_hit_box.y = _tile_point_move_grid.y;
  // add the initial walk tile point to the units starting point
  auto ai_walk_path = AIWalkPath(_tile_point_move_grid, 0);
  if (ai_walk_paths.size() > 0) {
    ai_walk_paths[0] = ai_walk_path;
  } else {
    ai_walk_paths.push_back(ai_walk_path);
  }
}

void Unit::update(Game &game) {
  if (stats.hp.current_equals_lower_bound()) {
    sprite.anim_state = UnitAnimState::Dead;
  }
  sprite.update(game);
  coin.update(game);
  unit_ui_before_unit.update(game, *this);
  unit_ui_after_unit.update(game, *this);
  unit_ui_last_layer.update(game, *this);
  if (game.editor_state.no_editor_or_editor_and_in_play_mode() &&
      !in_dialogue && !is_ai_walking) {
    do_next_ai_walk(game);
  }
}

// the map draws the units before ui and after ui
void Unit::draw(Game &game) { sprite.draw(game); }

// returns true when the dialogue sequence is over
bool Unit::show_next_dialogue(Game &game,
                              boost::uuids::uuid _talking_to_unit_handle) {
  // clear any move tweens as the unit is no win dialogue
  sprite.tweens.clear();
  for (auto &dialogue : dialogues) {
    if (game.is_game_flag_set(dialogue.game_flag_condition)) {
      // dialogue is over
      if (dialogue_idx > (int)dialogue.strs.size() - 1) {
        unit_ui_after_unit.dialogue_box.is_hidden = true;
        dialogue_idx = 0;
        return true;
      }
      in_dialogue_with_unit_guid = _talking_to_unit_handle;
      unit_ui_after_unit.dialogue_box.is_hidden = false;
      unit_ui_after_unit.dialogue_box.set_text_str(game, *this,
                                                   dialogue.strs[dialogue_idx]);
      dialogue_idx += 1;
      return false;
    }
  }
  return false;
}

void Unit::do_next_ai_walk(Game &game) {
  // every unit starts out with an ai walk point of their starting point
  // so they can get back to their starting point after doing their walk
  // cycle.
  if (ai_walk_paths.size() <= 1) {
    return;
  }
  ai_walk_path_idx += 1;
  if (ai_walk_path_idx > (int)ai_walk_paths.size() - 1) {
    ai_walk_path_idx = 0;
  }
  auto &ai_walk_path = ai_walk_paths.at(ai_walk_path_idx);
  is_ai_walking = true;
  move_to(game, ai_walk_path.target_point, ai_walk_path.delay, true);
}

void Unit::move_to(Game &game, Vec2 target, Uint32 _delay,
                   bool allow_units_to_path_through_each_other) {
  // clear previous tweens (clears all of them right now, if needed maybe just
  // cancel move tweens later).
  stop_moving(game);
  auto unit_tile_point = get_tile_point();
  game.path_finder.set_path(game, game.map, *this, unit_tile_point, target,
                            allow_units_to_path_through_each_other);
  /*if (!game.path_finder.path_found) {
    game.path_finder.set_path(game, game.map, *this, unit_tile_point,
                              game.path_finder.closest_point_to_target,
                              allow_units_to_path_through_each_other);
  }*/
  if (game.path_finder.path.size() == 0) {
    return;
  }

  // cout << "path size " << game.path_finder.path.size() << "\n";
  auto prev_tile_point = unit_tile_point;
  // first prev world point should be the sprite's dst, not the world point from
  // the index. if the grid is larger it would cause a jarring snap effect from
  // the sprites dst to the world point of the index initially.
  auto prev_world_point = sprite.dst;
  // set move icon pos
  if (!in_battle && is_player_unit_guid(game.map.player_unit_guids, guid)) {
    auto target_world_point = tile_point_to_world_point_move_grid(target);
    unit_ui_before_unit.move_icon.dst.x = target_world_point.x;
    unit_ui_before_unit.move_icon.dst.y = target_world_point.y;
    unit_ui_before_unit.move_icon.is_hidden = false;
  }

  Uint32 delay = _delay;
  for (size_t i = 0; i < game.path_finder.path.size(); i++) {
    auto p = game.path_finder.path[i];
    auto move_speed = 30;
    if (manhattan_distance(prev_tile_point, p) > 1) {
      move_speed = (int)(move_speed * 1.5);
    }
    // cout << "path " << i << " " << p.x << " " << p.y << "\n";
    auto world_point_xy = tile_point_to_world_point_move_grid(p);
    auto world_point = Rect(world_point_xy.x, world_point_xy.y, 0, 0);
    auto callback = TweenCallback();
    callback.set_as_unit_move_callback(guid, p,
                                       i == game.path_finder.path.size() - 1);
    sprite.tweens.tween_xys.emplace_back(TweenXY(
        prev_world_point, world_point, game.engine.current_time,
        (Uint32)move_speed, delay, callback, []() {}, []() {}));
    prev_tile_point = p;
    prev_world_point = world_point;
    delay += move_speed;
  }
}

void Unit::send_item_to_player(Game &game, boost::uuids::uuid item_guid) {
  auto &item = game.map.item_dict.at(item_guid);
  // make sure this is set
  item.being_sent_to_player = true;
  auto callback = TweenCallback();
  callback.set_as_send_item_to_unit_callback(guid, item_guid);
  item.sprite.tweens.tween_xys_speed_moving_target.emplace_back(
      TweenXYSpeedMovingTarget(
          item.sprite.dst, sprite.dst, TweenInterpType::QuadraticIn,
          game.engine.current_time, 0, 2, TweenMovingTargetType::Unit, guid,
          callback, []() {}, []() {}));
}

Vec2 Unit::get_tile_point() {
  return Vec2(sprite.tile_point_hit_box.x, sprite.tile_point_hit_box.y);
}

void Unit::stop_moving(Game &game) {
  sprite.tweens.clear();
  is_moving = false;
  unit_ui_before_unit.move_icon.is_hidden = true;
}

void Unit::add_battle_text(Game &game, string &_text) {
  auto handle = unit_ui_last_layer.battle_texts.battle_texts.get_handle(
      BattleText(game, *this, _text));
  auto &battle_text =
      unit_ui_last_layer.battle_texts.battle_texts.items.at(handle);
  auto callback = TweenCallback();
  callback.set_as_battle_text(guid, handle);
  auto top_center = sprite.dst.get_top_center();
  auto text_start =
      Rect(top_center.x, top_center.y, sprite.dst.w, sprite.dst.h);
  auto text_target = text_start;
  text_target.y += 50;
  battle_text.damage_text.tweens.tween_xys.push_back(TweenXY(
      text_start, text_target, game.engine.current_time, 500, 1000, callback,
      []() {}, []() {}));
}

void Unit::add_status_effect(Game &game, const StatusEffect &_status_effect) {
  for (auto &status_effect : status_effects) {
    if (status_effect.status_effect_name == _status_effect.status_effect_name) {
      // reset turns remaining if the unit already has the status effect
      status_effect.turns_remaining = _status_effect.turns_remaining;
      return;
    }
  }

  // unit doesn't have the status effect, add it
  status_effects.push_back(_status_effect);
}

void Unit::dec_status_effects(Game &game) {
  for (int i = (int)status_effects.size() - 1; i >= 0; i--) {
    auto &status_effect = status_effects[i];
    status_effect.turns_remaining -= 1;
    if (status_effect.turns_remaining <= 0) {
      status_effects.erase(status_effects.begin() + i);
    }
  }
}

void Unit::remove_all_battle_status_effects(Game &game) {
  // just clearing all status effects for now, later there may
  // be just status effects that were added during a battle
  // that need to be removed.
  status_effects.clear();
}

const Ability &Unit::get_selected_ability(Game &game) {
  if (!is_ability_selected) {
    return get_default_attack_ability(game, *this);
  } else {
    return ability_slots[selected_ability_idx.first]
                        [selected_ability_idx.second];
  }
}

bool Unit::has_ability(AbilityName _ability_name) {
  for (auto &ability : abilities) {
    if (ability.ability_name == _ability_name) {
      return true;
    }
  }
  return false;
}

void Unit::add_ability(const Ability &ability) {
  if (ability.ability_name == AbilityName::None) {
    return;
  }
  if (!has_ability(ability.ability_name)) {
    abilities.push_back(ability);
  }
}

void unit_serialize(Game &game, Unit &unit) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_string_val("guid", to_string(unit.guid));
  game.serializer.serialize_int("unit_name", static_cast<int>(unit.unit_name));
  game.serializer.serialize_int("faction", static_cast<int>(unit.faction));
  game.serializer.serialize_int("money", unit.coin.quantity);
  game.serializer.serialize_vec2("hitbox_dims_input_events",
                                 unit.sprite.hitbox_dims_input_events);
  game.serializer.serialize_tile_point_hitbox(unit.sprite.hitbox_dims,
                                              unit.sprite.tile_point_hit_box);
  game.serializer.serialize_rect("dst", unit.sprite.dst);
  GAME_ASSERT(unit.sprite.image.image_name != ImageName::None);
  game.serializer.serialize_int("image_name",
                                (int)unit.sprite.image.image_name);
  game.serializer.serialize_int("portrait_anim_speed",
                                unit.sprite.portrait_anim_speed);
  game.serializer.serialize_int("idle_anim_speed", unit.sprite.idle_anim_speed);
  game.serializer.serialize_int("walk_anim_speed", unit.sprite.walk_anim_speed);
  game.serializer.serialize_int("attack_anim_speed",
                                unit.sprite.attack_anim_speed);
  game.serializer.serialize_int("cast_anim_speed", unit.sprite.cast_anim_speed);
  game.serializer.serialize_int("hit_anim_speed", unit.sprite.hit_anim_speed);
  game.serializer.serialize_int("dead_anim_speed", unit.sprite.dead_anim_speed);
  game.serializer.serialize_sprite_src_vec("portrait", unit.sprite.portrait);
  game.serializer.serialize_sprite_src_vec("idle_down", unit.sprite.idle_down);
  game.serializer.serialize_sprite_src_vec("walk_down", unit.sprite.walk_down);
  game.serializer.serialize_sprite_src_vec("attack_down",
                                           unit.sprite.attack_down);
  game.serializer.serialize_sprite_src_vec("cast_down", unit.sprite.cast_down);
  game.serializer.serialize_sprite_src_vec("hit_down", unit.sprite.hit_down);
  game.serializer.serialize_sprite_src_vec("dead_down", unit.sprite.dead_down);
  game.serializer.serialize_bool("is_shop", unit.is_shop);
  game.serializer.writer.String("inventory");
  game.serializer.writer.StartArray();
  for (auto &item : unit.inventory.items) {
    item_serialize(game, item);
  }
  game.serializer.writer.EndArray();
  game.serializer.serialize_bool("is_ai_walking", unit.is_ai_walking);
  game.serializer.serialize_bool("is_moving", unit.is_moving);
  game.serializer.serialize_bool("in_dialogue", unit.in_dialogue);
  game.serializer.serialize_dialogues(unit.dialogues);
  game.serializer.serialize_int("dialogue_idx", unit.dialogue_idx);
  game.serializer.serialize_string_val(
      "in_dialogue_with_unit_guid", to_string(unit.in_dialogue_with_unit_guid));
  game.serializer.serialize_ai_walk_paths(unit.ai_walk_paths);
  game.serializer.serialize_int("ai_walk_path_idx", unit.ai_walk_path_idx);
  game.serializer.writer.String("tweens");
  tweens_serialize(game, unit.sprite.tweens);
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Unit unit_deserialize(Game &game, GenericObject<false, Value> &obj,
                      bool use_static_asset_data) {
  auto unit = Unit(game);
  unit.guid = game.engine.string_gen(obj["guid"].GetString());
  unit.unit_name = static_cast<UnitName>(obj["unit_name"].GetInt());

  if (use_static_asset_data) {
    auto &static_unit = game.assets.get_unit(unit.unit_name);
    unit.sprite.image = static_unit.sprite.image;
    unit.sprite.portrait_anim_speed = static_unit.sprite.portrait_anim_speed;
    unit.sprite.idle_anim_speed = static_unit.sprite.idle_anim_speed;
    unit.sprite.walk_anim_speed = static_unit.sprite.walk_anim_speed;
    unit.sprite.attack_anim_speed = static_unit.sprite.attack_anim_speed;
    unit.sprite.cast_anim_speed = static_unit.sprite.cast_anim_speed;
    unit.sprite.hit_anim_speed = static_unit.sprite.hit_anim_speed;
    unit.sprite.dead_anim_speed = static_unit.sprite.dead_anim_speed;
    unit.sprite.portrait = static_unit.sprite.portrait;
    unit.sprite.idle_down = static_unit.sprite.idle_down;
    unit.sprite.walk_down = static_unit.sprite.walk_down;
    unit.sprite.attack_down = static_unit.sprite.attack_down;
    unit.sprite.cast_down = static_unit.sprite.cast_down;
    unit.sprite.hit_down = static_unit.sprite.hit_down;
    unit.sprite.dead_down = static_unit.sprite.dead_down;
    unit.sprite.hitbox_dims_input_events =
        static_unit.sprite.hitbox_dims_input_events;
  } else {
    // called from Assets::start when it initially loads all the prefab
    // files.
    unit.sprite.image =
        game.engine.get_image((ImageName)obj["image_name"].GetInt());
    unit.sprite.portrait_anim_speed = obj["portrait_anim_speed"].GetInt();
    unit.sprite.idle_anim_speed = obj["idle_anim_speed"].GetInt();
    unit.sprite.walk_anim_speed = obj["walk_anim_speed"].GetInt();
    unit.sprite.attack_anim_speed = obj["attack_anim_speed"].GetInt();
    unit.sprite.cast_anim_speed = obj["cast_anim_speed"].GetInt();
    unit.sprite.hit_anim_speed = obj["hit_anim_speed"].GetInt();
    unit.sprite.dead_anim_speed = obj["dead_anim_speed"].GetInt();
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "portrait", unit.sprite.portrait);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "idle_down", unit.sprite.idle_down);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "walk_down", unit.sprite.walk_down);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "attack_down", unit.sprite.attack_down);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "cast_down", unit.sprite.cast_down);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "hit_down", unit.sprite.hit_down);
    game.serializer.deserialize_sprite_src_vec(
        obj, unit.sprite.image, "dead_down", unit.sprite.dead_down);
    game.serializer.deserialize_vec2(obj, "hitbox_dims_input_events",
                                     unit.sprite.hitbox_dims_input_events);
  }

  unit.faction = static_cast<Faction>(obj["faction"].GetInt());
  unit.coin.quantity = obj["money"].GetInt();
  game.serializer.deserialize_tile_point_hitbox(obj, unit.sprite.hitbox_dims,
                                                unit.sprite.tile_point_hit_box);
  game.serializer.deserialize_rect(obj, "dst", unit.sprite.dst);
  unit.is_shop = obj["is_shop"].GetBool();
  auto items_array = obj["inventory"].GetArray();
  for (auto &item_obj : items_array) {
    auto obj = item_obj.GetObject();
    auto item = item_deserialize(game, obj);
    unit.inventory.add_item(item);
  }
  unit.is_ai_walking = obj["is_ai_walking"].GetBool();
  unit.is_moving = obj["is_moving"].GetBool();
  unit.in_dialogue = obj["in_dialogue"].GetBool();
  game.serializer.deserialize_dialogues(obj, unit.dialogues);
  unit.dialogue_idx = obj["dialogue_idx"].GetInt();
  unit.in_dialogue_with_unit_guid =
      game.engine.string_gen(obj["in_dialogue_with_unit_guid"].GetString());
  game.serializer.deserialize_ai_walk_paths(obj, unit.ai_walk_paths);
  unit.ai_walk_path_idx = obj["ai_walk_path_idx"].GetInt();
  auto tweens_obj = obj["tweens"].GetObject();
  unit.sprite.tweens = tweens_deserialize(game, tweens_obj);
  return unit;
}

void unit_serialize_into_file(Game &game, Unit &unit, const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  unit_serialize(game, unit);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "unit_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Unit unit_deserialize_from_file(Game &game, const char *file_path,
                                bool use_static_asset_data) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "unit_deserialize_from_file. File error " << file_path << "\n";
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
  auto unit = unit_deserialize(game, obj, use_static_asset_data);
  // set a unique guid as this is a copy of the unit from the json file
  // and will have the guid in the json file.
  unit.guid = game.engine.get_guid();
  return unit;
}