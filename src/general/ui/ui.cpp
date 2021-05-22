#include "ui/ui.h"
#include "game.h"
#include "utils_game.h"

UI::UI(Game &game) {
  bottom_navbar = BottomNavBar(game);
  item_pickup_list = ItemPickupList();
  equip_window = EquipWindow(game);
  skill_tree_window = SkillTreeWindow(game);
  shop_window = ShopWindow(game);
  action_point_display = ActionPointDisplay(game);
  turn_order_display = TurnOrderDisplay(game);
  player_portraits = Portraits(game);
  battle_start_alert = Alert(game, "Battle Start",
                             Vec2(300, game.engine.base_resolution.y - 30));
  item_tooltip = ItemTooltip(game);
  ability_tooltip = AbilityTooltip(game);
  status_effect_tooltip = StatusEffectTooltip(game);
}

void UI::update(Game &game) {
  is_mouse_over_ui = false;
  // always set tooltips to hidden at the beginning of each ui update
  // if they receive mouse over events they will be shown, I think this is
  // easier.
  item_tooltip.is_hidden = true;
  ability_tooltip.is_hidden = true;
  status_effect_tooltip.is_hidden = true;
  auto &player_unit = game.map.get_player_unit();
  bottom_navbar.update(game, player_unit);
  item_pickup_list.update(game);
  equip_window.update(game, player_unit);
  skill_tree_window.update(game, player_unit);
  if (!shop_window.is_hidden &&
      game.map.unit_dict.contains(shop_window.seller_unit_guid)) {
    auto &seller_unit = game.map.unit_dict[shop_window.seller_unit_guid];
    shop_window.update_when_open(game, player_unit, seller_unit);
  } else {
    shop_window.update_always(game, player_unit);
  }
  player_portraits.update(game);
  if (player_unit.in_battle) {
    GAME_ASSERT(game.map.battle_dict.contains(player_unit.battle_guid));
    auto &battle = game.map.battle_dict[player_unit.battle_guid];
    action_point_display.update(game, player_unit.stats.action_points.current,
                                player_unit.stats.action_points.max);
    turn_order_display.update(game, battle);
  }
  battle_start_alert.update(game);
  item_tooltip.update(game);
  ability_tooltip.update(game);
  status_effect_tooltip.update(game);
  drag_ghost.update(game);
}

void UI::draw(Game &game) {
  auto &player_unit = game.map.get_player_unit();
  if (player_unit.in_battle) {
    GAME_ASSERT(game.map.battle_dict.contains(player_unit.battle_guid));
    auto &battle = game.map.battle_dict[player_unit.battle_guid];
    action_point_display.draw(game);
    turn_order_display.draw(game, battle);
  }
  bottom_navbar.draw(game, player_unit);
  player_portraits.draw(game);
  item_pickup_list.draw(game);
  equip_window.draw(game, player_unit);
  skill_tree_window.draw(game, player_unit);
  if (!shop_window.is_hidden &&
      game.map.unit_dict.contains(shop_window.seller_unit_guid)) {
    auto &seller_unit = game.map.unit_dict[shop_window.seller_unit_guid];
    shop_window.draw(game, player_unit, seller_unit);
  }
  battle_start_alert.draw(game);
  item_tooltip.draw(game);
  ability_tooltip.draw(game);
  status_effect_tooltip.draw(game);
  drag_ghost.draw(game);
}

void DropCallback::set_as_equip_item(Item &_item, boost::uuids::uuid _unit_guid,
                                     int _item_idx) {
  drop_type = DropType::EquipItem;
  if (item.item_name != _item.item_name) {
    item = _item;
  }
  // the item passed in can have quantity > 1, but you only equip one item at a
  // time. set the newly copied item's quantity to 1 to make sure only 1 item is
  // equipped and removed from the inventory when equipped.
  item.quantity = 1;
  unit_guid = _unit_guid;
  item_idx = _item_idx;
}

void DropCallback::set_as_unequip_item(Item &_item,
                                       boost::uuids::uuid _unit_guid) {
  drop_type = DropType::UnequipItem;
  if (item.item_name != _item.item_name) {
    item = _item;
  }
  // should already be 1, but setting it again anyway
  item.quantity = 1;
  unit_guid = _unit_guid;
}

void DropCallback::set_as_shop_buy_item(Item &_item,
                                        boost::uuids::uuid _merchant_guid) {
  drop_type = DropType::ShopBuyItem;
  item = _item;
  merchant_guid = _merchant_guid;
}

void DropCallback::set_as_equip_ability(const Ability &_ability,
                                        boost::uuids::uuid _unit_guid) {
  drop_type = DropType::EquipAbility;
  ability = _ability;
  unit_guid = _unit_guid;
}

