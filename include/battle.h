#ifndef BATTLE_H
#define BATTLE_H

#include "constants.h"
#include "robin_hood.h"
#include "unit.h"
#include <SDL.h>
#include <ability.h>
#include <array>
#include <boost/uuid/uuid.hpp>
#include <queue>
#include <vector>
using namespace std;

struct Game;
struct AbilityTarget;

struct BattleAction {
  BattleActionType action_type;
  boost::uuids::uuid acting_unit_guid;
  vector<boost::uuids::uuid> receiving_unit_guids;
  Ability ability;
  Vec2 tile_point;
  Vec2 ability_target_dst;
  BattleAction() = default;
  void set_as_unit_move(boost::uuids::uuid _acting_unit_guid, Vec2 _tile_point);
  void set_as_use_ability(boost::uuids::uuid _acting_unit_guid,
                          vector<boost::uuids::uuid> _receiving_unit_guids,
                          const Ability &_ability, Vec2 _ability_target_dst);
  void set_as_end_turn(boost::uuids::uuid _acting_unit_guid);
};

struct Battle {
  boost::uuids::uuid guid;
  boost::uuids::uuid acting_unit_guid;
  vector<boost::uuids::uuid> unit_guids;
  queue<BattleAction> action_queue;
  vector<BattleAction> charging_actions;
  queue<BattleAction> counter_queue;
  PerformAbilityContext current_context = PerformAbilityContext::Default;
  Battle() = default;
  Battle(Game &game);
  void update(Game &game);
  void start(Game &game);
  void perform_next_battle_action(Game &game);
  void perform_battle_action(Game &game, BattleAction &battle_action,
                             PerformAbilityContext _ability_context);
  void clear_action_queue();
  void end_turn(Game &game);
  void advance_turn(Game &game);
  void enqueue_ai_actions(Game &game);
  void add_battle_action(Game &game, BattleAction &battle_action);
  void dec_charging_actions(Game &game);
  void dec_status_effects(Game &game);
  void perform_next_charge_action(Game &game);
  void perform_next_counter(Game &game);
  bool all_faction_units_dead(Game &game, Faction faction);
  bool is_battle_over(Game &game);
  void end_battle(Game &game);
  pair<bool, boost::uuids::uuid>
  get_closest_faction_unit(Game &game, boost::uuids::uuid _acting_unit_guid,
                           Faction faction_to_search_for);
  vector<boost::uuids::uuid>
  get_receiving_unit_guids(Game &game, Ability &ability,
                           AbilityTarget &ability_target);
  vector<boost::uuids::uuid> get_units_in_aoe(Game &game,
                                              AbilityTarget &ability_target);
};

#endif // BATTLE_H