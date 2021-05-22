#ifndef SPRITE_H
#define SPRITE_H

#include "engine.h"
#include "rapidjson/document.h"
#include "tween.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct Sprite {
  Image image;
  vector<SpriteSrc> srcs = vector<SpriteSrc>();
  Rect dst = Rect();
  Rect scaled_dst = Rect();
  int current_frame_idx = 0;
  Uint32 anim_speed = 1;
  Uint32 spawn_time = 0;
  bool is_hidden = false;
  Sprite() = default;
  Sprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
         Uint32 anim_speed);
  void update(Game &game);
  void draw(Game &game);
  void set_is_hidden(bool _is_hidden);
};

void sprite_serialize(Game &game, Sprite &sprite);
void sprite_serialize_into_file(Game &game, Sprite &sprite,
                                const char *file_path);
Sprite sprite_deserialize_from_file(Game &game, const char *file_path);
Sprite sprite_deserialize(Game &game, GenericObject<false, Value> &obj);

#endif // SPRITE_H