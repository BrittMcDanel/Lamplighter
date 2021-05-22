#include "spritesheet.h"
#include "game.h"
#include <assert.h>

SpriteSheet::SpriteSheet() {
  image = Image();
  dst = Rect(0, 0, 0, 0);
  scaled_dst = Rect(0, 0, 0, 0);
  srcs = vector<SpriteSrc>();
  anim_speed = 100;
  spawn_time = 0;
}

SpriteSheet::SpriteSheet(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
                         Uint32 _anim_speed) {
  image = _image;
  dst = Rect(_dst.x, _dst.y, 0, 0);
  scaled_dst = Rect(_dst.x, _dst.y, 0, 0);
  srcs = _srcs;
  anim_speed = _anim_speed;
  spawn_time = game.engine.current_time;
}

void SpriteSheet::update(Game &game) {
  assert(srcs.size() != 0);
  current_frame_idx = get_current_frame_idx(game, srcs.size());
  auto &current_src = srcs[current_frame_idx];
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  scaled_dst = dst;
  // set_scaled_rect(scaled_dst, 4);
  // scaled_dst.x += game.engine.game_rect.x;
  // scaled_dst.y += game.engine.game_rect.y;
  current_src.update(scaled_dst);
}

void SpriteSheet::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  // super super important to not push to the render queue things that don't need
  // to be rendered. Can reduce performance by a lot if you don't do this.
  // 80 by 80 tile map goes from 4500 fps to 2200 fps on -O3.
  // a SpriteSheet dst of 0, 0 is equal to the  game_rect.x, game_rect.y (bottom-left corner)
  if (scaled_dst.x < -scaled_dst.w || scaled_dst.x > game.engine.game_rect.w ||
      scaled_dst.y < -scaled_dst.h || scaled_dst.y > game.engine.game_rect.h) {
    return;
  }
  game.engine.set_active_image(image);
  auto &src = srcs[current_frame_idx];
  game.engine.push_to_render_buffer(src.vertices, 12, src.uvs, 12);
}

int SpriteSheet::get_current_frame_idx(Game &game, int size) {
  auto alive_time = game.engine.current_time - spawn_time;
  auto current_frame = 0;
  if (size > 0) {
    current_frame = (alive_time / anim_speed) % size;
  }
  return current_frame;
}

void SpriteSheet::set_is_hidden(bool _is_hidden) { is_hidden = _is_hidden; }
