#ifndef UI_UI_H
#define UI_UI_H

#include "item.h"
#include "pool.h"
#include "rapidjson/document.h"
#include "tween.h"
#include "tweenable_sprite.h"
#include "ui/action_point_display.h"
#include "ui/alert.h"
#include "ui/bottom_navbar.h"
#include "ui/equip_window.h"
#include "ui/inventory_window.h"
#include "ui/item_pickup_list.h"
#include "ui/portraits.h"
#include "ui/shop_window.h"
#include "ui/skill_tree_window.h"
#include "ui/tooltip.h"
#include "ui/turn_order_display.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>
#include <functional>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct DropCallback {
  DropType drop_type = DropType::None;
  boost::uuids::uuid unit_guid;
  boost::uuids::uuid merchant_guid;
  Item item = Item();
  Ability ability = Ability();
  int item_idx = 0;
  int ability_idx = 0;
  DropCallback() = default;
  void set_as_equip_item(Item &_item, boost::uuids::uuid _unit_guid,
                         int _item_idx);
  void set_as_unequip_item(Item &_item, boost::uuids::uuid _unit_guid);
  void set_as_shop_buy_item(Item &_item, boost::uuids::uuid _merchant_guid);
  void set_as_equip_ability(const Ability &_ability,
                            boost::uuids::uuid _unit_guid);
  void set_as_bottom_navbar_ability_swap(boost::uuids::uuid _unit_guid,
                                         int _ability_idx);
};

struct DragGhost {
  DragGhostType drag_ghost_type = DragGhostType::None;
  bool is_dragging = false;
  UISprite drag_ghost_ui_sprite = UISprite();
  TweenableSprite drag_ghost_tweenable_sprite = TweenableSprite();
  Item drag_ghost_item = Item();
  DropCallback drop_callback;
  DragGhost() = default;
  void update(Game &game);
  void draw(Game &game);
  // set the drag ghost's drop callback before calling start drag
  void start_drag(UISprite &_drag_ghost);
  void start_drag(TweenableSprite &_drag_ghost);
  void start_drag(Item &_drag_ghost);
  void handle_on_drop(Game &game);
};

struct UI {
  BottomNavBar bottom_navbar;
  ItemPickupList item_pickup_list;
  EquipWindow equip_window;
  SkillTreeWindow skill_tree_window;
  ShopWindow shop_window;
  Portraits player_portraits;
  ActionPointDisplay action_point_display;
  TurnOrderDisplay turn_order_display;
  Alert battle_start_alert;
  DragGhost drag_ghost;
  ItemTooltip item_tooltip;
  AbilityTooltip ability_tooltip;
  StatusEffectTooltip status_effect_tooltip;
  bool is_mouse_over_ui = false;
  UI() = default;
  UI(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_is_mouse_over_ui(const char *who_set_it);
};

void on_drop_equip(Game &game, Unit &unit, Item &item,
                   DropCallback &drop_callback);

#endif // UI_UI_H