#ifndef UI_BOTTOMNAVBAR_H
#define UI_BOTTOMNAVBAR_H

#include "rapidjson/document.h"
#include "text.h"
#include "tween.h"
#include "ui/ui_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;
struct Unit;

struct BottomNavBar {
  UISprite background = UISprite();
  UISprite inventory_button_slot = UISprite();
  UISprite inventory_button_slot_icon = UISprite();
  ButtonIcon skill_tree_button = ButtonIcon();
  vector<vector<UISprite>> ability_slots = vector<vector<UISprite>>();
  UISprite ability_up_arrow = UISprite();
  UISprite ability_down_arrow = UISprite();
  Text ability_row_text = Text();
  int showing_ability_row_idx = 0;
  Vec2 ability_slot_start_dst = Vec2(0, 0);
  BottomNavBar() = default;
  BottomNavBar(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
};

#endif // UI_BOTTOMNAVBAR_H