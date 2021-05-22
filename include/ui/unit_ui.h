#ifndef UI_UNITUI_H
#define UI_UNITUI_H

#include "dialogue_box.h"
#include "sprite.h"
#include "ui/battle_text.h"
#include "ui/crosshairs.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Unit;

// drawn before units
struct UnitUIBeforeUnit {
  Sprite move_icon;
  UnitUIBeforeUnit() = default;
  UnitUIBeforeUnit(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game);
};

// drawn after units
struct UnitUIAfterUnit {
  DialogueBox dialogue_box;
  DialogueBox ability_box;
  Crosshairs crosshairs;
  Sprite active_arrow;
  Sprite turn_order_arrow;
  UnitUIAfterUnit() = default;
  UnitUIAfterUnit(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game);
};

struct UnitUILastLayer {
  BattleTexts battle_texts;
  UnitUILastLayer() = default;
  UnitUILastLayer(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game);
};

#endif // UI_UNITUI_H