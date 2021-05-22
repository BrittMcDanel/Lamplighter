#ifndef UI_SPRITE_H
#define UI_SPRITE_H

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

struct UISprite {
  bool in_use_in_pool = false;
  int pool_handle = -1;
  Image image;
  vector<SpriteSrc> srcs = vector<SpriteSrc>();
  vector<SpriteSrc> hover_srcs = vector<SpriteSrc>();
  vector<SpriteSrc> active_srcs = vector<SpriteSrc>();
  vector<SpriteSrc> disabled_srcs = vector<SpriteSrc>();
  vector<SpriteSrc> focused_srcs = vector<SpriteSrc>();
  Rect dst = Rect();
  Rect scaled_dst = Rect();
  int current_frame_idx = 0;
  Uint32 anim_speed = 1;
  Uint32 hover_anim_speed = 1;
  Uint32 active_anim_speed = 1;
  Uint32 disabled_anim_speed = 1;
  Uint32 focused_anim_speed = 1;
  Uint32 spawn_time = 0;
  InputEvents input_events = InputEvents();
  bool is_focused = false;
  bool is_hidden = false;
  UISprite() = default;
  UISprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
           Uint32 _anim_speed,
           vector<SpriteSrc> _hover_srcs = vector<SpriteSrc>(),
           Uint32 _hover_anim_speed = 1,
           vector<SpriteSrc> _disabled_srcs = vector<SpriteSrc>(),
           Uint32 _disabled_anim_speed = 1,
           vector<SpriteSrc> _active_srcs = vector<SpriteSrc>(),
           Uint32 _active_anim_speed = 1,
           vector<SpriteSrc> _focused_srcs = vector<SpriteSrc>(),
           Uint32 _focused_anim_speed = 1);
  void update(Game &game);
  void draw(Game &game);
  void set_is_hidden(bool _is_hidden);
  void set_srcs(Image _image, vector<SpriteSrc> &_srcs, int _anim_speed);
  // returns itself
  const UISprite &set_dst_xy(int x, int y);
  SpriteSrc &get_current_src(Game &game);
};

#endif // UI_SPRITE_H