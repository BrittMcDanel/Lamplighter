#ifndef UI_TOOLTIP_H
#define UI_TOOLTIP_H

#include "ability.h"
#include "fixed_sprite.h"
#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "tweenable_sprite.h"
#include "ui/expandable_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct ItemTooltip {
  Vec2 content_top_left_dst = Vec2(0, 0);
  Rect dst;
  Item item;
  ExpandableSprite background;
  Text display_name_text;
  Text description_text;
  MoneyDisplay cost_display;
  bool is_hidden = true;
  ItemTooltip() = default;
  ItemTooltip(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_item_and_dst(Game &game, Item &_item, Rect r);
};

struct AbilityTooltip {
  Vec2 content_top_left_dst = Vec2(0, 0);
  Rect dst;
  Ability ability;
  ExpandableSprite background;
  Text display_name_text;
  Text description_text;
  bool is_hidden = true;
  AbilityTooltip() = default;
  AbilityTooltip(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_ability_and_dst(Game &game, Ability &_ability, Rect r);
};

struct StatusEffectTooltip {
  Vec2 content_top_left_dst = Vec2(0, 0);
  Rect dst;
  StatusEffect status_effect;
  ExpandableSprite background;
  Text display_name_text;
  Text description_text;
  bool is_hidden = true;
  StatusEffectTooltip() = default;
  StatusEffectTooltip(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void set_status_effect_and_dst(Game &game, StatusEffect &_status_effect,
                                 Rect r);
};

Rect keep_tooltip_on_screen(Game &game, Rect _dst, Vec2 _slot_dim);

#endif // UI_TOOLTIP_H