#include "ui/action_point_display.h"
#include "game.h"

ActionPointDisplay::ActionPointDisplay(Game &game) {
  available_icons = vector<UISprite>();
  active_icons = vector<UISprite>();
  inactive_icons = vector<UISprite>();
  for (size_t i = 0; i < 20; i++) {
    available_icons.push_back(get_action_point_icon(game, Vec2(0, 0)));
    active_icons.push_back(get_active_action_point_icon(game, Vec2(0, 0)));
    inactive_icons.push_back(get_empty_action_point_icon(game, Vec2(0, 0)));
  }
  end_turn_button =
      ButtonText::create_default_button(game, Vec2(0, 0), "End Turn");
}

void ActionPointDisplay::update(Game &game, int available, int total) {
  hide_all();
  auto diff = total - available;
  auto active_diff = available - active;
  if (active_diff < 0) {
    active_diff = 0;
  }
  auto icon_size = 20;
  auto margin_right = 2;
  auto w = icon_size + margin_right;
  auto total_w = total * w +
                 end_turn_button.background.srcs.at(0).image_location.src.w +
                 margin_right * 4;
  auto start_x = (game.engine.base_resolution.x / 2) - (total_w / 2);
  auto start_y = 22;
  GAME_ASSERT(active_diff >= 0);
  GAME_ASSERT(active >= 0);
  GAME_ASSERT(diff >= 0);
  for (int i = 0; i < active_diff; i++) {
    auto &available_icon = available_icons[i];
    available_icon.is_hidden = false;
    available_icon.dst.set_xy(start_x, start_y);
    available_icon.update(game);
    start_x += w;
  }
  for (int i = 0; i < active; i++) {
    auto &active_icon = active_icons[i];
    active_icon.is_hidden = false;
    active_icon.dst.set_xy(start_x, start_y);
    active_icon.update(game);
    start_x += w;
  }
  for (int i = 0; i < diff; i++) {
    auto &inactive_icon = inactive_icons[i];
    inactive_icon.is_hidden = false;
    inactive_icon.dst.set_xy(start_x, start_y);
    inactive_icon.update(game);
    start_x += w;
  }

  end_turn_button.set_is_hidden(is_hidden);
  end_turn_button.background.dst.set_xy(start_x + margin_right * 4,
                                        start_y + 8);
  end_turn_button.update(game);

  if (end_turn_button.background.input_events.is_click) {
    auto battle_found = false;
    // go through all the client's units and see which unit
    // is the active unit in a battle, then advance that battle's
    // turn.
    for (auto &unit_guid : game.map.player_unit_guids) {
      auto &u = game.map.unit_dict[unit_guid];
      if (u.in_battle) {
        auto &battle = game.map.battle_dict[u.battle_guid];
        if (battle.acting_unit_guid == unit_guid) {
          battle_found = true;
          battle.end_turn(game);
        }
      }
    }
    GAME_ASSERT(battle_found);
  }
  if (end_turn_button.background.input_events.is_mouse_over) {
    game.ui.set_is_mouse_over_ui("action point display");
  }
}

void ActionPointDisplay::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  for (auto &available_icon : available_icons) {
    available_icon.draw(game);
  }
  for (auto &active_icon : active_icons) {
    active_icon.draw(game);
  }
  for (auto &inactive_icon : inactive_icons) {
    inactive_icon.draw(game);
  }

  end_turn_button.draw(game);
}

void ActionPointDisplay::hide_all() {
  for (auto &available_icon : available_icons) {
    available_icon.is_hidden = true;
  }
  for (auto &active_icon : active_icons) {
    active_icon.is_hidden = true;
  }
  for (auto &inactive_icon : inactive_icons) {
    inactive_icon.is_hidden = true;
  }
}