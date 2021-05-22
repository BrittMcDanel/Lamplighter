#include "battle.h"
#include "ability_targets.h"
#include "game.h"
#include "utils_game.h"
#include <algorithm>

void BattleAction::set_as_unit_move(boost::uuids::uuid _acting_unit_guid,
                                    Vec2 _tile_point) {
  action_type = BattleActionType::Move;
  acting_unit_guid = _acting_unit_guid;
  tile_point = _tile_point;
}

void BattleAction::set_as_use_ability(
    boost::uuids::uuid _acting_unit_guid,
    vector<boost::uuids::uuid> _receiving_unit_guids, const Ability &_ability,
    Vec2 _ability_target_dst) {
  action_type = BattleActionType::UseAbility;
  acting_unit_guid = _acting_unit_guid;
  receiving_unit_guids = _receiving_unit_guids;
  ability = _ability;
  ability_target_dst = _ability_target_dst;
}

void BattleAction::set_as_end_turn(boost::uuids::uuid _acting_unit_guid) {
  action_type = BattleActionType::EndTurn;
  acting_unit_guid = _acting_unit_guid;
}

Battle::Battle(Game &game) { guid = game.engine.get_guid(); }

void Battle::update(Game &game) {
  // end_battle erases the battle in map at the end of frame,
  // so this condition should only execute once.
  if (is_battle_over(game)) {
    auto all_allies_dead = all_faction_units_dead(game, Faction::Ally);
    auto all_enemies_dead = all_faction_units_dead(game, Faction::Enemy);
    if (all_allies_dead) {
      cout << "All allies in the battle are dead.\n";
    } else if (all_enemies_dead) {
      cout << "All enemies in the battle are dead.\n";
    }
    end_battle(game);
    return;
  }
}

// assumes units have been added in already
void Battle::start(Game &game) {
  GAME_ASSERT(unit_guids.size() > 0);
  // shuffle units for random turn order
  shuffle(unit_guids.begin(), unit_guids.end(), game.engine.rnd);
  acting_unit_guid = unit_guids.at(0);
  auto &acting_unit = game.map.unit_dict[acting_unit_guid];
  if (!game.map.is_guid_in_all_player_units(acting_unit.guid)) {
    enqueue_ai_actions(game);
  }
}

void Battle::end_turn(Game &game) {
  // dec acting unit's status effects
  auto &acting_unit = game.map.unit_dict[acting_unit_guid];
  acting_unit.dec_status_effects(game);
  // dec all charge abilities once after turn ended
  dec_charging_actions(game);
  // start the charge action loop (will continue until no more
  // charge actions have a charge time that == 0)
  perform_next_charge_action(game);
}

void Battle::perform_next_battle_action(Game &game) {
  current_context = PerformAbilityContext::Default;
  if (action_queue.size() == 0) {
    GAME_ASSERT(game.map.unit_dict.contains(acting_unit_guid));
    auto &acting_unit = game.map.unit_dict[acting_unit_guid];
    // player unit is out of ap, end turn
    if (acting_unit.stats.action_points.current_equals_lower_bound() &&
        game.map.is_guid_in_all_player_units(acting_unit.guid)) {
      end_turn(game);
    }
    return;
  }
  // make a copy as pop invalidates the reference from front
  auto next_action = action_queue.front();
  action_queue.pop();

  perform_battle_action(game, next_action, PerformAbilityContext::Default);
}

void Battle::perform_next_charge_action(Game &game) {
  current_context = PerformAbilityContext::Charging;
  auto perform_charging_action_idx = -1;
  for (size_t i = 0; i < charging_actions.size(); i++) {
    auto &charging_action = charging_actions[i];
    if (charging_action.ability.stats.cast_time.current_equals_lower_bound()) {
      perform_charging_action_idx = (int)i;
      break;
    }
  }
  if (perform_charging_action_idx != -1) {
    auto charging_action = charging_actions[perform_charging_action_idx];
    charging_actions.erase(charging_actions.begin() +
                           perform_charging_action_idx);
    perform_battle_action(game, charging_action,
                          PerformAbilityContext::Charging);
  } else {
    advance_turn(game);
  }
}

void Battle::perform_next_counter(Game &game) {
  if (counter_queue.size() == 0) {
    if (current_context == PerformAbilityContext::Default) {
      perform_next_battle_action(game);
    } else if (current_context == PerformAbilityContext::Charging) {
      perform_next_charge_action(game);
    } else {
      cout << "Battle::perform_next_counter - unexpected context "
           << (int)current_context << "\n";
      abort();
    }
    return;
  }

  // make a copy as pop invalidates the reference from front
  auto next_action = counter_queue.front();
  counter_queue.pop();

  perform_battle_action(game, next_action, PerformAbilityContext::Counter);
}

