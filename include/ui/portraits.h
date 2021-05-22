#ifndef UI_PORTRAITS_H
#define UI_PORTRAITS_H

#include "engine.h"
#include "fixed_sprite.h"
#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "ui/ui_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Unit;

struct Portrait {
  UISprite background;
  FixedSprite hp_bar;
  vector<UISprite> status_effect_slots;
  Portrait() = default;
  Portrait(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
};

struct Portraits {
  vector<Portrait> portraits;
  Portraits() = default;
  Portraits(Game &game);
  void update(Game &game);
  void draw(Game &game);
};

#endif // UI_PORTRAITS_H