void DropCallback::set_as_bottom_navbar_ability_swap(
    boost::uuids::uuid _unit_guid, int _ability_idx) {
  drop_type = DropType::BottomNavbarAbilitySwap;
  unit_guid = _unit_guid;
  ability_idx = _ability_idx;
}

void on_drop_equip(Game &game, Unit &unit, Item &item,
                   DropCallback &drop_callback) {
  unit.inventory.add_item(item);
  item = drop_callback.item;
  unit.inventory.dec_quantity_at_idx(drop_callback.item_idx,
                                     drop_callback.item.quantity);
}

void DragGhost::handle_on_drop(Game &game) {
  switch (drop_callback.drop_type) {
  case DropType::None: {
    cout << "UI::handle_on_drop. drop type is None.\n";
    abort();
    break;
  }
  case DropType::EquipItem: {
    GAME_ASSERT(game.map.unit_dict.contains(drop_callback.unit_guid));
    auto &unit = game.map.unit_dict[drop_callback.unit_guid];
    // drop_callback's item's quantity is set to 1 when creating
    // the callback, so only one piece of equipment is removed in
    // unit.inventory.remove_item
    if (is_mouse_over_dst(
            game, false,
            game.ui.equip_window.equipment_slots.primary_hand.dst) &&
        drop_callback.item.item_type == ItemType::PrimaryHand) {
      on_drop_equip(game, unit, unit.equipment.primary_hand, drop_callback);
    } else if (is_mouse_over_dst(
                   game, false,
                   game.ui.equip_window.equipment_slots.secondary_hand.dst) &&
               drop_callback.item.item_type == ItemType::SecondaryHand) {
      on_drop_equip(game, unit, unit.equipment.secondary_hand, drop_callback);
    } else if (is_mouse_over_dst(
                   game, false,
                   game.ui.equip_window.equipment_slots.armor.dst) &&
               drop_callback.item.item_type == ItemType::Armor) {
      on_drop_equip(game, unit, unit.equipment.armor, drop_callback);
    } else if (is_mouse_over_dst(
                   game, false,
                   game.ui.equip_window.equipment_slots.head.dst) &&
               drop_callback.item.item_type == ItemType::Head) {
      on_drop_equip(game, unit, unit.equipment.head, drop_callback);
    } else if (is_mouse_over_dst(
                   game, false,
                   game.ui.equip_window.equipment_slots.boots.dst) &&
               drop_callback.item.item_type == ItemType::Boots) {
      on_drop_equip(game, unit, unit.equipment.boots, drop_callback);
    } else {
      for (size_t i = 0; i < game.ui.equip_window.inventory_window.slots.size();
           i++) {
        auto &slot = game.ui.equip_window.inventory_window.slots[i];
        if (is_mouse_over_dst(game, false, slot.dst)) {
          unit.inventory.swap(drop_callback.item_idx, (int)i);
          break;
        }
      }
    }
    break;
  }
  case DropType::UnequipItem: {
    GAME_ASSERT(game.map.unit_dict.contains(drop_callback.unit_guid));
    auto &unit = game.map.unit_dict[drop_callback.unit_guid];
    for (size_t i = 0; i < game.ui.equip_window.inventory_window.slots.size();
         i++) {
      auto &slot = game.ui.equip_window.inventory_window.slots[i];
      if (is_mouse_over_dst(game, false, slot.dst)) {
        unit.inventory.add_item_to_idx(drop_callback.item, (int)i);
        if (drop_callback.item.item_type == ItemType::PrimaryHand) {
          unit.equipment.primary_hand = Item();
        } else if (drop_callback.item.item_type == ItemType::SecondaryHand) {
          unit.equipment.secondary_hand = Item();
        } else if (drop_callback.item.item_type == ItemType::Armor) {
          unit.equipment.armor = Item();
        } else if (drop_callback.item.item_type == ItemType::Head) {
          unit.equipment.head = Item();
        } else if (drop_callback.item.item_type == ItemType::Boots) {
          unit.equipment.boots = Item();
        }
        break;
      }
    }
    break;
  }
  case DropType::ShopBuyItem: {
    GAME_ASSERT(game.map.unit_dict.contains(drop_callback.merchant_guid));
    auto &merchant = game.map.unit_dict[drop_callback.merchant_guid];
    for (auto &slot : game.ui.shop_window.buy_merchant_order_window.slots) {
      if (is_mouse_over_dst(game, false, slot.dst)) {
        game.ui.shop_window.buy_order_inventory.add_item(drop_callback.item);
        break;
      }
    }
    break;
  }
  case DropType::EquipAbility: {
    GAME_ASSERT(game.map.unit_dict.contains(drop_callback.unit_guid));
    auto &unit = game.map.unit_dict[drop_callback.unit_guid];
    for (size_t i = 0;
         i < game.ui.bottom_navbar
                 .ability_slots[game.ui.bottom_navbar.showing_ability_row_idx]
                 .size();
         i++) {
      auto &slot =
          game.ui.bottom_navbar
              .ability_slots[game.ui.bottom_navbar.showing_ability_row_idx][i];
      if (is_mouse_over_dst(game, false, slot.dst)) {
        unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx][i] =
            drop_callback.ability;
        break;
      }
    }
    break;
  }
  case DropType::BottomNavbarAbilitySwap: {
    GAME_ASSERT(game.map.unit_dict.contains(drop_callback.unit_guid));
    GAME_ASSERT(drop_callback.ability_idx >= 0 &&
                drop_callback.ability_idx <= NUM_ABILITY_SLOTS_PER_ROW - 1);
    auto &unit = game.map.unit_dict[drop_callback.unit_guid];
    auto dropped_on_slot = false;
    for (size_t i = 0;
         i < game.ui.bottom_navbar
                 .ability_slots[game.ui.bottom_navbar.showing_ability_row_idx]
                 .size();
         i++) {
      auto &slot =
          game.ui.bottom_navbar
              .ability_slots[game.ui.bottom_navbar.showing_ability_row_idx][i];
      if (is_mouse_over_dst(game, false, slot.dst)) {
        dropped_on_slot = true;
        auto temp =
            unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx]
                .at(drop_callback.ability_idx);
        unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx]
                          [drop_callback.ability_idx] =
            unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx]
                              [i];
        unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx][i] =
            temp;
        break;
      }
    }
    if (!dropped_on_slot) {
      unit.ability_slots[game.ui.bottom_navbar.showing_ability_row_idx].at(
          drop_callback.ability_idx) = Ability();
    }
    break;
  }
  default: {
    cout << "UI::handle_on_drop. drop type not handled "
         << (int)drop_callback.drop_type << ".\n";
    abort();
    break;
  }
  }
}

