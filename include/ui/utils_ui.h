#ifndef UI_UTILSUI_H
#define UI_UTILSUI_H

#include "fixed_sprite.h"
#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <functional>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Unit;

struct UIEquipmentSlots {
  UISprite primary_hand;
  UISprite secondary_hand;
  UISprite head;
  UISprite armor;
  UISprite boots;
  UIEquipmentSlots() = default;
  UIEquipmentSlots(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
  void update_slot(Game &game, Unit &unit, UISprite &slot, Item &item);
};

struct Tab {
  TabName tab_name;
  UISprite non_active_tab;
  UISprite active_tab;
  Text text;
  bool is_active = false;
  Tab() = default;
  Tab(TabName _tab_name, UISprite &_non_active_tab, UISprite &_active_tab,
      Text &_text);
  void update(Game &game);
  void draw(Game &game);
  void set_dst_xy(Vec2 _dst);
  static Tab create_default_tab(Game &game, TabName _tab_name,
                                const char *tab_str);
};

struct Tabs {
  TabName active_tab_name = TabName::None;
  Vec2 dst = Vec2(0, 0);
  int tab_margin_right;
  vector<Tab> tabs = vector<Tab>();
  bool active_tab_changed = false;
  Tabs() = default;
  Tabs(Game &game, Vec2 _dst, int _tab_margin_right, TabName _active_tab_name,
       vector<Tab> _tabs);
  void update(Game &game);
  void draw(Game &game);
  void set_dst_xy(Game &game, Vec2 _dst);
  void update_layout(Game &game);
  bool is_mouse_over(Game &game);
};

struct ButtonIcon {
  UISprite background;
  UISprite icon;
  ButtonIcon() = default;
  ButtonIcon(UISprite &_background, UISprite &_icon);
  void update(Game &game);
  void draw(Game &game);
  static ButtonIcon create_close_button(Game &game, Vec2 _dst);
};

struct ButtonText {
  UISprite background;
  Text text;
  ButtonText() = default;
  ButtonText(UISprite &_background, Text &_text);
  void update(Game &game);
  void draw(Game &game);
  void set_is_hidden(bool is_hidden);
  static ButtonText create_default_button(Game &game, Vec2 _dst,
                                          const char *_str);
};

struct MoneyDisplay {
  Vec2 dst;
  Text header_text;
  Item coin;
  Item cash;
  bool show_header = true;
  MoneyDisplay() = default;
  MoneyDisplay(Game &game, Vec2 _dst, const char *_header_str);
  void update(Game &game);
  void draw(Game &game);
};

UISprite get_money_icon(Game &game, Vec2 _dst);
UISprite get_portrait_background(Game &game, Vec2 _dst);
UISprite get_action_point_icon(Game &game, Vec2 _dst);
UISprite get_empty_action_point_icon(Game &game, Vec2 _dst);
UISprite get_active_action_point_icon(Game &game, Vec2 _dst);
UISprite create_slot(Game &game);
UISprite create_unlearned_skill_slot(Game &game);
UISprite create_button_slot(Game &game);
UISprite create_turn_order_slot(Game &game);
UISprite create_turn_order_charge_slot(Game &game);
UISprite create_status_effect_slot(Game &game);
UISprite get_skill_tree_up_arrow(Game &game, Vec2 _dst);
UISprite get_skill_tree_down_arrow(Game &game, Vec2 _dst);
UISprite get_skill_tree_left_arrow(Game &game, Vec2 _dst);
UISprite get_skill_tree_right_arrow(Game &game, Vec2 _dst);
UISprite get_skill_tree_top_left_connection(Game &game, Vec2 _dst);
UISprite get_skill_tree_top_right_connection(Game &game, Vec2 _dst);
UISprite get_skill_tree_bottom_left_connection(Game &game, Vec2 _dst);
UISprite get_skill_tree_bottom_right_connection(Game &game, Vec2 _dst);

#endif // UI_UTILSUI_H