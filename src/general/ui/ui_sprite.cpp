#include "ui/ui_sprite.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include "utils_game.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

UISprite::UISprite(Game &game, Image _image, Vec2 _dst, vector<SpriteSrc> _srcs,
                   Uint32 _anim_speed, vector<SpriteSrc> _hover_srcs,
                   Uint32 _hover_anim_speed, vector<SpriteSrc> _disabled_srcs,
                   Uint32 _disabled_anim_speed, vector<SpriteSrc> _active_srcs,
                   Uint32 _active_anim_speed, vector<SpriteSrc> _focused_srcs,
                   Uint32 _focused_anim_speed) {
  image = _image;
  dst = Rect(_dst.x, _dst.y, 0, 0);
  scaled_dst = Rect(_dst.x, _dst.y, 0, 0);
  srcs = _srcs;
  hover_srcs = _hover_srcs;
  disabled_srcs = _disabled_srcs;
  active_srcs = _active_srcs;
  focused_srcs = _focused_srcs;
  anim_speed = _anim_speed;
  hover_anim_speed = _hover_anim_speed;
  disabled_anim_speed = _disabled_anim_speed;
  active_anim_speed = _active_anim_speed;
  focused_anim_speed = _focused_anim_speed;
  spawn_time = game.engine.current_time;
}

void UISprite::update(Game &game) {
  // an empty UISprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  // updating input events first becuase get_current_src needs
  // data from input events, if dst w,h changes (it usually doesnt)
  // this may cause problems by being a frame behind the dst update.
  is_mouse_over_dst(game, false, dst);
  if (!is_hidden) {
    input_events.update(game, is_mouse_over_dst(game, false, dst));
  } else {
    input_events.clear_state();
  }
  auto &current_src = get_current_src(game);
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  scaled_dst = dst;
  set_scaled_rect(scaled_dst, game.engine.scale);
  current_src.update(scaled_dst);
}

void UISprite::draw(Game &game) {
  // an empty UISprite for a tile layer won't have any srcs
  if (srcs.size() == 0) {
    return;
  }
  if (is_hidden) {
    return;
  }
  // super super important to not push to the render queue things that don't
  // need to be rendered. Can reduce performance by a lot if you don't do this.
  // 80 by 80 tile map goes from 4500 fps to 2200 fps on -O3.
  // a UISprite dst of 0, 0 is equal to the  game_rect.x, game_rect.y
  // (bottom-left corner)
  if (scaled_dst.x < -scaled_dst.w || scaled_dst.x > game.engine.game_rect.w ||
      scaled_dst.y < -scaled_dst.h || scaled_dst.y > game.engine.game_rect.h) {
    return;
  }
  game.engine.set_active_image(image);
  auto &src = get_current_src(game);
  game.engine.push_to_render_buffer(src.vertices, 12, src.uvs, 12);
}

SpriteSrc &UISprite::get_current_src(Game &game) {
  if (input_events.is_disabled && disabled_srcs.size() > 0) {
    current_frame_idx =
        get_current_frame_idx(game.engine.current_time, spawn_time,
                              disabled_srcs.size(), disabled_anim_speed);
    return disabled_srcs[current_frame_idx];
  } else if (is_focused && focused_srcs.size() > 0) {
    current_frame_idx =
        get_current_frame_idx(game.engine.current_time, spawn_time,
                              focused_srcs.size(), focused_anim_speed);
    return focused_srcs[current_frame_idx];
  } else if (input_events.was_mouse_down_when_mouse_over &&
             active_srcs.size() > 0) {
    current_frame_idx =
        get_current_frame_idx(game.engine.current_time, spawn_time,
                              active_srcs.size(), active_anim_speed);
    return active_srcs[current_frame_idx];
  } else if (input_events.is_mouse_over && hover_srcs.size() > 0) {
    current_frame_idx =
        get_current_frame_idx(game.engine.current_time, spawn_time,
                              hover_srcs.size(), hover_anim_speed);
    return hover_srcs[current_frame_idx];
  } else {
    current_frame_idx = get_current_frame_idx(
        game.engine.current_time, spawn_time, srcs.size(), anim_speed);
    return srcs[current_frame_idx];
  }
  cout << "SpriteSrc::get_current_src. Unexpected path reached.\n";
  abort();
}

void UISprite::set_is_hidden(bool _is_hidden) { is_hidden = _is_hidden; }

void UISprite::set_srcs(Image _image, vector<SpriteSrc> &_srcs,
                        int _anim_speed) {
  image = _image;
  srcs = _srcs;
  anim_speed = _anim_speed;
}

const UISprite &UISprite::set_dst_xy(int x, int y) {
  dst.x = x;
  dst.y = y;
  return *this;
}