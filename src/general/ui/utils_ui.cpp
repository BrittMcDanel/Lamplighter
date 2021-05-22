#include "ui/utils_ui.h"
#include "game.h"
#include "unit.h"
#include "utils_game.h"

UIEquipmentSlots::UIEquipmentSlots(Game &game) {
  primary_hand = create_slot(game);
  secondary_hand = create_slot(game);
  head = create_slot(game);
  armor = create_slot(game);
  boots = create_slot(game);
}

void UIEquipmentSlots::update(Game &game, Unit &unit) {
  primary_hand.update(game);
  secondary_hand.update(game);
  head.update(game);
  armor.update(game);
  boots.update(game);

  if (primary_hand.input_events.is_mouse_over) {
    update_slot(game, unit, primary_hand, unit.equipment.primary_hand);
  } else if (secondary_hand.input_events.is_mouse_over) {
    update_slot(game, unit, secondary_hand, unit.equipment.secondary_hand);
  } else if (armor.input_events.is_mouse_over) {
    update_slot(game, unit, armor, unit.equipment.armor);
  } else if (head.input_events.is_mouse_over) {
    update_slot(game, unit, head, unit.equipment.head);
  } else if (boots.input_events.is_mouse_over) {
    update_slot(game, unit, boots, unit.equipment.boots);
  }
}

void UIEquipmentSlots::draw(Game &game, Unit &unit) {
  primary_hand.draw(game);
  secondary_hand.draw(game);
  head.draw(game);
  armor.draw(game);
  boots.draw(game);
  auto offset = Vec2(SLOT_OFFSET_X, SLOT_OFFSET_Y);
  unit.equipment.primary_hand.draw_at_dst(game, false,
                                          primary_hand.dst.get_xy() + offset);
  unit.equipment.secondary_hand.draw_at_dst(
      game, false, secondary_hand.dst.get_xy() + offset);
  unit.equipment.head.draw_at_dst(game, false, head.dst.get_xy() + offset);
  unit.equipment.armor.draw_at_dst(game, false, armor.dst.get_xy() + offset);
  unit.equipment.boots.draw_at_dst(game, false, boots.dst.get_xy() + offset);
}

void UIEquipmentSlots::update_slot(Game &game, Unit &unit, UISprite &slot,
                                   Item &item) {
  // prevent tooltip or drag and drop if
  // the equipment window is hidden.
  if (!game.ui.equip_window.is_hidden) {
    auto tooltip_dst = slot.dst;
    tooltip_dst.x += SLOT_DIM + 1;
    tooltip_dst.y += 1;
    game.ui.item_tooltip.set_item_and_dst(game, item, tooltip_dst);
    if (!game.ui.drag_ghost.is_dragging &&
        slot.input_events.was_mouse_down_when_mouse_over &&
        game.engine.mouse_point_game_rect_scaled !=
            slot.input_events.mouse_dst_when_mouse_down) {
      game.ui.drag_ghost.drop_callback.set_as_unequip_item(item, unit.guid);
      game.ui.drag_ghost.start_drag(item);
    }
  }
}

Tab::Tab(TabName _tab_name, UISprite &_non_active_tab, UISprite &_active_tab,
         Text &_text) {
  tab_name = _tab_name;
  non_active_tab = _non_active_tab;
  active_tab = _active_tab;
  text = _text;
}

void Tab::update(Game &game) {
  non_active_tab.update(game);
  active_tab.update(game);
  auto tab_h = non_active_tab.srcs.at(0).image_location.src.h;
  auto text_h = text.dst.h;
  text.dst.set_xy(non_active_tab.dst.x,
                  non_active_tab.dst.y + tab_h - (text_h / 2));
  text.update(game);
}

void Tab::draw(Game &game) {
  if (is_active) {
    active_tab.draw(game);
  } else {
    non_active_tab.draw(game);
  }
  text.draw(game);
}

void Tab::set_dst_xy(Vec2 _dst) {
  non_active_tab.dst.set_xy(_dst.x, _dst.y);
  active_tab.dst.set_xy(_dst.x, _dst.y);
  auto tab_h = non_active_tab.srcs.at(0).image_location.src.h;
  auto text_h = text.dst.h;
  text.dst.set_xy(non_active_tab.dst.x,
                  non_active_tab.dst.y + tab_h - (text_h / 2));
}

