#include "game.h"
#include "game_events.h"
#include <assert.h>
#include <fmt/format.h>

void MapTransitionRequest::set_transition(string &_transition_to_map_file,
                                          Vec2 _transition_to_map_tile_point) {
  transition_requested = true;
  transition_to_map_file = _transition_to_map_file;
  transition_to_map_tile_point = _transition_to_map_tile_point;
}

void MapTransitionRequest::clear() { transition_requested = false; }

bool EditorState::no_editor_or_editor_and_in_play_mode() {
  if (!use_editor) {
    return true;
  } else {
    return in_play_mode;
  }
}

void Game::start(std::string server, bool _is_host) {
  engine.start();
  // populate game flags vec with all false before loading into it
  for (size_t i = 0; i < static_cast<int>(GameFlag::Last); i++) {
    // treat GameFlag::None as truthy or as always being set. Useful for
    // lists that look for the first set flag.
    if (i == 0) {
      game_flags.push_back(true);
    } else {
      game_flags.push_back(false);
    }
  }
  assets = Assets();
  assets.start(*this);
  ability_targets = AbilityTargets(*this);
  ui = UI(*this);
  map = Map(*this, 40, 40);

  // Networking stuff
  std::string ip = server.substr(0, server.find(':'));
  uint16 port = (uint16)(std::stoi(
      server.substr(server.find(':') + 1, std::string::npos)));
  SteamNetworkingIPAddr addrServer;
  addrServer.Clear();

  if (!addrServer.ParseString(server.c_str())) {
    std::cout << "Invalid server address: " << server << std::endl;
    return;
  }

  InitSteamDatagramConnectionSockets();
  if (_is_host) {
    game_server.Start(port);
  }
  game_client.Start(addrServer);
  player = Player(game_client, _is_host);

  if (_is_host) {
    player.handle = 0;
    num_players = 1;
  } else {
    game_client.SendMessage(GameEvent::player_handle_request(*this));
  }
}

void Game::process_game_events() {
  for (const auto &message : game_client.GetMessages()) {
    auto event = GameEvent::deserialize(*this, message);
    switch (event.m_event_type) {
    case GameEventType::Move:
    case GameEventType::CollectItemRequest: // Sorry bruv - this was too easy
    case GameEventType::CollectItemRespond: // ~falling through oh yea~
      map.game_events.push(event);
      break;
    case GameEventType::PlayerHandleRespond: {
      if (player.guid == event.m_receiver_guid) {
        player.handle = event.m_player_guid;
        fmt::print("Got PlayerHandleRespond for guid {}: {}\n",
                   event.m_receiver_guid, event.m_player_guid);
      }
      break;
    }
    case GameEventType::PlayerHandleRequest: {
      if (player.is_host) {
        create_player_handle(event.m_sender_guid);
      }
      break;
    }
    default: {
      fmt::print("Game::process_game_event: Invalid m_event_type: {}\n",
                 event.m_event_type);
      abort();
    }
    }
  }
}

void Game::update() {
  game_server.Update();
  process_game_events();
  // every frame the cursor to set at the end of the frame is the default cursor
  // intiially
  engine.set_cursor(CursorType::Default);
  map.update(*this);
  // a map transition has been requested, transition to the new map.
  // doing this after map.update because if done immediately (from map.update
  // tween callback) everything will be invalidated as it is a new map now.
  if (map_transition_request.transition_requested) {
    map_transition(*this, map_transition_request.transition_to_map_file,
                   map_transition_request.transition_to_map_tile_point);
    // update the new map so it can draw this frame
    map.update(*this);
    map_transition_request.clear();
  }
  ui.update(*this);
  // set the cursor to whatever was set last using engine.set_cursor
  engine.change_cursor_to_end_of_frame_cursor();
  game_client.Update();
}

void Game::draw() {
  map.draw(*this);
  ui.draw(*this);
}

void Game::stop() {
  game_client.Stop();
  game_server.Stop();
  ShutdownSteamDatagramConnectionSockets();
}

bool Game::is_game_flag_set(GameFlag flag) {
  auto idx = static_cast<int>(flag);
  assert(idx >= 0 && idx < static_cast<int>(GameFlag::Last));
  return game_flags[idx];
}

// TODO: our current solution to handling multiple players:
// 1. Games always start in a Lobby
// 2. When a new player joins they are given the next available
//    handle via the create_player_handle method. The host is
//    defaulted to handle 0.
// 3. We will implement a swap_player_handles method which swaps
//    two of the player handles (which can be performed by the host).
// 4. On a disconnect in game, we quick save and go the lobby.
void Game::create_player_handle(uint32_t receiver_guid) {
  int player_handle = num_players;
  num_players++;
  game_client.SendMessage(
      GameEvent::player_handle_respond(*this, receiver_guid, player_handle));
}

