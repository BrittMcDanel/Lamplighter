#ifndef TREASURECHEST_H
#define TREASURECHEST_H
#include <SDL.h>

#include "constants.h"
#include "inventory.h"
#include "item.h"
#include "rapidjson/document.h"
#include "tweenable_sprite.h"
#include <boost/uuid/uuid.hpp>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct TreasureChest {
  boost::uuids::uuid guid;
  TreasureChestName treasure_chest_name = TreasureChestName::None;
  Vec2 tile_point = Vec2(0, 0);
  TweenableSprite sprite;
  // the sprite when the treasure chest is opened
  TweenableSprite opened_sprite;
  Inventory inventory = Inventory();
  bool is_opened = false;
  TreasureChest() = default;
  TreasureChest(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_tile_point(Game &game, Vec2 _tile_point);
};

void treasure_chest_serialize(Game &game, TreasureChest &treasure_chest);
void treasure_chest_serialize_into_file(Game &game,
                                        TreasureChest &treasure_chest,
                                        const char *file_path);
TreasureChest
treasure_chest_deserialize_from_file(Game &game, const char *file_path,
                                     bool use_static_asset_data = true);
TreasureChest treasure_chest_deserialize(Game &game,
                                         GenericObject<false, Value> &obj,
                                         bool use_static_asset_data = true);

#endif // TREASURECHEST_H