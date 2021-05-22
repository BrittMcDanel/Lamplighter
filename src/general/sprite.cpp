#include "sprite.h"
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

Sprite::Sprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
               Uint32 _anim_speed) {
  image = _image;
  dst = Rect(_dst.x, _dst.y, 0, 0);
  scaled_dst = Rect(_dst.x, _dst.y, 0, 0);
  srcs = _srcs;
  anim_speed = _anim_speed;
  spawn_time = game.engine.current_time;
}

void Sprite::update(Game &game) {
  // an empty sprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  current_frame_idx = get_current_frame_idx(
      game.engine.current_time, spawn_time, srcs.size(), anim_speed);
  auto &current_src = srcs[current_frame_idx];
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  scaled_dst = dst;
  set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                              game.engine.scale);
  current_src.update(scaled_dst);
}

void Sprite::draw(Game &game) {
  // an empty sprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  if (is_hidden) {
    return;
  }
  // super super important to not push to the render queue things that don't
  // need to be rendered. Can reduce performance by a lot if you don't do this.
  // 80 by 80 tile map goes from 4500 fps to 2200 fps on -O3.
  // a sprite dst of 0, 0 is equal to the  game_rect.x, game_rect.y (bottom-left
  // corner)
  if (scaled_dst.x < -scaled_dst.w || scaled_dst.x > game.engine.game_rect.w ||
      scaled_dst.y < -scaled_dst.h || scaled_dst.y > game.engine.game_rect.h) {
    return;
  }
  game.engine.set_active_image(image);
  auto &src = srcs[current_frame_idx];
  game.engine.push_to_render_buffer(src.vertices, 12, src.uvs, 12);
}

void Sprite::set_is_hidden(bool _is_hidden) { is_hidden = _is_hidden; }

void sprite_serialize(Game &game, Sprite &sprite) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_rect("dst", sprite.dst);
  game.serializer.serialize_int("image_name", (int)sprite.image.image_name);
  game.serializer.serialize_int("anim_speed", sprite.anim_speed);
  game.serializer.serialize_sprite_src_vec("srcs", sprite.srcs);
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Sprite sprite_deserialize(Game &game, GenericObject<false, Value> &obj) {
  Sprite sprite = Sprite();
  auto image_name = (ImageName)obj["image_name"].GetInt();
  if (image_name == ImageName::None) {
    sprite.image = Image();
  } else {
    sprite.image = game.engine.get_image((ImageName)obj["image_name"].GetInt());
  }
  game.serializer.deserialize_rect(obj, "dst", sprite.dst);
  sprite.anim_speed = obj["anim_speed"].GetInt();
  game.serializer.deserialize_sprite_src_vec(obj, sprite.image, "srcs",
                                             sprite.srcs);
  return sprite;
}

void sprite_serialize_into_file(Game &game, Sprite &sprite,
                                const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  sprite_serialize(game, sprite);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "sprite_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Sprite sprite_deserialize_from_file(Game &game, const char *file_path) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "sprite_deserialize_from_file. File error " << file_path << "\n";
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
  return sprite_deserialize(game, obj);
}