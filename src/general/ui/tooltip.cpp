#include "ui/tooltip.h"
#include "game.h"
#include "item.h"
#include "utils_game.h"

ItemTooltip::ItemTooltip(Game &game) {
  background = ExpandableSprite(game);
  background.set_is_camera_rendered(false);
  display_name_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0),
                           150, TextAlignment::Left);
  description_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
                          TextAlignment::Left);
  display_name_text.is_camera_rendered = false;
  description_text.is_camera_rendered = false;
  cost_display = MoneyDisplay(game, Vec2(0, 0), "cost");
  cost_display.show_header = false;
}

void ItemTooltip::update(Game &game) {
  display_name_text.str = item.display_name;
  description_text.str = item.description;
  auto h = TOOLTIP_PADDING * 2;
  auto display_name_dims =
      display_name_text.measure_text(game, display_name_text.str);
  auto description_dims =
      description_text.measure_text(game, description_text.str);
  // last ITEM_DIM is the coin sprite
  h += display_name_dims.y + description_dims.y + ITEM_DIM;
  auto w = display_name_dims.x;
  if (description_dims.x > w) {
    w = description_dims.x;
  }
  w += ITEM_DIM + TOOLTIP_PADDING * 2 + 4;
  h += TOOLTIP_PADDING * 2;
  // expandable sprite draws in opengl space, text draws from origin top left
  auto tooltip_dst = keep_tooltip_on_screen(game, Rect(dst.x, dst.y, w, h),
                                            Vec2(ITEM_DIM + 6, ITEM_DIM));
  // position content in the tooltip rect
  content_top_left_dst = Vec2(tooltip_dst.x, tooltip_dst.y + tooltip_dst.h);

  auto text_x = content_top_left_dst.x + ITEM_DIM + 8;
  display_name_text.dst.set_xy(text_x, content_top_left_dst.y - 2);
  description_text.dst.set_xy(text_x, content_top_left_dst.y - 2 - 14);
  cost_display.dst.x = text_x;
  cost_display.dst.y =
      description_text.dst.y - description_dims.y - ITEM_DIM - 3;
  display_name_text.update(game);
  description_text.update(game);
  cost_display.coin.quantity = item.cost;
  cost_display.update(game);
  background.update_layout(game, tooltip_dst);
  background.update(game);
}

void ItemTooltip::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  background.draw(game);
  item.draw_at_dst(game, false,
                   Vec2(content_top_left_dst.x + TOOLTIP_PADDING,
                        content_top_left_dst.y - ITEM_DIM - TOOLTIP_PADDING));
  display_name_text.draw(game);
  description_text.draw(game);
  cost_display.draw(game);
}

void ItemTooltip::set_item_and_dst(Game &game, Item &_item, Rect _dst) {
  if (_item.item_name == ItemName::None) {
    return;
  }
  is_hidden = false;
  dst = _dst;
  if (item.item_name != _item.item_name) {
    item = _item;
  }
}

AbilityTooltip::AbilityTooltip(Game &game) {
  background = ExpandableSprite(game);
  background.set_is_camera_rendered(false);
  display_name_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0),
                           150, TextAlignment::Left);
  description_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
                          TextAlignment::Left);
  display_name_text.is_camera_rendered = false;
  description_text.is_camera_rendered = false;
}

void AbilityTooltip::update(Game &game) {
  display_name_text.str = ability.display_name;
  description_text.str = ability.description;
  auto h = TOOLTIP_PADDING * 2;
  auto display_name_dims =
      display_name_text.measure_text(game, display_name_text.str);
  auto description_dims =
      description_text.measure_text(game, description_text.str);
  h += display_name_dims.y + description_dims.y;
  auto w = display_name_dims.x;
  if (description_dims.x > w) {
    w = description_dims.x + 2;
  }
  w += ITEM_DIM + TOOLTIP_PADDING * 2;
  // expandable sprite draws in opengl space, text draws from origin top left
  auto tooltip_dst = keep_tooltip_on_screen(game, Rect(dst.x, dst.y, w, h),
                                            Vec2(ITEM_DIM + 6, ITEM_DIM));
  // position content in the tooltip rect
  content_top_left_dst = Vec2(tooltip_dst.x, tooltip_dst.y + tooltip_dst.h);
  auto text_x = content_top_left_dst.x + ITEM_DIM + 8;
  display_name_text.dst.set_xy(text_x, content_top_left_dst.y - 2);
  description_text.dst.set_xy(text_x, content_top_left_dst.y - 2 - 14);
  display_name_text.update(game);
  description_text.update(game);
  background.update_layout(game, tooltip_dst);
  background.update(game);
}

