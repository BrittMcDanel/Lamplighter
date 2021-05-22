#ifndef TILE_H
#define TILE_H
#include "rapidjson/document.h"
#include "sprite.h"
#include "utils.h"
using namespace rapidjson;

struct Game;

struct Tile {
  Sprite sprite = Sprite();
  Vec2 warps_to_tile_point = Vec2(0, 0);
  bool is_obstacle = false;
  bool is_warp_point = false;
  void update(Game &game);
  void draw(Game &game);
  void set_to_default_tile(Game &game);
};

void tile_serialize(Game &game, Tile &tile);
void tile_serialize_into_file(Game &game, Tile &tile, const char *file_path);
Tile tile_deserialize_from_file(Game &game, const char *file_path);
Tile tile_deserialize(Game &game, GenericObject<false, Value> &unit_obj);

#endif // TILE_H
