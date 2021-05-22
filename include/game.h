#ifndef GAME_H
#define GAME_H

#include "ability_targets.h"
#include "assets.h"
#include "constants.h"
#include "engine.h"
#include "map.h"
#include "network.h"
#include "pathfinder.h"
#include "serializer.h"
#include "text.h"
#include "ui/ui.h"
#include <string>
#include <vector>
#define INVALID_HANDLE 1024
using namespace std;

struct EditorState {
  bool use_editor = false;
  bool in_play_mode = false;
  bool no_editor_or_editor_and_in_play_mode();
};

struct Player {
  uint32_t handle;
  uint32_t guid;
  bool is_host;
  Player() = default;
  Player(const GameClient &game_client, bool _is_host)
      : is_host(_is_host), guid(game_client.GetId()), handle(INVALID_HANDLE) {}
};

struct MapTransitionRequest {
  bool transition_requested = false;
  string transition_to_map_file = "";
  Vec2 transition_to_map_tile_point = Vec2(0, 0);
  MapTransitionRequest() = default;
  void set_transition(string &_transition_to_map_file,
                      Vec2 _transition_to_map_tile_point);
  void clear();
};

struct Game {
  EditorState editor_state = EditorState();
  MapTransitionRequest map_transition_request = MapTransitionRequest();
  Engine engine = Engine();
  Assets assets = Assets();
  Map map = Map();
  PathFinder path_finder = PathFinder();
  TextRenderer text_renderer = TextRenderer();
  AbilityTargets ability_targets = AbilityTargets();
  UI ui = UI();
  Serializer serializer = Serializer();
  vector<bool> game_flags = vector<bool>();
  GameServer game_server;
  GameClient game_client;
  Player player;
  int num_players;
  void start(std::string server, bool is_host);
  void process_game_events();
  void update();
  void draw();
  void stop();
  void create_player_handle(uint32_t guid);
  bool is_game_flag_set(GameFlag flag);
  vector<string> get_game_flags_as_strings();
  vector<string> get_item_names_as_strings();
  vector<string> get_ability_names_as_strings();
  vector<string> get_unit_names_as_strings();
  vector<string> get_treasure_chest_names_as_strings();
  vector<string> get_status_effect_names_as_strings();
};

#endif // GAME_H