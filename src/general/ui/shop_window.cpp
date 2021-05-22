#include "ui/shop_window.h"
#include "game.h"
#include "utils_game.h"

ShopWindow::ShopWindow(Game &game) {
  buy_order_inventory = Inventory(game);
  sell_order_inventory = Inventory(game);
  auto image = game.engine.get_image(ImageName::UI);
  int background_w = 415;
  int background_h = 280;
  auto background_dst =
      Vec2((game.engine.base_resolution.x / 2) - (background_w / 2), 32);
  background = UISprite(
      game, image,
      Vec2((game.engine.base_resolution.x / 2) - (background_w / 2), 32),
      vector<SpriteSrc>{
          SpriteSrc(
              ImageLocation(image, Rect(208, 713, background_w, background_h))),
      },
      100);

  shop_description_text =
      Text(game, 10, FontColor::Black, "What would you like to buy?",
           Vec2(background_dst.x + BACKGROUND_PADDING,
                background_dst.y + background_h - BACKGROUND_PADDING),
           background_w, TextAlignment::Left);
  shop_description_text.is_camera_rendered = false;
  shop_description_text.set_draw_one_char_at_a_time_mode(game);

  // shop has two inventory windows regardless of view, one on the left
  // and one ont he right
  left_dst = background_dst;
  right_dst = background_dst;
  left_dst.x += BACKGROUND_PADDING;
  left_dst.y +=
      background_h - ITEM_DIM - SLOT_OFFSET_Y - BACKGROUND_PADDING - 40;
  right_dst.x = left_dst.x + 224;
  right_dst.y = left_dst.y;
  buy_merchant_inventory_window = InventoryWindow(game, left_dst);
  buy_merchant_order_window = InventoryWindow(game, right_dst);
  sell_player_inventory_window = InventoryWindow(game, left_dst);
  sell_merchant_order_window = InventoryWindow(game, right_dst);

  shop_mode_tabs =
      Tabs(game, Vec2(background.dst.x, background.dst.y + background_h - 1), 2,
           TabName::ShopBuy,
           vector<Tab>{
               Tab::create_default_tab(game, TabName::ShopBuy, "Buy"),
               Tab::create_default_tab(game, TabName::ShopSell, "Sell"),
           });

  close_button = ButtonIcon::create_close_button(game, Vec2(0, 0));
  auto close_button_w = close_button.background.srcs.at(0).image_location.src.w;
  auto close_button_h = close_button.background.srcs.at(0).image_location.src.h;
  close_button.background.dst.set_xy(background.dst.x + background_w -
                                         close_button_w,
                                     background.dst.y + background_h + 8);

  auto confirm_button_w = 72;
  confirm_button = ButtonText::create_default_button(
      game,
      Vec2(background.dst.x + background_w - confirm_button_w -
               BACKGROUND_PADDING,
           background.dst.y + BACKGROUND_PADDING),
      "Buy");

  funds_display =
      MoneyDisplay(game,
                   Vec2(right_dst.x, background.dst.y + background_h -
                                         BACKGROUND_PADDING - 20),
                   "Funds");
  total_display =
      MoneyDisplay(game, Vec2(right_dst.x, background.dst.y + 50), "Total");
}

void ShopWindow::update_always(Game &game, Unit &unit) {
  shop_mode_tabs.update(game);
  close_button.update(game);
}

