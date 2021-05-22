#ifndef TWEENABLESPRITE_H
#define TWEENABLESPRITE_H

#include "engine.h"
#include "input_events.h"
#include "rapidjson/document.h"
#include "tween.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct TweenableSprite {
  Image image;
  vector<SpriteSrc> srcs = vector<SpriteSrc>();
  Rect dst = Rect();
  Rect scaled_dst = Rect();
  int current_frame_idx = 0;
  Uint32 anim_speed = 1;
  Uint32 spawn_time = 0;
  InputEvents input_events = InputEvents();
  Tweens tweens = Tweens();
  bool is_camera_rendered = true;
  bool is_hidden = false;
  TweenableSprite() = default;
  TweenableSprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
                  Uint32 anim_speed);
  void update(Game &game);
  void draw(Game &game);
  void set_is_hidden(bool _is_hidden);
};

void tweenable_sprite_serialize(Game &game, TweenableSprite &sprite);
void tweenable_sprite_serialize_into_file(Game &game, TweenableSprite &sprite,
                                          const char *file_path);
TweenableSprite tweenable_sprite_deserialize_from_file(Game &game,
                                                       const char *file_path);
TweenableSprite tweenable_sprite_deserialize(Game &game,
                                             GenericObject<false, Value> &obj);

#endif // TWEENABLESPRITE_H