#include "treasure_chest.h"
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

TreasureChest::TreasureChest(Game &game) {
  guid = game.engine.get_guid();
  tile_point = Vec2(0, 0);
  auto image = game.engine.get_image(ImageName::TreasureChests);
  sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))),
                      },
                      100);
  opened_sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(20, 0, 20, 20))),
                      },
                      100);
  is_opened = false;
}

void TreasureChest::update(Game &game) {
  // always update both even though you only draw one or the other.
  // when the is_opened changes the old sprite can have an input event
  // stuck in a state which can make things difficult. This is easier
  // to handle.
  sprite.update(game);
  opened_sprite.update(game);
}

void TreasureChest::draw(Game &game) {
  if (!is_opened) {
    sprite.draw(game);
  } else {
    opened_sprite.draw(game);
  }
}

void TreasureChest::set_tile_point(Game &game, Vec2 _tile_point) {
  tile_point = _tile_point;
  set_world_point_from_tile_point(_tile_point, sprite.dst);
  set_world_point_from_tile_point(_tile_point, opened_sprite.dst);
}

void treasure_chest_serialize(Game &game, TreasureChest &treasure_chest) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_string_val("guid", to_string(treasure_chest.guid));
  game.serializer.serialize_int(
      "treasure_chest_name",
      static_cast<int>(treasure_chest.treasure_chest_name));
  game.serializer.serialize_vec2("tile_point", treasure_chest.tile_point);
  game.serializer.serialize_bool("is_opened", treasure_chest.is_opened);
  game.serializer.writer.String("sprite");
  tweenable_sprite_serialize(game, treasure_chest.sprite);
  game.serializer.writer.String("opened_sprite");
  tweenable_sprite_serialize(game, treasure_chest.opened_sprite);
  game.serializer.writer.String("inventory");
  game.serializer.writer.StartArray();
  for (auto &item : treasure_chest.inventory.items) {
    item_serialize(game, item);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

TreasureChest treasure_chest_deserialize(Game &game,
                                         GenericObject<false, Value> &obj,
                                         bool use_static_asset_data) {
  TreasureChest treasure_chest = TreasureChest(game);
  // treasure_chest.guid = game.engine.string_gen(obj["guid"].GetString());
  game.serializer.deserialize_vec2(obj, "tile_point",
                                   treasure_chest.tile_point);
  treasure_chest.treasure_chest_name =
      static_cast<TreasureChestName>(obj["treasure_chest_name"].GetInt());
  treasure_chest.is_opened = obj["is_opened"].GetBool();
  auto sprite_obj = obj["sprite"].GetObject();
  auto opened_sprite_obj = obj["opened_sprite"].GetObject();
  treasure_chest.sprite = tweenable_sprite_deserialize(game, sprite_obj);
  treasure_chest.opened_sprite =
      tweenable_sprite_deserialize(game, opened_sprite_obj);
  auto items_array = obj["inventory"].GetArray();
  for (auto &item_obj : items_array) {
    auto obj = item_obj.GetObject();
    auto item = item_deserialize(game, obj);
    treasure_chest.inventory.add_item(item);
  }
  return treasure_chest;
}

void treasure_chest_serialize_into_file(Game &game,
                                        TreasureChest &treasure_chest,
                                        const char *file_path) {
  // clear as this is for individual treasure chests, not part of a nested
  // struct (like map)
  game.serializer.clear();
  treasure_chest_serialize(game, treasure_chest);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "treasure_chest_serialize_into_file. File error " << file_path
         << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

TreasureChest treasure_chest_deserialize_from_file(Game &game,
                                                   const char *file_path,
                                                   bool use_static_asset_data) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "treasure_chest_deserialize_from_file. File error " << file_path
         << "\n";
    abort();
  }
  stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  // cout << "buff " << buffer.str().c_str() << "\n";
  // clear as this is for individual treasure chests, not part of a nested
  // struct (like map)
  game.serializer.clear();
  // parse
  game.serializer.doc.Parse(buffer.str().c_str());
  // treasure chest object
  auto obj = game.serializer.doc.GetObject();
  // deserialize obj into a TreasureChest
  auto treasure_chest =
      treasure_chest_deserialize(game, obj, use_static_asset_data);
  // set a unique guid as this is a copy of the treasure chest from the json
  // file
  // and will have the guid in the json file.
  treasure_chest.guid = game.engine.get_guid();
  return treasure_chest;
}