void ShopWindow::update_when_open(Game &game, Unit &unit, Unit &seller) {
  /*if (!is_hidden &&
      (is_mouse_over_dst(game, false, background.dst) ||
       is_mouse_over_dst(game, false, close_button.background.dst) ||
       shop_mode_tabs.is_mouse_over(game))) {
    game.ui.is_mouse_over_ui = true;
  }*/
  // always disabling clicks outside of the shop window think it feels better
  game.ui.set_is_mouse_over_ui("shop window");
  background.update(game);
  shop_description_text.update(game);
  shop_mode_tabs.update(game);
  close_button.update(game);
  funds_display.coin.quantity = unit.coin.quantity;
  funds_display.update(game);
  total_display.update(game);
  buy_merchant_inventory_window.update(game, seller.inventory, DropType::None,
                                       seller.guid);
  buy_merchant_order_window.update(game, buy_order_inventory, DropType::None,
                                   unit.guid);
  sell_player_inventory_window.update(game, unit.inventory, DropType::None,
                                      unit.guid);
  sell_merchant_order_window.update(game, sell_order_inventory, DropType::None,
                                    unit.guid);
  confirm_button.update(game);

  // after shop_mode_tabs update to avoid a frame of nothing being drawn
  buy_merchant_inventory_window.is_hidden =
      is_hidden || shop_mode_tabs.active_tab_name != TabName::ShopBuy;
  buy_merchant_order_window.is_hidden =
      is_hidden || shop_mode_tabs.active_tab_name != TabName::ShopBuy;
  sell_player_inventory_window.is_hidden =
      is_hidden || shop_mode_tabs.active_tab_name != TabName::ShopSell;
  sell_merchant_order_window.is_hidden =
      is_hidden || shop_mode_tabs.active_tab_name != TabName::ShopSell;

  if (shop_mode_tabs.active_tab_name == TabName::ShopBuy) {
    int buy_order_total_cost = buy_order_inventory.get_total_cost();
    total_display.coin.quantity = buy_order_total_cost;
    confirm_button.text.str = "Buy";
    for (size_t i = 0; i < NUM_INVENTORY_SLOTS; i++) {
      auto &slot = buy_merchant_inventory_window.slots[i];
      auto &item = seller.inventory.items[i];
      auto has_enough_money =
          buy_order_total_cost + item.cost <= unit.coin.quantity;
      slot.input_events.is_disabled = !has_enough_money;
      if (slot.input_events.is_click && has_enough_money) {
        buy_order_inventory.add_item(item);
      }
    }
    for (size_t i = 0; i < NUM_INVENTORY_SLOTS; i++) {
      auto &slot = buy_merchant_order_window.slots[i];
      if (slot.input_events.is_click) {
        // make a copy of the item so that the player can remove one item
        // at a time.
        auto item_cpy = buy_order_inventory.items[i];
        item_cpy.quantity = 1;
        buy_order_inventory.remove_item(item_cpy);
      }
    }
  } else if (shop_mode_tabs.active_tab_name == TabName::ShopSell) {
    int sell_order_total_cost = sell_order_inventory.get_total_cost();
    total_display.coin.quantity = sell_order_total_cost;
    confirm_button.text.str = "Sell";
    for (size_t i = 0; i < NUM_INVENTORY_SLOTS; i++) {
      auto &slot = sell_player_inventory_window.slots[i];
      if (slot.input_events.is_click) {
        auto item_cpy = unit.inventory.items[i];
        sell_order_inventory.add_item(item_cpy);
        unit.inventory.remove_item(item_cpy);
      }
    }
    for (size_t i = 0; i < NUM_INVENTORY_SLOTS; i++) {
      auto &slot = sell_merchant_order_window.slots[i];
      if (slot.input_events.is_click) {
        auto item_cpy = sell_order_inventory.items[i];
        sell_order_inventory.remove_item(item_cpy);
        unit.inventory.add_item(item_cpy);
      }
    }
  }

  if ((shop_mode_tabs.active_tab_name == TabName::ShopBuy &&
       buy_order_inventory.size() == 0) ||
      (shop_mode_tabs.active_tab_name == TabName::ShopSell &&
       sell_order_inventory.size() == 0)) {
    confirm_button.background.input_events.is_disabled = true;
  } else {
    confirm_button.background.input_events.is_disabled = false;
  }

  if (confirm_button.background.input_events.is_click) {
    if (shop_mode_tabs.active_tab_name == TabName::ShopBuy) {
      auto total_cost = buy_order_inventory.get_total_cost();
      unit.coin.quantity -= total_cost;
      for (auto &item : buy_order_inventory.items) {
        unit.inventory.add_item(item);
      }
      buy_order_inventory.clear();
    } else if (shop_mode_tabs.active_tab_name == TabName::ShopSell) {
      auto total_cost = sell_order_inventory.get_total_cost();
      unit.coin.quantity += total_cost;
      // don't need to remove items from unit's inventory here,
      // they are removd when added to the sell order inventory
      sell_order_inventory.clear();
    }
  }

  if (shop_mode_tabs.active_tab_changed) {
    if (shop_mode_tabs.active_tab_name == TabName::ShopBuy) {
      shop_description_text.str = "What would you like to buy?";
    } else if (shop_mode_tabs.active_tab_name == TabName::ShopSell) {
      shop_description_text.str = "What would you like to sell?";
    }
    shop_description_text.start_draw_individual_chars(game);
  }

  if (close_button.background.input_events.is_click) {
    is_hidden = true;
    shop_mode_tabs.active_tab_name = TabName::ShopBuy;
    // clear buy order inventory
    buy_order_inventory.clear();
    sell_order_inventory.clear();
  }
}

void ShopWindow::draw(Game &game, Unit &unit, Unit &seller) {
  if (is_hidden) {
    return;
  }
  background.draw(game);
  shop_description_text.draw(game);
  shop_mode_tabs.draw(game);
  close_button.draw(game);
  funds_display.draw(game);
  total_display.draw(game);
  if (shop_mode_tabs.active_tab_name == TabName::ShopBuy) {
    buy_merchant_inventory_window.draw(game, seller.inventory);
    buy_merchant_order_window.draw(game, buy_order_inventory);
  } else if (shop_mode_tabs.active_tab_name == TabName::ShopSell) {
    sell_player_inventory_window.draw(game, unit.inventory);
    sell_merchant_order_window.draw(game, sell_order_inventory);
  } else {
    cout << "ShopWindow::draw - unexpected tab name found "
         << (int)shop_mode_tabs.active_tab_name << "\n";
    abort();
  }
  confirm_button.draw(game);
}

void ShopWindow::show_shop(boost::uuids::uuid _seller_unit_guid) {
  is_hidden = false;
  seller_unit_guid = _seller_unit_guid;
}