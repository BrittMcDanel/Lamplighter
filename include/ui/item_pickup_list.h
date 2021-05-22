#ifndef UI_ITEMPICKUPLIST_H
#define UI_ITEMPICKUPLIST_H

#include "engine.h"
#include "item.h"
#include "tween.h"
#include "ui/expandable_sprite.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct ItemPickupPanel {
  boost::uuids::uuid guid;
  int idx;
  Rect dst;
  Vec2 margin_from_window;
  int height;
  int item_dim;
  int padding;
  int margin_top;
  Uint32 remove_time;
  Vec2 item_dst;
  Item item;
  ExpandableSprite background;
  Text display_name_text;
  Tweens tweens;
  ItemPickupPanel() = default;
  ItemPickupPanel(Game &game, Item &_item, int _idx);
  void update(Game &game);
  void draw(Game &game);
  Vec2 get_dst(Game &game);
  void set_complete_state_tween(Game &game);
};

struct ItemPickupList {
  vector<ItemPickupPanel> item_pickup_panels = vector<ItemPickupPanel>();
  void update(Game &game);
  void draw(Game &game);
  void add_item(Game &game, Item &_item);
  void remove_panel_with_guid(Game &game, boost::uuids::uuid guid);
  ItemPickupPanel &get_panel_with_guid(Game &game, boost::uuids::uuid);
};

#endif // UI_ITEMPICKUPLIST_H