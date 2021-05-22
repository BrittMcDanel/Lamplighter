#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

#include "utils.h"
#include <boost/uuid/uuid.hpp>
#include <string>

using namespace std;

enum class GameEventType {
  Invalid,
  PlayerHandleRequest,
  PlayerHandleRespond,
  CollectItemRequest,
  CollectItemRespond,
  Move,
};

struct Game;

class GameEvent {
public:
  GameEventType m_event_type;
  uint32_t m_sender_guid;
  uint32_t m_receiver_guid;
  uint32_t m_player_guid;
  boost::uuids::uuid m_unit_guid;
  boost::uuids::uuid m_item_guid;
  Vec2 m_tile_point;
  bool m_allow_units_to_path_through_each_other;

  string serialize(Game &game);
  static GameEvent deserialize(Game &game, const string &message);
  static string create_move_unit(Game &game, boost::uuids::uuid unit_guid,
                                 Vec2 tile_point,
                                 bool allow_units_to_path_through_each_other);
  static string player_handle_request(Game &game);
  static string player_handle_respond(Game &game, uint32_t receiver_guid,
                                      uint32_t player_guid);
  static string collect_item_request(Game &game, boost::uuids::uuid m_unit_guid,
                                     boost::uuids::uuid item_guid);
  static string collect_item_respond(Game &game, boost::uuids::uuid m_unit_guid,
                                     boost::uuids::uuid item_guid);
};

#endif // GAME_EVENTS_H
