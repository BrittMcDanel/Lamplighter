#ifndef MAP_H
#define MAP_H
#include "ability.h"
#include "ability_targets.h"
#include "battle.h"
#include "game_events.h"
#include "item.h"
#include "pool.h"
#include "rapidjson/document.h"
#include "robin_hood.h"
#include "sprite.h"
#include "tile.h"
#include "treasure_chest.h"
#include "tween.h"
#include "ui/move_point_icons.h"
#include "unit.h"
#include "utils.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <vector>
using namespace std;
using namespace rapidjson;

#define MAX_LAYERS 10
// how many units are reserved for the players 1 for now until its figured
// out how it works
#define PLAYER_CONTROLLED_UNITS_SIZE 1

struct Game;

struct Map {
  // the unit guids of every player and player controlled unit
  // i.e all host and client player unit guids
  vector<boost::uuids::uuid> all_player_unit_guids =
      vector<boost::uuids::uuid>();
  // just the unit guids you control i.e either the host units
  // or a particular client's units
  vector<boost::uuids::uuid> player_unit_guids = vector<boost::uuids::uuid>();
  int rows = 0;
  int cols = 0;
  int rows_move_grid = 0;
  int cols_move_grid = 0;
  queue<GameEvent> game_events;
  vector<Tile> tiles = vector<Tile>();
  vector<vector<Sprite>> layers = vector<vector<Sprite>>();
  // keys are boost::uuids::uuid
  robin_hood::unordered_flat_map<boost::uuids::uuid, TreasureChest,
                                 BoostUUIDHash>
      treasure_chest_dict =
          robin_hood::unordered_flat_map<boost::uuids::uuid, TreasureChest,
                                         BoostUUIDHash>();
  robin_hood::unordered_flat_map<boost::uuids::uuid, Unit, BoostUUIDHash>
      unit_dict = robin_hood::unordered_flat_map<boost::uuids::uuid, Unit,
                                                 BoostUUIDHash>();
  robin_hood::unordered_flat_map<boost::uuids::uuid, Item, BoostUUIDHash>
      item_dict = robin_hood::unordered_flat_map<boost::uuids::uuid, Item,
                                                 BoostUUIDHash>();
  vector<boost::uuids::uuid> sorted_unit_guids = vector<boost::uuids::uuid>();
  robin_hood::unordered_flat_map<boost::uuids::uuid, Battle, BoostUUIDHash>
      battle_dict = robin_hood::unordered_flat_map<boost::uuids::uuid, Battle,
                                                   BoostUUIDHash>();
  Pool<Ability> abilities = Pool<Ability>();
  vector<boost::uuids::uuid> item_guids_to_remove_at_end_of_frame =
      vector<boost::uuids::uuid>();
  vector<boost::uuids::uuid> battle_guids_to_remove_at_end_of_frame =
      vector<boost::uuids::uuid>();
  vector<int> ability_handles_to_release_at_end_of_frame = vector<int>();
  Tweens ability_timeout_tweens = Tweens();
  // some generic data (used by editor)
  EntityInput unit_input = EntityInput();
  EntityInput treasure_chest_input = EntityInput();
  EntityInput item_input = EntityInput();
  // ui stuff
  MovePointIcons move_point_icons;
  Text battle_move_action_point_text;
  Text battle_action_point_text;
  Text battle_action_point_description_text;
  AbilityTarget range_ability_target;
  AbilityTarget displayed_ability_target;
  AbilityTarget get_receiving_units_ability_target;
  AbilityTarget turn_order_ability_target;
  Map();
  Map(Game &game);
  Map(Game &game, int _rows, int _cols);
  void update(Game &game);
  void process_game_events(Game &game);
  void update_battle_input(Game &game, Unit &acting_unit);
  void battle_show_and_check_for_move(Game &game, Unit &acting_unit, Vec2 start,
                                      Vec2 target);
  void update_battle_ap_text(Game &game, Unit &acting_unit,
                             const Ability &ability, int ap_cost);
  void update_battle_path(Game &game, Unit &acting_unit, const Ability &ability,
                          Vec2 start, Vec2 target);
  void update_ability_selected(Game &game, Unit &acting_unit,
                               const Ability &ability);
  pair<int, Vec2> get_ap_of_move(Game &game, Unit &acting_unit, Vec2 start,
                                 Vec2 target);
  vector<boost::uuids::uuid>
  get_receiving_unit_guids(Game &game, const Ability &ability, Vec2 target_dst);
  vector<boost::uuids::uuid>
  get_units_in_aoe(Game &game, const Ability &ability, Vec2 target_dst);
  void update_non_battle_input(Game &game, Unit &acting_unit);
  void draw(Game &game);
  void create_battle(Game &game, Unit &acting_unit);
  void check_if_battle_start(Game &game, Unit &acting_unit);
  void pickup_nearby_items(Game &game, Unit &acting_unit);
  void handle_in_dialogue(Game &game, Unit &acting_unitd);
  void handle_on_unit_click(Game &game, Unit &acting_unit,
                            boost::uuids::uuid unit_guid);
  void handle_on_treasure_chest_click(Game &game, Unit &acting_unit,
                                      boost::uuids::uuid treasure_chest_guid);
  bool point_in_bounds(Vec2 &p);
  bool point_in_bounds_move_grid(Vec2 &p);
  bool unit_occupies_tile_point_move_grid(Game &game,
                                          const Rect &tile_point_hit_box,
                                          boost::uuids::uuid acting_unit_guid);
  void start_ability_timeout(Game &game, boost::uuids::uuid acting_unit_guid,
                             vector<boost::uuids::uuid> &receiving_unit_guids,
                             const Ability &ability, Vec2 target_point,
                             PerformAbilityContext _ability_contex);
  void perform_ability(Game &game, boost::uuids::uuid acting_unit_guid,
                       vector<boost::uuids::uuid> &receiving_unit_guids,
                       const Ability &ability, Vec2 target_point,
                       PerformAbilityContext _ability_context);
  void set_sorted_unit_guids();
  void erase_unit_guid(Game &game, boost::uuids::uuid _unit_guid);
  Unit &get_player_unit();
  bool is_guid_in_all_player_units(boost::uuids::uuid unit_guid);
  void add_all_player_units(Game &game);
  void remove_item_guid_at_end_of_frame(boost::uuids::uuid item_guid);
};

void map_transition(Game &game, string &file_path, Vec2 warp_to_map_tile_point);
void map_serialize(Game &game, Map &map, bool is_save_file = false);
void map_serialize_into_file(Game &game, Map &map, const char *file_path,
                             bool is_save_file = false);
Map map_deserialize_from_file(Game &game, const char *file_path,
                              bool is_save_file = false);
Map map_deserialize(Game &game, GenericObject<false, Value> &obj,
                    bool is_save_file = false);

#endif // MAP_H
