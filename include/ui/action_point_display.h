#ifndef UI_ACTION_POINT_DISPLAY_H
#define UI_ACTION_POINT_DISPLAY_H

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

struct ActionPointDisplay {
  int active = 0;
  vector<UISprite> available_icons = vector<UISprite>();
  vector<UISprite> active_icons = vector<UISprite>();
  vector<UISprite> inactive_icons = vector<UISprite>();
  ButtonText end_turn_button = ButtonText();
  bool is_hidden = true;
  ActionPointDisplay() = default;
  ActionPointDisplay(Game &game);
  void update(Game &game, int available, int total);
  void draw(Game &game);
  void hide_all();
};

#endif // UI_ACTION_POINT_DISPLAY_H