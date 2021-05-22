#ifndef UI_SHOPWINDOW_H
#define UI_SHOPWINDOW_H

#include "inventory.h"
#include "rapidjson/document.h"
#include "robin_hood.h"
#include "tween.h"
#include "ui/inventory_window.h"
#include "ui/ui_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;
struct Unit;

struct ShopWindow {
  int start_y;
  Vec2 left_dst;
  Vec2 right_dst;
  UISprite background = UISprite();
  Tabs shop_mode_tabs = Tabs();
  ButtonIcon close_button = ButtonIcon();
  Text shop_description_text = Text();
  // merchant's inventory for sale
  InventoryWindow buy_merchant_inventory_window;
  // what the player wants to buy from the merchant
  InventoryWindow buy_merchant_order_window;
  // player's inventory for sale
  InventoryWindow sell_player_inventory_window;
  // waht the player wants to sell to the merchant
  InventoryWindow sell_merchant_order_window;
  // what the player is going to buy from the merchant
  Inventory buy_order_inventory;
  // what the player is goign to sell to the merchant
  Inventory sell_order_inventory;
  MoneyDisplay funds_display = MoneyDisplay();
  MoneyDisplay total_display = MoneyDisplay();
  ButtonText confirm_button;
  boost::uuids::uuid seller_unit_guid;
  bool is_hidden = true;
  ShopWindow() = default;
  ShopWindow(Game &game);
  void update_always(Game &game, Unit &unit);
  void update_when_open(Game &game, Unit &unit, Unit &seller);
  void draw(Game &game, Unit &unit, Unit &seller);
  void show_shop(boost::uuids::uuid _seller_unit_guid);
};

#endif // UI_SHOPWINDOW_H