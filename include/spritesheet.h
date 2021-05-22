#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "utils.h"
#include "engine.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

// only used by the editor
struct SpriteSheet {
  Image image;
  vector<SpriteSrc> srcs = vector<SpriteSrc>();
  Rect dst = Rect();
  Rect scaled_dst = Rect();
  int current_frame_idx = 0;
  Uint32 anim_speed = 1;
  Uint32 spawn_time = 0;
  bool is_hidden = false;
  SpriteSheet();
  SpriteSheet(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs, Uint32 anim_speed);
  void update(Game &game);
  void draw(Game &game);
  int get_current_frame_idx(Game &game, int size);
  void set_is_hidden(bool _is_hidden);
};

#endif // SPRITESHEET_H