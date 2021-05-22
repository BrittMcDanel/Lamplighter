#ifndef UI_CROSSHAIRS_H
#define UI_CROSSHAIRS_H

#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct Crosshairs {
  Sprite top_left;
  Sprite top_right;
  Sprite bottom_left;
  Sprite bottom_right;
  bool is_hidden;
  Crosshairs() = default;
  Crosshairs(Game &game);
  void update(Game &game, Rect &dst);
  void draw(Game &game);
  void set_is_hidden(bool _is_hidden);
};

#endif // UI_CROSSHAIRS_H