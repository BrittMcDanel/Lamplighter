#include "turn_order_display.h"
#include "battle.h"
#include "game.h"
#include "utils_game.h"

TurnOrderDisplay::TurnOrderDisplay(Game &game) {
  auto hp_bar_image = game.engine.get_image(ImageName::UI);
  for (size_t i = 0; i < NUM_BATTLE_TURN_ORDER_SLOTS; i++) {
    slots.push_back(create_turn_order_slot(game));
    charge_slots.push_back(create_turn_order_charge_slot(game));
    auto hp_bar = FixedSprite(
        game, hp_bar_image, Vec2(0, 0),
        vector<SpriteSrc>{
            SpriteSrc(ImageLocation(hp_bar_image, Rect(0, 10, 1, 1))),
        },
        100);
    hp_bar.is_camera_rendered = false;
    hp_bars.push_back(hp_bar);
  }
}

void TurnOrderDisplay::update(Game &game, Battle &battle) {
  auto slot_w = SLOT_DIM;
  auto slot_h = 28;
  auto charge_slot_h = 59;
  auto total_w = NUM_BATTLE_TURN_ORDER_SLOTS * (slot_w + SLOT_MARGIN_RIGHT);
  Vec2 slot_start =
      Vec2((game.engine.base_resolution.x / 2) - (total_w / 2),
           game.engine.base_resolution.y - slot_h - BACKGROUND_PADDING);
  for (size_t i = 0; i < NUM_BATTLE_TURN_ORDER_SLOTS; i++) {
    auto &slot = slots[i];
    auto &charge_slot = charge_slots[i];
    slot.dst.set_xy(slot_start);
    auto charge_slot_dst =
        Vec2(slot_start.x, game.engine.base_resolution.y - charge_slot_h -
                               BACKGROUND_PADDING);
    charge_slot.dst.set_xy(charge_slot_dst);
    slot_start.x += slot_w + SLOT_MARGIN_RIGHT;
  }
  for (auto &unit_guid : battle.unit_guids) {
    auto &unit = game.map.unit_dict[unit_guid];
    unit.unit_ui_after_unit.turn_order_arrow.is_hidden = true;
  }
  game.map.turn_order_ability_target.is_hidden = true;
  update_layout(game, battle, false);
}

void TurnOrderDisplay::draw(Game &game, Battle &battle) {
  update_layout(game, battle, true);
}

