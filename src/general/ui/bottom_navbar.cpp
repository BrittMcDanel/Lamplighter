#include "bottom_navbar.h"
#include "game.h"
#include "utils.h"
#include "utils_game.h"

BottomNavBar::BottomNavBar(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  auto items_image = game.engine.get_image(ImageName::Items);
  background =
      UISprite(game, image, Vec2(0, 0),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(208, 1000, 640, 24))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(208, 1000, 640, 24))),
               },
               100);
  inventory_button_slot = create_slot(game);
  inventory_button_slot.dst.set_xy(0, 0);
  inventory_button_slot_icon =
      UISprite(game, items_image, Vec2(0, 0),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(40, 40, 20, 20))),
               },
               100);
  auto skill_tree_button_slot = create_slot(game);
  skill_tree_button_slot.dst.set_xy(SLOT_DIM + SLOT_MARGIN_RIGHT, 0);
  auto skill_tree_button_icon =
      UISprite(game, items_image,
               Vec2(skill_tree_button_slot.dst.get_xy() +
                    Vec2(SLOT_OFFSET_X, SLOT_OFFSET_Y)),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(0, 60, 20, 20))),
               },
               100);
  skill_tree_button =
      ButtonIcon(skill_tree_button_slot, skill_tree_button_icon);
  ability_slot_start_dst = Vec2(116, 0);
  ability_slots = vector<vector<UISprite>>();
  for (size_t i = 0; i < NUM_ABILITY_SLOT_ROWS; i++) {
    ability_slots.push_back(vector<UISprite>());
    for (size_t j = 0; j < NUM_ABILITY_SLOTS_PER_ROW; j++) {
      ability_slots[i].push_back(create_slot(game));
    }
  }
  auto arrow_x = ability_slot_start_dst.x - 10;
  ability_up_arrow =
      UISprite(game, items_image, Vec2(arrow_x, background.dst.y + 12),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(100, 40, 8, 8))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(100, 48, 8, 8))),
               },
               100, vector<SpriteSrc>(), 100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(116, 40, 8, 8))),
               },
               100);
  ability_down_arrow =
      UISprite(game, items_image, Vec2(arrow_x, background.dst.y + 2),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(108, 40, 8, 8))),
               },
               100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(108, 48, 8, 8))),
               },
               100, vector<SpriteSrc>(), 100,
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(items_image, Rect(124, 40, 8, 8))),
               },
               100);
  ability_row_text =
      Text(game, 10, FontColor::WhiteShadow, "0",
           Vec2(ability_slot_start_dst.x - 18, 18), 100, TextAlignment::Left);
  ability_row_text.is_camera_rendered = false;
}

void BottomNavBar::update(Game &game, Unit &unit) {
  inventory_button_slot.dst.set_xy(0, 0);
  inventory_button_slot_icon.dst.set_xy(SLOT_OFFSET_X, SLOT_OFFSET_Y);
  if (is_mouse_over_dst(game, false, background.dst)) {
    game.ui.set_is_mouse_over_ui("bottom navbar");
  }
  background.update(game);
  inventory_button_slot.update(game);
  inventory_button_slot_icon.update(game);
  skill_tree_button.update(game);

  if (inventory_button_slot.input_events.is_click) {
    game.ui.equip_window.is_hidden = !game.ui.equip_window.is_hidden;
  }
  if (skill_tree_button.background.input_events.is_click) {
    game.ui.skill_tree_window.is_hidden = !game.ui.skill_tree_window.is_hidden;
  }

  // +1 is just for display, its to avoid showing zero
  ability_row_text.str = to_string(showing_ability_row_idx + 1);
  ability_row_text.update(game);
  ability_up_arrow.update(game);
  ability_down_arrow.update(game);

  if (ability_up_arrow.input_events.is_click) {
    showing_ability_row_idx += 1;
    if (showing_ability_row_idx > NUM_ABILITY_SLOT_ROWS - 1) {
      showing_ability_row_idx = 0;
    }
  }
  if (ability_down_arrow.input_events.is_click) {
    showing_ability_row_idx -= 1;
    if (showing_ability_row_idx < 0) {
      showing_ability_row_idx = NUM_ABILITY_SLOT_ROWS - 1;
    }
  }

  auto ability_selected = false;
  for (size_t i = 0; i < NUM_ABILITY_SLOT_ROWS; i++) {
    for (size_t j = 0; j < NUM_ABILITY_SLOTS_PER_ROW; j++) {
      auto &slot = ability_slots[i][j];
      auto &ability = unit.ability_slots[i][j];
      slot.is_hidden = (int)i != showing_ability_row_idx;
      slot.input_events.is_disabled = unit.is_battle_acting;
      slot.is_focused = unit.is_ability_selected &&
                        unit.selected_ability_idx.first == (int)i &&
                        unit.selected_ability_idx.second == (int)j;
      slot.dst.set_xy(ability_slot_start_dst.x + (j * SLOT_DIM) +
                          (j * SLOT_MARGIN_RIGHT),
                      background.dst.y);
      slot.update(game);

      if (ability.ability_name != AbilityName::None) {
        if (slot.input_events.is_mouse_over) {
          auto tooltip_dst = slot.dst;
          tooltip_dst.x += SLOT_DIM + 1;
          tooltip_dst.y += 1;
          game.ui.ability_tooltip.set_ability_and_dst(game, ability,
                                                      tooltip_dst);
        }

        if (!game.ui.drag_ghost.is_dragging &&
            slot.input_events.was_mouse_down_when_mouse_over &&
            game.engine.mouse_point_game_rect_scaled !=
                slot.input_events.mouse_dst_when_mouse_down) {
          game.ui.drag_ghost.drop_callback.set_as_bottom_navbar_ability_swap(
              unit.guid, j);
          game.ui.drag_ghost.start_drag(ability.portrait);
        }

        if (!game.ui.drag_ghost.is_dragging && slot.input_events.is_click) {
          ability_selected = true;
          unit.is_ability_selected = true;
          unit.stop_moving(game);
          unit.selected_ability_idx = make_pair(i, j);
        }
      }
    }
  }
}

void BottomNavBar::draw(Game &game, Unit &unit) {
  background.draw(game);
  inventory_button_slot.draw(game);
  inventory_button_slot_icon.draw(game);
  skill_tree_button.draw(game);
  ability_up_arrow.draw(game);
  ability_down_arrow.draw(game);
  for (size_t i = 0; i < NUM_ABILITY_SLOT_ROWS; i++) {
    for (size_t j = 0; j < NUM_ABILITY_SLOTS_PER_ROW; j++) {
      auto &slot = ability_slots[i][j];
      slot.draw(game);

      if ((int)i == showing_ability_row_idx) {
        auto &ability = unit.ability_slots[i][j];
        auto ability_dst = slot.dst.get_xy();
        ability_dst.x += SLOT_OFFSET_X;
        ability_dst.y += SLOT_OFFSET_Y;
        ability.draw_at_dst(game, false, ability_dst);
      }
    }
  }
  ability_row_text.draw(game);
}