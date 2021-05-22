#ifndef UNIT_SPRITE_H
#define UNIT_SPRITE_H
#include <SDL.h>

#include "engine.h"
#include "input_events.h"
#include "tween.h"
#include "utils.h"

class Game;

class UnitSprite {
public:
  Image image;
  Dir dir = Dir::Down;
  UnitAnimState anim_state;
  SpriteSrc current_src;
  Rect dst;
  Rect scaled_dst;
  Vec2 hitbox_dims_input_events;
  Vec2 hitbox_dims;
  Vec2 hitbox_dims_move_grid;
  Rect tile_point_hit_box;
  vector<SpriteSrc> portrait;
  vector<SpriteSrc> idle_down;
  vector<SpriteSrc> walk_down;
  vector<SpriteSrc> attack_down;
  vector<SpriteSrc> cast_down;
  vector<SpriteSrc> hit_down;
  vector<SpriteSrc> dead_down;
  Uint32 idle_anim_speed = 1;
  Uint32 walk_anim_speed = 1;
  Uint32 attack_anim_speed = 1;
  Uint32 cast_anim_speed = 1;
  Uint32 hit_anim_speed = 1;
  Uint32 dead_anim_speed = 1;
  Uint32 portrait_anim_speed = 1;
  Uint32 spawn_time;
  bool is_camera_rendered;
  bool is_hidden;
  bool is_disabled;
  bool can_receive_input_events;
  InputEvents input_events;
  Tweens tweens;
  UnitSprite() = default;
  UnitSprite(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void draw_at_dst(Game &game, bool _is_camera_rendered, Vec2 _dst_xy);
  void set_current_src(Game &game);
  void set_scaled_screen_dst(Game &game);
  void set_src_from_current_frame(Game &game, vector<SpriteSrc> &srcs,
                                  Uint32 _anim_speed);
};

#endif // UNIT_SPRITE_H
