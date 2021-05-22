#include "inventory_window.h"
#include "game.h"
#include "inventory.h"
#include "unit.h"
#include "utils_game.h"

InventoryWindow::InventoryWindow(Game &game, Vec2 _dst) {
  dst = _dst;
  slots = vector<UISprite>();
  for (int i = 0; i < NUM_INVENTORY_SLOTS; i++) {
    slots.push_back(create_slot(game));
  }
}

// unit_guid is passed in because its needed for some drop callbacks
// but its not needed for other callbacks so its a little ugly.
void InventoryWindow::update(Game &game, Inventory &inventory,
                             DropType _drop_type,
                             boost::uuids::uuid unit_guid) {
  Vec2 start_dst = dst;
  Vec2 slot_dst = dst;
  int columns = 7;
  int wrap_at_idx = columns - 1;
  for (size_t i = 0; i < slots.size(); i++) {
    auto &slot = slots[i];
    slot.dst.set_xy(slot_dst.x, slot_dst.y);
    assert(slot.srcs.size() > 0);
    slot_dst.x += SLOT_DIM + SLOT_MARGIN_RIGHT;
    if ((int)i == wrap_at_idx) {
      slot_dst.x = start_dst.x;
      slot_dst.y -= (SLOT_DIM + SLOT_MARGIN_BOTTOM);
      wrap_at_idx += columns;
    }
    slot.is_hidden = is_hidden;
    slot.update(game);
    // prevent drag and drop if the inventory window is hidden
    if (!is_hidden && i < inventory.items.size()) {
      auto &item = inventory.items[i];
      if (slot.input_events.is_mouse_over) {
        auto tooltip_dst = slot.dst;
        tooltip_dst.x += SLOT_DIM + 1;
        tooltip_dst.y += 1;
        game.ui.item_tooltip.set_item_and_dst(game, item, tooltip_dst);
      }
      if (!game.ui.drag_ghost.is_dragging &&
          slot.input_events.was_mouse_down_when_mouse_over &&
          game.engine.mouse_point_game_rect_scaled !=
              slot.input_events.mouse_dst_when_mouse_down) {
        // set the drop callback data and then start the drag to visually
        // show the item being dragged by the mouse.
        switch (_drop_type) {
        // drop type of none means no drag and drop for this inventory window
        case DropType::None: {
          break;
        }
        case DropType::EquipItem: {
          game.ui.drag_ghost.drop_callback.set_as_equip_item(item, unit_guid,
                                                             (int)i);
          game.ui.drag_ghost.start_drag(item);
          break;
        }
        // currently doing clicks instead of drag and drop for items, add back
        // later if needed
        /*case DropType::ShopBuyItem: {
          DropCallback drop_callback = DropCallback();
          drop_callback.set_as_shop_buy_item(item, unit_guid);
          game.ui.drag_ghost.start_drag(item, drop_callback);
          break;
        }*/
        default: {
          cout << "InventoryWindow::update, passed in _drop_type not handled "
               << (int)_drop_type << "\n";
          abort();
          break;
        }
        }
      }
    }
  }
}

void InventoryWindow::draw(Game &game, Inventory &inventory) {
  if (is_hidden) {
    return;
  }
  for (size_t i = 0; i < slots.size(); i++) {
    auto &slot = slots[i];
    slot.draw(game);
    if (i < inventory.items.size()) {
      auto &item = inventory.items[i];
      auto item_dst = slot.dst.get_xy();
      item_dst.x += SLOT_OFFSET_X;
      item_dst.y += SLOT_OFFSET_Y;
      item.draw_at_dst(game, false, item_dst);
    }
  }
}