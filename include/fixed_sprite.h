#ifndef FIXEDSPRITE_H
#define FIXEDSPRITE_H

#include "engine.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct FixedSprite {
  Image image;
  vector<SpriteSrc> srcs = vector<SpriteSrc>();
  Rect dst = Rect();
  Rect scaled_dst = Rect();
  int current_frame_idx = 0;
  Uint32 anim_speed = 1;
  Uint32 spawn_time = 0;
  bool is_camera_rendered = true;
  bool is_hidden = false;
  FixedSprite();
  FixedSprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
              Uint32 anim_speed);
  void update(Game &game);
  void draw(Game &game);
  void set_is_hidden(bool _is_hidden);
};

#endif // FIXEDSPRITE_H