vector<string> Game::get_game_flags_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)GameFlag::Always; i < (int)GameFlag::Last; i++) {
    auto game_flag = (GameFlag)i;
    switch (game_flag) {
    case GameFlag::Always: {
      strs.push_back("Always");
      break;
    }
    case GameFlag::TalkedToBob: {
      strs.push_back("Talked to Bob");
      break;
    }
    default: {
      cout << "Game::get_game_flags_as_strings. GameFlag not handled " << i
           << "\n";
      abort();
    }
    }
  }
  return strs;
}
vector<string> Game::get_item_names_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)ItemName::None; i < (int)ItemName::Last; i++) {
    auto item_name = (ItemName)i;
    switch (item_name) {
    case ItemName::None: {
      strs.push_back("None");
      break;
    }
    case ItemName::Coin: {
      strs.push_back("Coin");
      break;
    }
    case ItemName::Cash: {
      strs.push_back("Cash");
      break;
    }
    case ItemName::SlimeJelly: {
      strs.push_back("Slime Jelly");
      break;
    }
    case ItemName::FurPelt: {
      strs.push_back("Fur Pelt");
      break;
    }
    case ItemName::Stump: {
      strs.push_back("Stump");
      break;
    }
    case ItemName::Mushroom: {
      strs.push_back("Mushroom");
      break;
    }
    case ItemName::BatWing: {
      strs.push_back("Bat Wing");
      break;
    }
    case ItemName::Apple: {
      strs.push_back("Apple");
      break;
    }
    case ItemName::IronArmor: {
      strs.push_back("Iron Armor");
      break;
    }
    case ItemName::IronHelmet: {
      strs.push_back("Iron Helmet");
      break;
    }
    case ItemName::IronSword: {
      strs.push_back("Iron Sword");
      break;
    }
    default: {
      cout << "Game::get_item_names_as_strings. ItemName not handled " << i
           << "\n";
      abort();
    }
    }
  }
  return strs;
}

vector<string> Game::get_ability_names_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)AbilityName::None; i < (int)AbilityName::Last; i++) {
    auto ability_name = (AbilityName)i;
    switch (ability_name) {
    case AbilityName::None: {
      strs.push_back("None");
      break;
    }
    case AbilityName::Fire: {
      strs.push_back("Fire");
      break;
    }
    case AbilityName::FireBall: {
      strs.push_back("Fire Ball");
      break;
    }
    case AbilityName::SwordSlash: {
      strs.push_back("Sword Slash");
      break;
    }
    case AbilityName::CounterAttack: {
      strs.push_back("Counter Attack");
      break;
    }
    case AbilityName::Bullet: {
      strs.push_back("Bullet");
      break;
    }
    case AbilityName::Resolve: {
      strs.push_back("Resolve");
      break;
    }
    default: {
      cout << "Game::get_ability_names_as_strings. AbilityName not handled "
           << i << "\n";
      abort();
    }
    }
  }
  return strs;
}

vector<string> Game::get_unit_names_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)UnitName::None; i < (int)UnitName::Last; i++) {
    auto unit_name = (UnitName)i;
    switch (unit_name) {
    case UnitName::None: {
      strs.push_back("None");
      break;
    }
    case UnitName::Ally: {
      strs.push_back("Ally");
      break;
    }
    case UnitName::Enemy: {
      strs.push_back("Enemy");
      break;
    }
    default: {
      cout << "Game::get_unit_names_as_strings. UnitName not handled " << i
           << "\n";
      abort();
    }
    }
  }
  return strs;
}

vector<string> Game::get_treasure_chest_names_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)TreasureChestName::None; i < (int)TreasureChestName::Last;
       i++) {
    auto unit_name = (TreasureChestName)i;
    switch (unit_name) {
    case TreasureChestName::None: {
      strs.push_back("None");
      break;
    }
    case TreasureChestName::Small: {
      strs.push_back("Small");
      break;
    }
    default: {
      cout << "Game::get_treasure_chest_names_as_strings. TreasureChestName "
              "not handled "
           << i << "\n";
      abort();
    }
    }
  }
  return strs;
}

vector<string> Game::get_status_effect_names_as_strings() {
  auto strs = vector<string>();
  for (int i = (int)StatusEffectName::None; i < (int)StatusEffectName::Last;
       i++) {
    auto status_effect_name = (StatusEffectName)i;
    switch (status_effect_name) {
    case StatusEffectName::None: {
      strs.push_back("None");
      break;
    }
    case StatusEffectName::Resolve: {
      strs.push_back("Resolve");
      break;
    }
    default: {
      cout << "Game::get_status_effect_names_as_strings. StatusEffectName "
              "not handled "
           << i << "\n";
      abort();
    }
    }
  }
  return strs;
}