void Battle::advance_turn(Game &game) {
  // all units could be dead so it won't be able to find the next
  // active unit
  if (is_battle_over(game)) {
    return;
  }

  size_t acting_unit_idx = 0;
  for (size_t i = 0; i < unit_guids.size(); i++) {
    auto unit_guid = unit_guids[i];
    if (unit_guid == acting_unit_guid) {
      acting_unit_idx = i;
      break;
    }
  }

  auto iters = 0;
  while (true) {
    iters += 1;
    if (iters > 1000) {
      cout << "Battle:advance_turn - iters > 1000.\n";
      abort();
    }
    acting_unit_idx += 1;
    if (acting_unit_idx > unit_guids.size() - 1) {
      acting_unit_idx = 0;
    }
    acting_unit_guid = unit_guids.at(acting_unit_idx);
    auto &unit = game.map.unit_dict[acting_unit_guid];
    // this acting_unit would be dead, keep searching
    if (unit.stats.hp.current_equals_lower_bound()) {
      continue;
    } else {
      break;
    }
  }

  auto &acting_unit = game.map.unit_dict[acting_unit_guid];
  // restore ap
  acting_unit.stats.action_points.current = acting_unit.stats.action_points.max;
  // if enemy or non player ally do ai actions
  if (acting_unit.faction == Faction::Enemy ||
      !game.map.is_guid_in_all_player_units(acting_unit_guid)) {
    enqueue_ai_actions(game);
  }
}

bool Battle::all_faction_units_dead(Game &game, Faction faction) {
  for (auto unit_guid : unit_guids) {
    auto &unit = game.map.unit_dict[unit_guid];
    if (unit.faction == faction &&
        !unit.stats.hp.current_equals_lower_bound()) {
      return false;
    }
  }
  return true;
}

bool Battle::is_battle_over(Game &game) {
  return all_faction_units_dead(game, Faction::Ally) ||
         all_faction_units_dead(game, Faction::Enemy);
}

void Battle::end_battle(Game &game) {
  // set all units to not in battle, including enemies. Dead enemies
  // can't trigger battles so its fine.
  for (auto unit_guid : unit_guids) {
    auto &unit = game.map.unit_dict[unit_guid];
    unit.in_battle = false;
    // restore ap and hp
    unit.stats.action_points.set_current_to_max();
    unit.stats.hp.set_current_to_max();
    // remove any status effects added in the battle
    unit.remove_all_battle_status_effects(game);
  }
  // add the battle guid to the vec of battle guids to be removed
  // at the end of the frame.
  game.map.battle_guids_to_remove_at_end_of_frame.push_back(guid);
}

void Battle::dec_charging_actions(Game &game) {
  for (auto &action : charging_actions) {
    action.ability.stats.cast_time.dec_current(1);
  }
}

void Battle::add_battle_action(Game &game, BattleAction &battle_action) {
  if (battle_action.action_type == BattleActionType::UseAbility &&
      battle_action.ability.stats.cast_time.current > 0) {
    charging_actions.push_back(battle_action);
    return;
  }
  action_queue.push(battle_action);
}

void Battle::perform_battle_action(Game &game, BattleAction &battle_action,
                                   PerformAbilityContext _ability_context) {
  if (battle_action.action_type == BattleActionType::Move) {
    GAME_ASSERT(game.map.unit_dict.contains(battle_action.acting_unit_guid));
    auto &acting_unit = game.map.unit_dict[battle_action.acting_unit_guid];
    acting_unit.move_to(game, battle_action.tile_point, 0, false);
  } else if (battle_action.action_type == BattleActionType::UseAbility) {
    GAME_ASSERT(game.map.unit_dict.contains(battle_action.acting_unit_guid));
    auto &acting_unit = game.map.unit_dict[battle_action.acting_unit_guid];
    acting_unit.is_battle_acting = true;
    game.map.start_ability_timeout(
        game, battle_action.acting_unit_guid,
        battle_action.receiving_unit_guids, battle_action.ability,
        battle_action.ability_target_dst, _ability_context);
  } else if (battle_action.action_type == BattleActionType::EndTurn) {
    end_turn(game);
  } else {
    cout << "Battle::perform_battle_action. action_type not handled "
         << static_cast<int>(battle_action.action_type) << "\n";
    abort();
  }
}

