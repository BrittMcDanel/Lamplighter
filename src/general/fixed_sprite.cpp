#include "fixed_sprite.h"
#include "game.h"
#include <assert.h>

FixedSprite::FixedSprite() {
  image = Image();
  dst = Rect(0, 0, 0, 0);
  scaled_dst = Rect(0, 0, 0, 0);
  srcs = vector<SpriteSrc>();
  anim_speed = 100;
  spawn_time = 0;
}

FixedSprite::FixedSprite(Game &game, Image _image, Vec2 _dst,
                         vector<SpriteSrc> _srcs, Uint32 _anim_speed) {
  image = _image;
  dst = Rect(_dst.x, _dst.y, 0, 0);
  scaled_dst = Rect(_dst.x, _dst.y, 0, 0);
  srcs = _srcs;
  anim_speed = _anim_speed;
  spawn_time = game.engine.current_time;
}

void FixedSprite::update(Game &game) {
  // an empty FixedSprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  current_frame_idx = get_current_frame_idx(
      game.engine.current_time, spawn_time, srcs.size(), anim_speed);
  auto &current_src = srcs[current_frame_idx];
  scaled_dst = dst;
  if (is_camera_rendered) {
    set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                                game.engine.scale);
  } else {
    set_scaled_rect(scaled_dst, game.engine.scale);
  }
  current_src.update(scaled_dst);
}

void FixedSprite::draw(Game &game) {
  // an empty FixedSprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  if (is_hidden) {
    return;
  }
  // super super important to not push to the render queue things that don't
  // need to be rendered. Can reduce performance by a lot if you don't do this.
  // 80 by 80 tile map goes from 4500 fps to 2200 fps on -O3.
  // a FixedSprite dst of 0, 0 is equal to the  game_rect.x, game_rect.y
  // (bottom-left corner)
  if (scaled_dst.x < -scaled_dst.w || scaled_dst.x > game.engine.game_rect.w ||
      scaled_dst.y < -scaled_dst.h || scaled_dst.y > game.engine.game_rect.h) {
    return;
  }
  game.engine.set_active_image(image);
  auto &src = srcs[current_frame_idx];
  game.engine.push_to_render_buffer(src.vertices, 12, src.uvs, 12);
}

void FixedSprite::set_is_hidden(bool _is_hidden) { is_hidden = _is_hidden; }
