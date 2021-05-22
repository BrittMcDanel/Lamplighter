#include "alert.h"
#include "game.h"

Alert::Alert(Game &game, const char *_text_str, Vec2 _dst) {
  dst.set_xy(_dst.x, _dst.y);
  background = ExpandableSprite(game);
  text = Text(game, 10, FontColor::WhiteShadow, _text_str, _dst, 100,
              TextAlignment::Left);
  background.set_is_camera_rendered(false);
  text.is_camera_rendered = false;
  tweens = Tweens();
}

void Alert::update(Game &game) {
  tweens.update(game, dst);
  auto text_dims = text.measure_text(game, text.str);
  dst.w = text_dims.x + (BACKGROUND_PADDING * 2) - 2;
  dst.h = text_dims.y + BACKGROUND_PADDING * 2;
  // center alert
  dst.x = (game.engine.base_resolution.x / 2) - (dst.w / 2);
  text.dst.set_xy(dst.x + BACKGROUND_PADDING, dst.y + 12 + BACKGROUND_PADDING);
  background.update_layout(game, dst);
  text.update(game);
}

void Alert::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  background.draw(game);
  text.draw(game);
}

void Alert::show(Game &game) {
  is_hidden = false;
  auto start_rect = dst;
  // offscreen
  start_rect.y = game.engine.base_resolution.y - 90;
  auto target_rect = start_rect;
  target_rect.y = game.engine.base_resolution.y + 100;
  dst = start_rect;
  TweenCallback callback;
  callback.cb_type = TweenCallbackType::BattleAlert;
  tweens.tween_xys.push_back(TweenXY(
      start_rect, target_rect, game.engine.current_time, 500, 3000, callback,
      []() {}, []() {}));
}