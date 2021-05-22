#include "item.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include "utils_game.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Item::Item(Game &game) {
  guid = game.engine.get_guid();
  item_name = ItemName::Coin;
  auto image = game.engine.get_image(ImageName::Items);
  sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))),
                      },
                      100);
  display_name = "";
  description = "";
  cost = 1;
  quantity = 1;
  quantity_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 100,
                       TextAlignment::Right);
  being_sent_to_player = false;
}

// for deserialization
void Item::set_quantity_text_to_default(Game &game) {
  quantity_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 100,
                       TextAlignment::Right);
}

void Item::update(Game &game) {
  sprite.update(game);
  quantity_text.dst = sprite.dst;
  quantity_text.dst.x += 3;
  quantity_text.dst.y += 10;
  quantity_text.max_width = sprite.dst.w;
  quantity_text.str = to_string(quantity);
  quantity_text.is_hidden = quantity <= 1;
  quantity_text.update(game);
}

void Item::draw(Game &game) {
  sprite.draw(game);
  quantity_text.draw(game);
}

void Item::set_is_camera_rendered(bool _is_camera_rendered) {
  sprite.is_camera_rendered = _is_camera_rendered;
  quantity_text.is_camera_rendered = _is_camera_rendered;
}

void Item::draw_at_dst(Game &game, bool _is_camera_rendered, Vec2 _dst_xy) {
  // don't draw ItemName of none
  if (item_name == ItemName::None) {
    return;
  }
  auto _quantity_text_dst = _dst_xy;
  _quantity_text_dst.x += 3;
  _quantity_text_dst.y += 10;
  auto _quantity_text_str = to_string(quantity);
  draw_sprite_at_dst(game, sprite.image, sprite.srcs, sprite.spawn_time,
                     sprite.anim_speed, false, _dst_xy);
  if (quantity > 1) {
    auto _quantity_text_scaled_max_width = ITEM_DIM * game.engine.scale;
    draw_text_at_dst(game, quantity_text.font_handle, _quantity_text_str,
                     quantity_text.alignment, _quantity_text_scaled_max_width,
                     false, _quantity_text_dst);
  }
}

void item_serialize(Game &game, Item &item) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_string_val("guid", to_string(item.guid));
  game.serializer.serialize_int("item_name", static_cast<int>(item.item_name));
  game.serializer.serialize_int("item_type", static_cast<int>(item.item_type));
  game.serializer.serialize_string("display_name", item.display_name);
  game.serializer.serialize_string("description", item.description);
  game.serializer.serialize_int("cost", item.cost);
  game.serializer.serialize_int("quantity", item.quantity);
  game.serializer.writer.String("sprite");
  tweenable_sprite_serialize(game, item.sprite);
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Item item_deserialize(Game &game, GenericObject<false, Value> &obj,
                      bool use_static_asset_data) {
  Item item = Item(game);
  item.item_name = static_cast<ItemName>(obj["item_name"].GetInt());
  auto sprite_obj = obj["sprite"].GetObject();
  auto sprite = tweenable_sprite_deserialize(game, sprite_obj);

  if (use_static_asset_data) {
    auto &static_item = game.assets.get_item(item.item_name);
    item = static_item;
  } else {
    item.item_type = static_cast<ItemType>(obj["item_type"].GetInt());
    item.display_name = obj["display_name"].GetString();
    item.description = obj["description"].GetString();
    item.cost = obj["cost"].GetInt();
    // need the whole sprite here
    item.sprite = sprite;
  }
  // things that differ among item instantiations, all other data is static
  item.guid = game.engine.string_gen(obj["guid"].GetString());
  item.quantity = obj["quantity"].GetInt();
  item.set_quantity_text_to_default(game);
  item.sprite.dst = sprite.dst;
  item.sprite.tweens = sprite.tweens;
  return item;
}

void item_serialize_into_file(Game &game, Item &item, const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  item_serialize(game, item);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "item_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Item item_deserialize_from_file(Game &game, const char *file_path,
                                bool use_static_asset_data) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "item_deserialize_from_file. File error " << file_path << "\n";
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
  // item object
  auto obj = game.serializer.doc.GetObject();
  // deserialize item_obj into a Unit
  auto item = item_deserialize(game, obj, use_static_asset_data);
  // set a unique guid as this is a copy of item from the json
  // file and will have the guid in the json file.
  item.guid = game.engine.get_guid();
  return item;
}