void TurnOrderDisplay::update_layout(Game &game, Battle &battle, bool _draw) {
  for (size_t i = 0; i < NUM_BATTLE_TURN_ORDER_SLOTS; i++) {
    auto &slot = slots[i];
    auto &charge_slot = charge_slots[i];
    slot.is_hidden = true;
    charge_slot.is_hidden = true;
  }

  if (battle.unit_guids.size() > 0) {
    auto turn_idx = 0;
    int slot_idx = 0;
    int battle_unit_guid_idx = 0;
    for (size_t i = 0; i < battle.unit_guids.size(); i++) {
      if (battle.unit_guids[i] == battle.acting_unit_guid) {
        battle_unit_guid_idx = (int)i;
        break;
      }
    }
    for (size_t i = 0; i < NUM_BATTLE_TURN_ORDER_SLOTS; i++) {
      auto &slot = slots[slot_idx];
      auto &hp_bar = hp_bars[slot_idx];
      slot.is_hidden = false;
      auto unit_guid = battle.unit_guids.at(battle_unit_guid_idx);
      auto &unit = game.map.unit_dict[unit_guid];
      if (!_draw) {
        slot.update(game);
        auto hp_bar_w = slot.dst.w - 2;
        hp_bar_w = (int)(unit.stats.hp.get_current_max_pct() * hp_bar_w);
        hp_bar.dst = Rect(slot.dst.x + 1, slot.dst.y + 1, hp_bar_w, 1);
        hp_bar.update(game);
        if (slot.input_events.is_mouse_over) {
          game.ui.set_is_mouse_over_ui("turn order slot");
          if (unit.unit_ui_after_unit.turn_order_arrow.is_hidden) {
            unit.unit_ui_after_unit.turn_order_arrow.is_hidden = false;
          }
        }
      }
      auto portrait_dst = slots[slot_idx].dst.get_xy();
      portrait_dst.x += SLOT_OFFSET_X;
      portrait_dst.y += 4;
      if (_draw) {
        slot.draw(game);
        draw_sprite_at_dst(game, unit.sprite.image, unit.sprite.portrait,
                           unit.sprite.spawn_time,
                           unit.sprite.portrait_anim_speed, false,
                           portrait_dst);
        hp_bar.draw(game);
      }
      battle_unit_guid_idx += 1;
      if (battle_unit_guid_idx > (int)battle.unit_guids.size() - 1) {
        battle_unit_guid_idx = 0;
      }
      for (auto &charging_action : battle.charging_actions) {
        if (charging_action.ability.stats.cast_time.current - 1 == turn_idx) {
          slot_idx += 1;
          if (slot_idx > NUM_BATTLE_TURN_ORDER_SLOTS - 1) {
            return;
          }
          auto &charge_slot = charge_slots[slot_idx];
          auto &hp_bar_charge_slot = hp_bars[slot_idx];
          charge_slot.is_hidden = false;
          auto &acting_unit =
              game.map.unit_dict[charging_action.acting_unit_guid];
          if (!_draw) {
            charge_slot.update(game);
            auto hp_bar_w = charge_slot.dst.w - 2;
            hp_bar_w = (int)(unit.stats.hp.get_current_max_pct() * hp_bar_w);
            hp_bar_charge_slot.dst =
                Rect(charge_slot.dst.x + 1, charge_slot.dst.y + 1, hp_bar_w, 1);
            hp_bar_charge_slot.update(game);
            if (charge_slot.input_events.is_mouse_over) {
              game.ui.set_is_mouse_over_ui("turn order charge slot");
              // show ability tooltip for charging abilities
              auto tooltip_dst = charge_slot.dst;
              tooltip_dst.x += SLOT_DIM + 1;
              tooltip_dst.y -= 2;
              game.ui.ability_tooltip.set_ability_and_dst(
                  game, charging_action.ability, tooltip_dst);
              if (acting_unit.unit_ui_after_unit.turn_order_arrow.is_hidden) {
                acting_unit.unit_ui_after_unit.turn_order_arrow.is_hidden =
                    false;
              }
              if (charging_action.ability.stats.aoe.current != 1) {
                game.map.turn_order_ability_target =
                    game.ability_targets.get_target(charging_action.ability);
                game.map.turn_order_ability_target.set_dst(
                    game, charging_action.ability_target_dst);
                game.map.turn_order_ability_target.update(game);
                game.map.turn_order_ability_target.is_hidden = false;
              } else {
                GAME_ASSERT(charging_action.receiving_unit_guids.size() > 0);
                GAME_ASSERT(game.map.unit_dict.contains(
                    charging_action.receiving_unit_guids.at(0)));
                auto &receiving_unit =
                    game.map
                        .unit_dict[charging_action.receiving_unit_guids.at(0)];
                receiving_unit.unit_ui_after_unit.crosshairs.set_is_hidden(
                    false);
              }
            }
          }
          if (_draw) {
            charge_slot.draw(game);
            auto slot_dst = slots[slot_idx].dst;
            auto ability_portrait_dst = slot_dst.get_xy();
            ability_portrait_dst.x += SLOT_OFFSET_X;
            ability_portrait_dst.y += 4;
            portrait_dst = charge_slot.dst.get_xy();
            portrait_dst.x += SLOT_OFFSET_X;
            portrait_dst.y += 4;
            charging_action.ability.draw_at_dst(game, false,
                                                ability_portrait_dst);
            draw_sprite_at_dst(
                game, acting_unit.sprite.image, acting_unit.sprite.portrait,
                acting_unit.sprite.spawn_time,
                acting_unit.sprite.portrait_anim_speed, false, portrait_dst);
            hp_bar_charge_slot.draw(game);
          }
        }
      }
      slot_idx += 1;
      turn_idx += 1;

      if (slot_idx > NUM_BATTLE_TURN_ORDER_SLOTS - 1) {
        return;
      }
    }
  }
}