void Battle::clear_action_queue() {
  while (action_queue.size() != 0) {
    action_queue.pop();
  }
}

void Battle::enqueue_ai_actions(Game &game) {
  GAME_ASSERT(game.map.unit_dict.contains(acting_unit_guid));
  auto &acting_unit = game.map.unit_dict[acting_unit_guid];

  auto start = acting_unit.sprite.tile_point_hit_box.get_xy();
  auto closest_unit = get_closest_faction_unit(
      game, acting_unit_guid, get_opposite_faction(acting_unit.faction));
  auto target = Vec2(0, 0);
  if (closest_unit.first) {
    auto &closest_u = game.map.unit_dict[closest_unit.second];
    target = closest_u.sprite.tile_point_hit_box.get_xy();
  }

  auto ap_remaining = acting_unit.stats.action_points.current;
  auto move_ap_cost_pair =
      game.map.get_ap_of_move(game, acting_unit, start, target);
  auto move_ap_cost = move_ap_cost_pair.first;
  target = move_ap_cost_pair.second;
  ap_remaining -= move_ap_cost;

  auto move_action = BattleAction();
  move_action.set_as_unit_move(acting_unit.guid, target);
  add_battle_action(game, move_action);

  if (closest_unit.first) {
    auto &closest_u = game.map.unit_dict[closest_unit.second];
    auto ability_action = BattleAction();
    auto rnd_int = game.engine.get_random_int(0, 100);
    if (rnd_int < 25) {
      ability_action.set_as_use_ability(
          acting_unit.guid, vector<boost::uuids::uuid>{closest_unit.second},
          game.assets.get_ability(AbilityName::Fire),
          closest_u.sprite.dst.get_xy());
    } else {
      ability_action.set_as_use_ability(
          acting_unit.guid, vector<boost::uuids::uuid>{closest_unit.second},
          get_default_attack_ability(game, acting_unit),
          closest_u.sprite.dst.get_xy());
    }
    add_battle_action(game, ability_action);
  }

  auto end_turn_action = BattleAction();
  end_turn_action.set_as_end_turn(acting_unit_guid);
  add_battle_action(game, end_turn_action);

  perform_next_battle_action(game);
}

pair<bool, boost::uuids::uuid>
Battle::get_closest_faction_unit(Game &game,
                                 boost::uuids::uuid _acting_unit_guid,
                                 Faction faction_to_search_for) {
  GAME_ASSERT(game.map.unit_dict.contains(_acting_unit_guid));
  auto &acting_unit = game.map.unit_dict[_acting_unit_guid];
  auto min_unit_found = false;
  boost::uuids::uuid min_unit_guid;
  auto min_dist = 1000000;
  for (auto &unit_guid : unit_guids) {
    if (unit_guid == _acting_unit_guid) {
      continue;
    }
    auto &unit = game.map.unit_dict[unit_guid];
    if (unit.faction == faction_to_search_for) {
      auto dist =
          manhattan_distance(acting_unit.sprite.tile_point_hit_box.get_xy(),
                             unit.sprite.tile_point_hit_box.get_xy());
      if (dist < min_dist) {
        min_unit_found = true;
        dist = min_dist;
        min_unit_guid = unit.guid;
      }
    }
  }
  return make_pair(min_unit_found, min_unit_guid);
}

vector<boost::uuids::uuid>
Battle::get_receiving_unit_guids(Game &game, Ability &ability,
                                 AbilityTarget &ability_target) {
  if (ability.stats.aoe.current != 1) {
    return get_units_in_aoe(game, ability_target);
  } else if (game.map.unit_input.is_mouse_over) {
    auto &receiving_unit = game.map.unit_dict[game.map.unit_input.guid];
    return vector<boost::uuids::uuid>{receiving_unit.guid};
  }
}

vector<boost::uuids::uuid>
Battle::get_units_in_aoe(Game &game, AbilityTarget &ability_target) {
  auto receiving_units = vector<boost::uuids::uuid>();
  for (auto unit_guid : unit_guids) {
    auto &unit = game.map.unit_dict[unit_guid];
    if (rect_contains_circle(ability_target.sprite.dst.get_center(),
                             ability_target.sprite.dst.w / 2,
                             unit.sprite.dst)) {
      receiving_units.push_back(unit_guid);
    }
  }
  return receiving_units;
}