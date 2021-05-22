#include "portraits.h"
#include "game.h"
#include "unit.h"
#include "utils_game.h"
#include "utils_ui.h"

Portrait::Portrait(Game &game) {
  auto hp_bar_image = game.engine.get_image(ImageName::UI);
  background = get_portrait_background(game, Vec2(0, 0));
  hp_bar =
      FixedSprite(game, hp_bar_image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(hp_bar_image, Rect(0, 10, 1, 1))),
                  },
                  100);
  hp_bar.is_camera_rendered = false;
  for (size_t i = 0; i < 20; i++) {
    status_effect_slots.push_back(create_status_effect_slot(game));
  }
}

void Portrait::update(Game &game, Unit &unit) {
  auto hp_bar_w = background.dst.w - 6;
  hp_bar_w = (int)(unit.stats.hp.get_current_max_pct() * hp_bar_w);
  hp_bar.dst = Rect(background.dst.x + 3, background.dst.y + 3, hp_bar_w, 1);
  background.update(game);
  hp_bar.update(game);

  auto slot_start =
      Vec2(background.dst.x + background.dst.w + SLOT_MARGIN_RIGHT,
           background.dst.y + background.dst.h - STATUS_EFFECT_SLOT_DIM);
  auto slot_dst = slot_start;
  int columns = 7;
  int wrap_at_idx = columns - 1;
  for (size_t i = 0; i < unit.status_effects.size(); i++) {
    // only display if there are enough ui slots to show
    if (i > status_effect_slots.size() - 1) {
      break;
    }
    auto &slot = status_effect_slots.at(i);
    auto &status_effect = unit.status_effects[i];
    slot.dst.set_xy(slot_dst);
    slot.update(game);

    if (slot.input_events.is_mouse_over) {
      game.ui.set_is_mouse_over_ui("portraits - status effects");
      auto tooltip_dst = slot.dst;
      tooltip_dst.x += STATUS_EFFECT_SLOT_DIM + SLOT_MARGIN_BOTTOM;
      tooltip_dst.y += 1;
      game.ui.status_effect_tooltip.set_status_effect_and_dst(
          game, status_effect, tooltip_dst);
    }

    slot_dst.x += STATUS_EFFECT_SLOT_DIM + SLOT_MARGIN_RIGHT;
    if (i == (size_t)wrap_at_idx) {
      slot_dst.x = slot_start.x;
      slot_dst.y -= (STATUS_EFFECT_SLOT_DIM + SLOT_MARGIN_BOTTOM);
      wrap_at_idx += columns;
    }
  }

  if (background.input_events.is_mouse_over) {
    game.ui.set_is_mouse_over_ui("portraits");
  }
  if (background.input_events.is_click) {
    game.engine.camera.camera_mode = CameraMode::FollowPlayer;
  }
}

void Portrait::draw(Game &game, Unit &unit) {
  background.draw(game);
  auto portrait_dst = Vec2(background.dst.x + 4, background.dst.y + 6);
  draw_sprite_at_dst(game, unit.sprite.image, unit.sprite.portrait,
                     unit.sprite.spawn_time, unit.sprite.portrait_anim_speed,
                     false, portrait_dst);
  hp_bar.draw(game);

  for (size_t i = 0; i < unit.status_effects.size(); i++) {
    // only display if there are enough ui slots to show
    if (i > status_effect_slots.size() - 1) {
      break;
    }
    auto &slot = status_effect_slots.at(i);
    auto &status_effect = unit.status_effects[i];
    slot.draw(game);
    auto r = status_effect.portrait.srcs.at(0).image_location.src;
    draw_sprite_at_dst(
        game, status_effect.portrait.image, status_effect.portrait.srcs,
        status_effect.portrait.spawn_time, status_effect.portrait.anim_speed,
        false, slot.dst.get_xy() + Vec2(SLOT_OFFSET_X, SLOT_OFFSET_Y));
  }
}

Portraits::Portraits(Game &game) {
  portraits = vector<Portrait>();
  for (size_t i = 0; i < PLAYER_CONTROLLED_UNITS_SIZE; i++) {
    portraits.push_back(Portrait(game));
  }
}

void Portraits::update(Game &game) {
  auto &acting_unit = game.map.get_player_unit();
  auto &portrait = portraits.at(0);
  auto background_h = 32;
  portrait.background.dst.set_xy(BACKGROUND_PADDING,
                                 game.engine.base_resolution.y - background_h -
                                     BACKGROUND_PADDING);
  portrait.update(game, acting_unit);
}

void Portraits::draw(Game &game) {
  auto &acting_unit = game.map.get_player_unit();
  auto &portrait = portraits.at(0);
  portrait.draw(game, acting_unit);
}