Tab Tab::create_default_tab(Game &game, TabName _tab_name,
                            const char *tab_str) {
  auto image = game.engine.get_image(ImageName::UI);
  auto tab_w = 72;
  auto non_active_tab =
      UISprite(game, image, Vec2(0, 0),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(0, 656, tab_w, 24))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(146, 656, tab_w, 24))),
               },
               100, vector<SpriteSrc>(), 100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(219, 656, tab_w, 24))),
               },
               100);
  auto active_tab =
      UISprite(game, image, Vec2(0, 0),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(73, 656, tab_w, 24))),
               },
               100);
  auto text = Text(game, 10, FontColor::Black, tab_str, Vec2(0, 0), tab_w,
                   TextAlignment::Center);
  text.is_camera_rendered = false;
  return Tab(_tab_name, non_active_tab, active_tab, text);
}

Tabs::Tabs(Game &game, Vec2 _dst, int _tab_margin_right,
           TabName _active_tab_name, vector<Tab> _tabs) {
  dst = _dst;
  tab_margin_right = _tab_margin_right;
  active_tab_name = _active_tab_name;
  tabs = _tabs;
}

void Tabs::update(Game &game) {
  active_tab_changed = false;
  if (tabs.size() > 0) {
    auto tab_dst = dst;
    for (auto &tab : tabs) {
      assert(tab.non_active_tab.srcs.size() > 0);
      if (tab.tab_name == active_tab_name) {
        tab.is_active = true;
      } else {
        tab.is_active = false;
      }
      tab.set_dst_xy(tab_dst);
      tab_dst.x +=
          tab.non_active_tab.srcs.at(0).image_location.src.w + tab_margin_right;
      tab.update(game);
      // change active tab name if user clicks on a tab
      if ((tab.non_active_tab.input_events.is_click ||
           tab.active_tab.input_events.is_click) &&
          tab.tab_name != active_tab_name) {
        active_tab_changed = true;
        active_tab_name = tab.tab_name;
      }
    }
  }
}

void Tabs::draw(Game &game) {
  for (auto &tab : tabs) {
    tab.draw(game);
  }
}

void Tabs::set_dst_xy(Game &game, Vec2 _dst) {
  dst = _dst;
  update(game);
}

bool Tabs::is_mouse_over(Game &game) {
  for (auto &tab : tabs) {
    // really only need to check either active or inactive tab ui sprite
    // because they are the same size. Just checking both for now.
    if (is_mouse_over_dst(game, false, tab.non_active_tab.dst) ||
        is_mouse_over_dst(game, false, tab.active_tab.dst)) {
      return true;
    }
  }
  return false;
}

ButtonIcon::ButtonIcon(UISprite &_background, UISprite &_icon) {
  background = _background;
  icon = _icon;
}

void ButtonIcon::update(Game &game) {
  assert(icon.srcs.size() > 0);
  background.update(game);
  icon.update(game);
}

void ButtonIcon::draw(Game &game) {
  background.draw(game);
  icon.draw(game);
}

ButtonIcon ButtonIcon::create_close_button(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  auto background =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(127, 0, 24, 24))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(177, 0, 24, 24))),
               },
               100, vector<SpriteSrc>(), 100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(227, 0, 24, 24))),
               },
               100);
  auto close_icon =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(112, 32, 12, 12))),
               },
               100);
  return ButtonIcon(background, close_icon);
}

ButtonText::ButtonText(UISprite &_background, Text &_text) {
  background = _background;
  text = _text;
}

void ButtonText::update(Game &game) {
  auto background_h = background.srcs.at(0).image_location.src.h;
  text.dst.set_xy(background.dst.x,
                  background.dst.y + background_h - (text.dst.h / 2));

  background.update(game);
  text.update(game);
}

void ButtonText::draw(Game &game) {
  background.draw(game);
  text.draw(game);
}

ButtonText ButtonText::create_default_button(Game &game, Vec2 _dst,
                                             const char *_str) {
  auto image = game.engine.get_image(ImageName::UI);
  auto background_w = 72;
  auto background = UISprite(
      game, image, _dst,
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(0, 631, background_w, 24))),
      },
      100,
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(73, 631, background_w, 24))),
      },
      100,
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(146, 631, background_w, 24))),
      },
      100,
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(219, 631, background_w, 24))),
      },
      100);
  auto text = Text(game, 10, FontColor::WhiteShadow, _str, _dst, background_w,
                   TextAlignment::Center);
  text.is_camera_rendered = false;
  return ButtonText(background, text);
}

