#include "equip_window.h"
#include "game.h"
#include "utils_game.h"

EquipWindow::EquipWindow(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  auto background_w = 200;
  background = UISprite(
      game, image, Vec2(0, 24),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(0, 688, background_w, 336))),
      },
      100);
  inventory_background = UISprite(
      game, image, Vec2(game.engine.base_resolution.x - background_w, 24),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(0, 688, background_w, 336))),
      },
      100);
  equipment_slots = UIEquipmentSlots(game);
  auto right_start_dst =
      Vec2(inventory_background.dst.x + BACKGROUND_PADDING,
           inventory_background.dst.y +
               inventory_background.srcs.at(0).image_location.src.h -
               BACKGROUND_PADDING);
  funds_money_display = MoneyDisplay(
      game, Vec2(right_start_dst.x, right_start_dst.y - ITEM_DIM), "Funds");
  inventory_window =
      InventoryWindow(game, Vec2(right_start_dst.x, right_start_dst.y - 75));
}

void EquipWindow::update(Game &game, Unit &unit) {
  if (!is_hidden &&
      (is_mouse_over_dst(game, false, background.dst) ||
       is_mouse_over_dst(game, false, inventory_background.dst))) {
    game.ui.set_is_mouse_over_ui("equip window");
  }
  inventory_window.is_hidden = is_hidden;
  start_y = background.dst.y;
  start_y += background.dst.h - SLOT_DIM - BACKGROUND_PADDING;
  auto right_column_x = BACKGROUND_PADDING + 100;
  equipment_slots.primary_hand.dst.set_xy(BACKGROUND_PADDING, start_y);
  equipment_slots.secondary_hand.dst.set_xy(
      BACKGROUND_PADDING, start_y - (SLOT_DIM + SLOT_MARGIN_BOTTOM));
  equipment_slots.head.dst.set_xy(right_column_x, start_y);
  equipment_slots.armor.dst.set_xy(right_column_x,
                                   start_y - (SLOT_DIM + SLOT_MARGIN_BOTTOM));
  equipment_slots.boots.dst.set_xy(
      right_column_x, start_y - (SLOT_DIM + SLOT_MARGIN_BOTTOM) * 2);
  equipment_slots.update(game, unit);
  background.update(game);
  inventory_background.update(game);
  funds_money_display.coin.quantity = unit.coin.quantity;
  funds_money_display.update(game);
  inventory_window.update(game, unit.inventory, DropType::EquipItem, unit.guid);
}

void EquipWindow::draw(Game &game, Unit &unit) {
  if (is_hidden) {
    return;
  }
  background.draw(game);
  inventory_background.draw(game);
  funds_money_display.draw(game);
  equipment_slots.draw(game, unit);
  unit.sprite.draw_at_dst(game, false, Vec2(60, start_y - 30));
  inventory_window.draw(game, unit.inventory);
}