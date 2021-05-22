#ifndef UI_TURN_ORDER_DISPLAY_H
#define UI_TURN_ORDER_DISPLAY_H

#include "fixed_sprite.h"
#include "ui/ui_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Battle;

struct TurnOrderDisplay {
  vector<UISprite> slots = vector<UISprite>();
  vector<UISprite> charge_slots = vector<UISprite>();
  vector<FixedSprite> hp_bars = vector<FixedSprite>();
  TurnOrderDisplay() = default;
  TurnOrderDisplay(Game &game);
  void update(Game &game, Battle &battle);
  void draw(Game &game, Battle &battle);
  void update_layout(Game &game, Battle &battle, bool _draw);
};

#endif // UI_TURN_ORDER_DISPLAY_H