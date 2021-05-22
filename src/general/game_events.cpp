#include "game_events.h"
#include "game.h"

#include <fmt/format.h>
using namespace rapidjson;

string GameEvent::serialize(Game &game) {
  game.serializer.clear();
  game.serializer.writer.StartObject();
  game.serializer.serialize_int("type", (int)m_event_type);
  game.serializer.serialize_string_val("guid", to_string(game.player.guid));
  switch (m_event_type) {
  case GameEventType::Move: {
    game.serializer.serialize_string_val("unit_guid", to_string(m_unit_guid));
    game.serializer.serialize_int("x", m_tile_point.x);
    game.serializer.serialize_int("y", m_tile_point.y);
    game.serializer.serialize_bool("allow_units_to_path_through_each_other",
                                   m_allow_units_to_path_through_each_other);
    break;
  }
  case GameEventType::PlayerHandleRequest: {
    break;
  }
  case GameEventType::PlayerHandleRespond: {
    game.serializer.serialize_string_val("receiver_guid",
                                         to_string(m_receiver_guid));
    game.serializer.serialize_string_val("player_guid",
                                         to_string(m_player_guid));
    break;
  }
  case GameEventType::CollectItemRequest: {
    game.serializer.serialize_string_val("unit_guid", to_string(m_unit_guid));
    game.serializer.serialize_string_val("item_guid", to_string(m_item_guid));
    break;
  }
  case GameEventType::CollectItemRespond: {
    game.serializer.serialize_string_val("unit_guid", to_string(m_unit_guid));
    game.serializer.serialize_string_val("item_guid", to_string(m_item_guid));
    break;
  }
  default: {
    fmt::print("GameEvent::serialize: Invalid m_event_type: {}", m_event_type);
    abort();
  }
  }

  game.serializer.writer.EndObject();
  return string(game.serializer.sb.GetString());
}

GameEvent GameEvent::deserialize(Game &game, const string &message) {
  GameEvent event;
  game.serializer.clear();
  game.serializer.doc.Parse(message.c_str());
  auto obj = game.serializer.doc.GetObject();
  event.m_event_type = (GameEventType)obj["type"].GetInt();
  event.m_sender_guid = obj["guid"].GetUint();
  switch (event.m_event_type) {
  case GameEventType::Move: {
    event.m_unit_guid = game.engine.string_gen(obj["unit_guid"].GetString());
    event.m_tile_point.x = obj["x"].GetInt();
    event.m_tile_point.y = obj["y"].GetInt();
    event.m_allow_units_to_path_through_each_other =
        obj["allow_units_to_path_through_each_other"].GetBool();
    break;
  }
  case GameEventType::PlayerHandleRequest: {
    break;
  }
  case GameEventType::PlayerHandleRespond: {
    event.m_player_guid = obj["player_guid"].GetUint();
    event.m_receiver_guid = obj["receiver_guid"].GetUint();
    break;
  }
  case GameEventType::CollectItemRequest: {
    event.m_unit_guid = game.engine.string_gen(obj["unit_guid"].GetString());
    event.m_item_guid = game.engine.string_gen(obj["item_guid"].GetString());
    break;
  }
  case GameEventType::CollectItemRespond: {
    event.m_unit_guid = game.engine.string_gen(obj["unit_guid"].GetString());
    event.m_item_guid = game.engine.string_gen(obj["item_guid"].GetString());
    break;
  }
  default: {
    fmt::print("GameEvent::deserialize: Invalid m_event_type: {}",
               event.m_event_type);
    abort();
  }
  }
  return event;
}

string
GameEvent::create_move_unit(Game &game, boost::uuids::uuid unit_guid,
                            Vec2 tile_point,

                            bool allow_units_to_path_through_each_other) {
  GameEvent event;
  event.m_event_type = GameEventType::Move;
  event.m_sender_guid = game.player.guid;
  event.m_unit_guid = unit_guid;
  event.m_tile_point = tile_point;
  event.m_allow_units_to_path_through_each_other =
      allow_units_to_path_through_each_other;
  return event.serialize(game);
}

string GameEvent::player_handle_request(Game &game) {
  GameEvent event;
  event.m_event_type = GameEventType::PlayerHandleRequest;
  event.m_sender_guid = game.player.guid;
  return event.serialize(game);
}

string GameEvent::player_handle_respond(Game &game, uint32_t receiver_guid,
                                        uint32_t player_guid) {
  GameEvent event;
  event.m_event_type = GameEventType::PlayerHandleRespond;
  event.m_sender_guid = game.player.guid;
  event.m_receiver_guid = receiver_guid;
  event.m_player_guid = player_guid;
  return event.serialize(game);
}

string GameEvent::collect_item_request(Game &game, boost::uuids::uuid unit_guid,
                                       boost::uuids::uuid item_guid) {
  GameEvent event;
  event.m_event_type = GameEventType::CollectItemRequest;
  event.m_sender_guid = game.player.guid;
  event.m_unit_guid = unit_guid;
  event.m_item_guid = item_guid;
  return event.serialize(game);
}

string GameEvent::collect_item_respond(Game &game, boost::uuids::uuid unit_guid,
                                       boost::uuids::uuid item_guid) {
  GameEvent event;
  event.m_event_type = GameEventType::CollectItemRespond;
  event.m_sender_guid = game.player.guid;
  event.m_unit_guid = unit_guid;
  event.m_item_guid = item_guid;
  return event.serialize(game);
}
