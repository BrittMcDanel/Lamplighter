#include "unit_sprite.h"
#include "game.h"
#include "utils_game.h"
#include <assert.h>

UnitSprite::UnitSprite(Game &game) {
  image = game.engine.get_image(ImageName::Units);
  dir = Dir::Down;
  anim_state = UnitAnimState::Idle;
  current_src = SpriteSrc();
  hitbox_dims_input_events = Vec2(20, 40);
  // used in move grid, it is /= MOVE_GRID_TILE_SIZE to get the move_grid_hitbox
  // dims
  hitbox_dims = Vec2(20, 40);
  dst = Rect(0, 0, 0, 0);
  scaled_dst = Rect(0, 0, 0, 0);
  tile_point_hit_box = Rect(0, 0, 0, 0);
  anim_state = UnitAnimState::Idle;
  portrait = vector<SpriteSrc>{
      SpriteSrc(ImageLocation(image, Rect(140, 41, 20, 22))),
  };
  idle_down = vector<SpriteSrc>{
      SpriteSrc(ImageLocation(image, Rect(437, 0, 15, 24))),
      SpriteSrc(ImageLocation(image, Rect(453, 0, 15, 24))),
  };
  walk_down =
      vector<SpriteSrc>{SpriteSrc(ImageLocation(image, Rect(0, 0, 0, 0)))};
  attack_down =
      vector<SpriteSrc>{SpriteSrc(ImageLocation(image, Rect(0, 0, 0, 0)))};
  cast_down =
      vector<SpriteSrc>{SpriteSrc(ImageLocation(image, Rect(0, 0, 0, 0)))};
  hit_down =
      vector<SpriteSrc>{SpriteSrc(ImageLocation(image, Rect(0, 0, 0, 0)))};
  dead_down = vector<SpriteSrc>{
      SpriteSrc(ImageLocation(image, Rect(360, 0, 20, 40))),
      SpriteSrc(ImageLocation(image, Rect(380, 0, 20, 40))),
      SpriteSrc(ImageLocation(image, Rect(360, 0, 20, 40))),
      SpriteSrc(ImageLocation(image, Rect(400, 0, 20, 40))),
  };
  idle_anim_speed = 150;
  walk_anim_speed = 150;
  attack_anim_speed = 150;
  cast_anim_speed = 150;
  hit_anim_speed = 150;
  dead_anim_speed = 150;
  portrait_anim_speed = 150;
  spawn_time = game.engine.current_time;
  is_camera_rendered = true;
  is_hidden = false;
  input_events = InputEvents();
  tweens = Tweens();
}

void UnitSprite::update(Game &game) {
  // update hitbox
  hitbox_dims_move_grid.x = hitbox_dims.x / MOVE_GRID_TILE_SIZE;
  hitbox_dims_move_grid.y = hitbox_dims.y / MOVE_GRID_TILE_SIZE;
  tile_point_hit_box.w = hitbox_dims_move_grid.x;
  tile_point_hit_box.h = hitbox_dims_move_grid.y;

  // src and dst
  set_current_src(game);
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  scaled_dst = dst;
  set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                              game.engine.scale);
  // scaled_dst.x += game.engine.game_rect.x;
  // scaled_dst.y += game.engine.game_rect.y;
  current_src.update(scaled_dst);

  // updates
  input_events.update(game, is_mouse_over_dst(game, true, dst));
  // map transition can be called from here as a move tween can trigger it
  tweens.update(game, dst);
}

void UnitSprite::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  // super super important to not push to the render queue things that don't
  // need to be rendered. Can reduce performance by a lot if you don't do this.
  // 80 by 80 tile map goes from 4500 fps to 2200 fps on -O3.
  // a sprite dst of 0, 0 is equal to the  game_rect.x, game_rect.y (bottom-left
  // corner)
  if (scaled_dst.x < -scaled_dst.w || scaled_dst.x > game.engine.game_rect.w ||
      scaled_dst.y < -scaled_dst.h || scaled_dst.y > game.engine.game_rect.h) {
    return;
  }
  game.engine.set_active_image(image);
  game.engine.push_to_render_buffer(current_src.vertices, 12, current_src.uvs,
                                    12);
}

void UnitSprite::draw_at_dst(Game &game, bool _is_camera_rendered,
                             Vec2 _dst_xy) {
  draw_sprite_at_dst(game, image, current_src, _is_camera_rendered, _dst_xy);
}

void UnitSprite::set_src_from_current_frame(Game &game, vector<SpriteSrc> &srcs,
                                            Uint32 _anim_speed) {
  if (srcs.size() > 0) {
    auto alive_time = game.engine.current_time - spawn_time;
    auto current_frame = (alive_time / _anim_speed) % srcs.size();
    current_src = srcs[current_frame];
  }
}

void UnitSprite::set_current_src(Game &game) {
  switch (anim_state) {
  case UnitAnimState::Idle: {
    set_src_from_current_frame(game, idle_down, idle_anim_speed);
    break;
  }
  case UnitAnimState::Walk: {
    set_src_from_current_frame(game, walk_down, walk_anim_speed);
    break;
  }
  case UnitAnimState::Attack: {
    set_src_from_current_frame(game, attack_down, attack_anim_speed);
    break;
  }
  case UnitAnimState::Cast: {
    set_src_from_current_frame(game, cast_down, cast_anim_speed);
    break;
  }
  case UnitAnimState::Hit: {
    set_src_from_current_frame(game, hit_down, hit_anim_speed);
    break;
  }
  case UnitAnimState::Dead: {
    set_src_from_current_frame(game, dead_down, dead_anim_speed);
    break;
  }
  default: {
    printf("UnitSprite::set_current_src. anim_state not handled: %d\n",
           (int)anim_state);
    abort();
  }
  }
}
