#ifndef UI_EXPANDABLE_SPRITE_H
#define UI_EXPANDABLE_SPRITE_H

#include "fixed_sprite.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "tweenable_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct ExpandableSprite {
  vector<TweenableSprite> corners;
  vector<FixedSprite> edges;
  FixedSprite center;
  int corner_size;
  bool is_hidden = false;
  ExpandableSprite() = default;
  ExpandableSprite(Game &game);
  ExpandableSprite(Game &game, int _corner_size,
                   vector<TweenableSprite> &_corners,
                   vector<FixedSprite> &_edges, FixedSprite &_center);
  void update(Game &game);
  void draw(Game &game);
  void update_layout(Game &game, Rect r);
  void set_is_camera_rendered(bool _is_camera_rendered);
};

#endif // UI_EXPANDABLE_SPRITE_H