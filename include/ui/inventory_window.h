#ifndef UI_INVENTORYWINDOW_H
#define UI_INVENTORYWINDOW_H

#include "rapidjson/document.h"
#include "tween.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;
struct Inventory;

// when more units are added that the player can control
// make sure all the items end up in the player units inventory,
// not the inventory of the player controlled unit who got the item
// (i.e only one inventory) so that a shared inventory can be used.
struct InventoryWindow {
  Vec2 dst = Vec2(0, 0);
  vector<UISprite> slots = vector<UISprite>();
  bool is_hidden = true;
  InventoryWindow() = default;
  InventoryWindow(Game &game, Vec2 _dst);
  void update(Game &game, Inventory &inventory, DropType _drop_type,
              boost::uuids::uuid unit_guid);
  void draw(Game &game, Inventory &inventory);
};

#endif // UI_INVENTORYWINDOW_H