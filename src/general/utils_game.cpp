#include "utils_game.h"
#include "ability.h"
#include "game.h"
#include "unit.h"

Equipment::Equipment(Game &game) {
  primary_hand = Item();
  secondary_hand = Item();
  head = Item();
  armor = Item();
  boots = Item();
}

bool is_mouse_over_dst(Game &game, bool _is_camera_rendered, const Rect &dst) {
  if (_is_camera_rendered) {
    return rect_contains_point(dst,
                               game.engine.mouse_point_game_rect_scaled_camera);
  } else {
    return rect_contains_point(dst, game.engine.mouse_point_game_rect_scaled);
  }
  return false;
}

void draw_sprite_at_dst(Game &game, Image image, vector<SpriteSrc> &srcs,
                        Uint32 spawn_time, Uint32 anim_speed,
                        bool is_camera_rendered, Vec2 dst_xy) {
  GAME_ASSERT(image.image_name != ImageName::None);
  int current_frame_idx = get_current_frame_idx(
      game.engine.current_time, spawn_time, srcs.size(), anim_speed);
  auto &current_src = srcs[current_frame_idx];
  Rect dst = Rect(dst_xy.x, dst_xy.y, 0, 0);
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  auto scaled_dst = dst;
  if (is_camera_rendered) {
    set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                                game.engine.scale);
  } else {
    set_scaled_rect(scaled_dst, game.engine.scale);
  }
  current_src.update(scaled_dst);

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
  auto &src = srcs[current_frame_idx];
  game.engine.push_to_render_buffer(src.vertices, 12, src.uvs, 12);
}

void draw_sprite_at_dst(Game &game, Image image, SpriteSrc &current_src,
                        bool is_camera_rendered, Vec2 dst_xy) {
  GAME_ASSERT(image.image_name != ImageName::None);
  Rect dst = Rect(dst_xy.x, dst_xy.y, 0, 0);
  dst.w = current_src.image_location.src.w;
  dst.h = current_src.image_location.src.h;
  auto scaled_dst = dst;
  if (is_camera_rendered) {
    set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                                game.engine.scale);
  } else {
    set_scaled_rect(scaled_dst, game.engine.scale);
  }
  current_src.update(scaled_dst);

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

void draw_text_at_dst(Game &game, int font_handle, string &str,
                      TextAlignment alignment, int scaled_max_width,
                      bool is_camera_rendered, Vec2 dst_xy) {
  Rect dst = Rect(dst_xy.x, dst_xy.y, 0, 0);
  Rect scaled_dst = dst;
  if (is_camera_rendered) {
    set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                                game.engine.scale);
  } else {
    set_scaled_rect(scaled_dst, game.engine.scale);
  }
  auto &font = game.engine.fonts[font_handle];
  int line = -1;
  int total_line_width = 0;
  int line_width, line_start_x;
  Vec2 word_dims = Vec2(0, 0);
  Vec2 text_dims = Vec2(scaled_dst.x, scaled_dst.y);
  // 1.1 to give a little more space between lines
  auto font_height = font.get_adjusted_line_height();

  game.text_renderer.set_text_line_split_info(game, font_handle, scaled_dst,
                                              scaled_max_width, str, ' ');
  game.text_renderer.str_buffer.clear();
  for (size_t i = 0; i < game.text_renderer.words.size(); i++) {
    auto &word = game.text_renderer.words[i];
    if (word.line != line) {
      line = word.line;
      total_line_width = game.text_renderer.line_widths[line];
      text_dims.x = scaled_dst.x;
      if (alignment == TextAlignment::Center) {
        text_dims.x =
            scaled_dst.x + ((scaled_max_width - total_line_width) / 2);
      } else if (alignment == TextAlignment::Right) {
        text_dims.x = scaled_dst.x + (scaled_max_width - total_line_width);
      }
    }

    text_dims.y = scaled_dst.y - (line * font_height);
    for (size_t j = word.start_idx; j < (size_t)word.end_idx; j++) {
      game.text_renderer.str_buffer.push_back(str[j]);
    }
    if (i != game.text_renderer.words.size() - 1) {
      game.text_renderer.str_buffer.push_back(' ');
    }

    game.engine.draw_string(font, game.text_renderer.str_buffer, text_dims);
    word_dims = game.engine.measure_string(font, game.text_renderer.str_buffer);
    text_dims.x += word_dims.x;

    game.text_renderer.str_buffer.clear();
  }

  text_dims.x = scaled_dst.w;
}

Item get_coin_item(Game &game) {
  Item item = Item(game);
  auto image = game.engine.get_image(ImageName::Items);
  item.item_name = ItemName::Coin;
  item.item_type = ItemType::Money;
  item.display_name = "Money";
  item.description = "Money description";
  item.sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))),
                      },
                      100);
  return item;
}

Item get_cash_item(Game &game) {
  Item item = Item(game);
  auto image = game.engine.get_image(ImageName::Items);
  item.item_name = ItemName::Coin;
  item.item_type = ItemType::Money;
  item.display_name = "Money";
  item.description = "Money description";
  item.sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(140, 0, 20, 20))),
                      },
                      100);
  return item;
}

const Ability &get_default_attack_ability(Game &game, Unit &unit) {
  return game.assets.get_ability(AbilityName::SwordSlash);
}

bool is_ability_in_range(const Ability &ability, Vec2 start_dst,
                         Vec2 target_dst) {
  // ability.stats.range is the diameter, radius is diameter / 2
  // the circle is in a square
  return dist(start_dst, target_dst) <= (ability.stats.range.current / 2);
}