void DragGhost::start_drag(UISprite &_drag_ghost) {
  GAME_ASSERT(_drag_ghost.image.image_name != ImageName::None);
  drag_ghost_type = DragGhostType::UISprite;
  is_dragging = true;
  drag_ghost_ui_sprite = _drag_ghost;
}

void DragGhost::start_drag(TweenableSprite &_drag_ghost) {
  GAME_ASSERT(_drag_ghost.image.image_name != ImageName::None);
  drag_ghost_type = DragGhostType::TweenableSprite;
  is_dragging = true;
  drag_ghost_tweenable_sprite = _drag_ghost;
}

void DragGhost::start_drag(Item &_drag_ghost) {
  if (_drag_ghost.item_name == ItemName::None) {
    return;
  }
  drag_ghost_type = DragGhostType::Item;
  is_dragging = true;
  if (drag_ghost_item.item_name != _drag_ghost.item_name) {
    drag_ghost_item = _drag_ghost;
  }
  drag_ghost_item.quantity = _drag_ghost.quantity;
}

void DragGhost::update(Game &game) {
  if (is_dragging) {
    if (game.engine.is_mouse_up) {
      is_dragging = false;
      handle_on_drop(game);
    }
  }
}

void DragGhost::draw(Game &game) {
  if (is_dragging) {
    auto mouse_pos = game.engine.mouse_point_game_rect_scaled;

    switch (drag_ghost_type) {
    case DragGhostType::None: {
      cout << "DragGhost::draw - drag_ghost_type is none.";
      abort();
      break;
    }
    case DragGhostType::UISprite: {
      draw_sprite_at_dst(game, drag_ghost_ui_sprite.image,
                         drag_ghost_ui_sprite.srcs,
                         drag_ghost_ui_sprite.spawn_time,
                         drag_ghost_ui_sprite.anim_speed, false, mouse_pos);
      break;
    }
    case DragGhostType::TweenableSprite: {
      draw_sprite_at_dst(game, drag_ghost_tweenable_sprite.image,
                         drag_ghost_tweenable_sprite.srcs,
                         drag_ghost_tweenable_sprite.spawn_time,
                         drag_ghost_tweenable_sprite.anim_speed, false,
                         mouse_pos);
      break;
    }
    case DragGhostType::Item: {
      drag_ghost_item.draw_at_dst(game, false, mouse_pos);
      break;
    }
    default: {
      cout << "DragGhost::draw - drag_ghost_type is not handled "
           << (int)drag_ghost_type;
      abort();
      break;
    }
    }
  }
}

void UI::set_is_mouse_over_ui(const char *who_set_it) {
  is_mouse_over_ui = true;
  if (DEBUG_WHO_SET_IS_MOUSE_OVER_UI) {
    cout << "UI mouse over set by: " << who_set_it << "\n";
  }
}
