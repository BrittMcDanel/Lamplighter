#ifndef UI_ALERT_H
#define UI_ALERT_H

#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "ui/expandable_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct Alert {
  Rect dst;
  ExpandableSprite background;
  Text text;
  Tweens tweens;
  bool is_hidden = true;
  Alert() = default;
  Alert(Game &game, const char *_text_str, Vec2 _dst);
  void update(Game &game);
  void draw(Game &game);
  void show(Game &game);
};

#endif // UI_ALERT_H