void ButtonText::set_is_hidden(bool is_hidden) {
  background.is_hidden = is_hidden;
  text.is_hidden = is_hidden;
}

MoneyDisplay::MoneyDisplay(Game &game, Vec2 _dst, const char *_header_str) {
  dst = _dst;
  auto background_w = 100;
  header_text =
      Text(game, 10, FontColor::Black, _header_str, Vec2(_dst.x, _dst.y + 16),
           background_w, TextAlignment::Left);
  header_text.is_camera_rendered = false;
  coin = get_coin_item(game);
  coin.sprite.dst.set_xy(_dst.x + 70, _dst.y);
  coin.set_is_camera_rendered(false);
  cash = get_cash_item(game);
  cash.sprite.dst.set_xy(_dst.x + 40, _dst.y);
  cash.set_is_camera_rendered(false);
}

void MoneyDisplay::update(Game &game) {
  if (show_header) {
    header_text.dst.set_xy(dst.x, dst.y + 16);
    coin.sprite.dst.set_xy(dst.x + 70, dst.y);
    cash.sprite.dst.set_xy(dst.x + 40, dst.y);
  } else {
    coin.sprite.dst.set_xy(dst.x + 30, dst.y);
    cash.sprite.dst.set_xy(dst.x, dst.y);
  }
  header_text.update(game);
  coin.update(game);
  cash.update(game);
}

void MoneyDisplay::draw(Game &game) {
  if (show_header) {
    header_text.draw(game);
  }
  coin.draw(game);
  cash.draw(game);
}

UISprite get_money_icon(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::Items);
  auto money_icon =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))),
               },
               100);
  return money_icon;
}

UISprite get_portrait_background(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  auto portrait_background =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(252, 0, 28, 32))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(281, 0, 28, 32))),
               },
               100, vector<SpriteSrc>(), 100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(310, 0, 28, 32))),
               },
               100);
  return portrait_background;
}

UISprite get_action_point_icon(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  auto action_point_background =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(144, 28, 20, 30))),
               },
               100);
  return action_point_background;
}

UISprite get_empty_action_point_icon(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  auto action_point_background =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(165, 28, 20, 30))),
               },
               100);
  return action_point_background;
}

UISprite get_active_action_point_icon(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  auto action_point_background =
      UISprite(game, image, _dst,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(186, 28, 20, 30))),
               },
               100);
  return action_point_background;
}

UISprite create_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(102, 0, 24, 24))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(152, 0, 24, 24))),
                  },
                  100,
                  // disabled srcs uses the same srcs as srcs
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(102, 0, 24, 24))),
                  },
                  100, vector<SpriteSrc>(), 100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(339, 0, 24, 24))),
                  },
                  100);
}

UISprite create_unlearned_skill_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(498, 0, 24, 24))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(523, 0, 24, 24))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(573, 0, 24, 24))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(548, 0, 24, 24))),
                  },
                  100);
}

UISprite create_button_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(80, 24, 24, 26))),
                  },
                  100);
}

UISprite create_turn_order_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(368, 0, 24, 28))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(393, 0, 24, 28))),
                  },
                  100);
}

UISprite create_turn_order_charge_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(418, 0, 24, 59))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(443, 0, 24, 59))),
                  },
                  100);
}

UISprite create_status_effect_slot(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(468, 0, 14, 14))),
                  },
                  100,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(483, 0, 14, 14))),
                  },
                  100);
}

UISprite get_skill_tree_up_arrow(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(21, 0, 8, 5))),
                  },
                  100);
}

UISprite get_skill_tree_down_arrow(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(21, 6, 8, 5))),
                  },
                  100);
}

UISprite get_skill_tree_left_arrow(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(36, 0, 5, 8))),
                  },
                  100);
}

UISprite get_skill_tree_right_arrow(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(30, 0, 5, 8))),
                  },
                  100);
}

UISprite get_skill_tree_top_left_connection(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(42, 0, 4, 4))),
                  },
                  100);
}

UISprite get_skill_tree_top_right_connection(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(47, 0, 4, 4))),
                  },
                  100);
}

UISprite get_skill_tree_bottom_left_connection(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(42, 5, 4, 4))),
                  },
                  100);
}

UISprite get_skill_tree_bottom_right_connection(Game &game, Vec2 _dst) {
  auto image = game.engine.get_image(ImageName::UI);
  return UISprite(game, image, _dst,
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(image, Rect(47, 5, 4, 4))),
                  },
                  100);
}