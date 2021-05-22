#ifndef UI_EQUIPWINDOW_H
#define UI_EQUIPWINDOW_H

#include "rapidjson/document.h"
#include "robin_hood.h"
#include "tween.h"
#include "ui/inventory_window.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include "utils_ui.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;
struct Unit;

struct EquipWindow {
  int start_y;
  UISprite background = UISprite();
  UISprite inventory_background = UISprite();
  UIEquipmentSlots equipment_slots;
  InventoryWindow inventory_window;
  MoneyDisplay funds_money_display = MoneyDisplay();
  bool is_hidden = true;
  EquipWindow() = default;
  EquipWindow(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
};

#endif // UI_EQUIPWINDOW_H