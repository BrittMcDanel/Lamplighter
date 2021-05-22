#include "tile.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void Tile::update(Game &game) { sprite.update(game); }

void Tile::draw(Game &game) { sprite.draw(game); }

void Tile::set_to_default_tile(Game &game) {
  auto image = game.engine.get_image(ImageName::Tiles);
  sprite = Sprite(
      game, image, Vec2(200, 200),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(ImageLocation(image, Rect(0, 0, 20, 20)))),
      },
      100);
}

void tile_serialize(Game &game, Tile &tile) {
  game.serializer.writer.StartObject();
  game.serializer.writer.String("sprite");
  sprite_serialize(game, tile.sprite);
  game.serializer.serialize_bool("is_obstacle", tile.is_obstacle);
  game.serializer.serialize_bool("is_warp_point", tile.is_warp_point);
  game.serializer.serialize_vec2("warps_to_tile_point",
                                 tile.warps_to_tile_point);
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Tile tile_deserialize(Game &game, GenericObject<false, Value> &obj) {
  Tile tile = Tile();
  auto sprite_obj = obj["sprite"].GetObject();
  tile.sprite = sprite_deserialize(game, sprite_obj);
  tile.is_obstacle = obj["is_obstacle"].GetBool();
  tile.is_warp_point = obj["is_warp_point"].GetBool();
  game.serializer.deserialize_vec2(obj, "warps_to_tile_point",
                                   tile.warps_to_tile_point);
  return tile;
}

void tile_serialize_into_file(Game &game, Tile &tile, const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  tile_serialize(game, tile);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "tile_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Tile tile_deserialize_from_file(Game &game, const char *file_path) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "tile_deserialize_from_file. File error " << file_path << "\n";
    abort();
  }
  stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  // cout << "buff " << buffer.str().c_str() << "\n";
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  // parse
  game.serializer.doc.Parse(buffer.str().c_str());
  // unit object
  auto obj = game.serializer.doc.GetObject();
  // deserialize unit_obj into a Unit
  return tile_deserialize(game, obj);
}