void AbilityTooltip::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  if (ability.ability_name == AbilityName::None) {
    return;
  }
  background.draw(game);
  ability.draw_at_dst(
      game, false,
      Vec2(content_top_left_dst.x + TOOLTIP_PADDING,
           content_top_left_dst.y - ITEM_DIM - TOOLTIP_PADDING));
  display_name_text.draw(game);
  description_text.draw(game);
}

void AbilityTooltip::set_ability_and_dst(Game &game, Ability &_ability,
                                         Rect _dst) {
  is_hidden = false;
  dst = _dst;
  if (ability.ability_name != _ability.ability_name) {
    ability = _ability;
  }
}

// se

StatusEffectTooltip::StatusEffectTooltip(Game &game) {
  background = ExpandableSprite(game);
  background.set_is_camera_rendered(false);
  display_name_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0),
                           150, TextAlignment::Left);
  description_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
                          TextAlignment::Left);
  display_name_text.is_camera_rendered = false;
  description_text.is_camera_rendered = false;
}

void StatusEffectTooltip::update(Game &game) {
  display_name_text.str = status_effect.display_name;
  description_text.str = status_effect.description;
  auto h = TOOLTIP_PADDING * 2;
  auto display_name_dims =
      display_name_text.measure_text(game, display_name_text.str);
  auto description_dims =
      description_text.measure_text(game, description_text.str);
  h += display_name_dims.y + description_dims.y;
  auto w = display_name_dims.x;
  if (description_dims.x > w) {
    w = description_dims.x + 2;
  }
  w += ITEM_DIM + TOOLTIP_PADDING * 2;
  // expandable sprite draws in opengl space, text draws from origin top left
  auto tooltip_dst = keep_tooltip_on_screen(game, Rect(dst.x, dst.y, w, h),
                                            Vec2(ITEM_DIM + 6, ITEM_DIM));
  // position content in the tooltip rect
  content_top_left_dst = Vec2(tooltip_dst.x, tooltip_dst.y + tooltip_dst.h);
  auto text_x = content_top_left_dst.x + ITEM_DIM + 8;
  display_name_text.dst.set_xy(text_x, content_top_left_dst.y - 2);
  description_text.dst.set_xy(text_x, content_top_left_dst.y - 2 - 14);
  display_name_text.update(game);
  description_text.update(game);
  background.update_layout(game, tooltip_dst);
  background.update(game);
}

void StatusEffectTooltip::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  if (status_effect.status_effect_name == StatusEffectName::None) {
    return;
  }
  background.draw(game);
  draw_sprite_at_dst(game, status_effect.portrait.image,
                     status_effect.portrait.srcs,
                     status_effect.portrait.spawn_time,
                     status_effect.portrait.anim_speed, false,
                     Vec2(content_top_left_dst.x + TOOLTIP_PADDING,
                          content_top_left_dst.y - ITEM_DIM - TOOLTIP_PADDING));
  display_name_text.draw(game);
  description_text.draw(game);
}

void StatusEffectTooltip::set_status_effect_and_dst(
    Game &game, StatusEffect &_status_effect, Rect _dst) {
  is_hidden = false;
  dst = _dst;
  if (status_effect.status_effect_name != _status_effect.status_effect_name) {
    status_effect = _status_effect;
  }
}

// opengl coords
Rect keep_tooltip_on_screen(Game &game, Rect _dst, Vec2 _slot_dim) {
  auto dst = _dst;
  if (dst.x < 0) {
    dst.x = 0;
  }
  if (dst.x + dst.w > game.engine.base_resolution.x) {
    // go to the left of the slot
    dst.x -= _slot_dim.x;
    // move x w amount to the left so that it lines up with the slot
    dst.x -= dst.w;
  }
  if (dst.y < 0) {
    dst.y = 0;
  }
  if (dst.y + dst.h > game.engine.base_resolution.y) {
    dst.y = game.engine.base_resolution.y - dst.h - BACKGROUND_PADDING;
  }
  return dst;
}