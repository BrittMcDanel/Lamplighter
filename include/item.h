#ifndef ITEM_H
#define ITEM_H
#include <SDL.h>

#include "constants.h"
#include "rapidjson/document.h"
#include "text.h"
#include "tweenable_sprite.h"
#include <boost/uuid/uuid.hpp>
#include <vector>
using namespace std;
using namespace rapidjson;

#define ITEM_MAX_QUANTITY 999
#define ITEM_MAX_COST 999999

struct Game;

struct Item {
  boost::uuids::uuid guid;
  ItemName item_name = ItemName::None;
  ItemType item_type = ItemType::None;
  bool in_use_in_pool = false;
  TweenableSprite sprite;
  string display_name;
  string description;
  int quantity = 0;
  int cost = 1;
  Text quantity_text;
  bool being_sent_to_player = false;
  Item() = default;
  Item(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_quantity_text_to_default(Game &game);
  void set_is_camera_rendered(bool _is_camera_rendered);
  void draw_at_dst(Game &game, bool _is_camera_rendered, Vec2 _dst_xy);
};

void item_serialize(Game &game, Item &item);
void item_serialize_into_file(Game &game, Item &item, const char *file_path);
Item item_deserialize_from_file(Game &game, const char *file_path,
                                bool use_static_asset_data = true);
Item item_deserialize(Game &game, GenericObject<false, Value> &obj,
                      bool use_static_asset_data = true);

#endif // ITEM_H