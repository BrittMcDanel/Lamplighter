#include "ui/dialogue_box.h"
#include "game.h"

DialogueBox::DialogueBox(Game &game, bool _draw_one_char_at_a_time,
                         bool _show_cursor) {
  draw_one_char_at_a_time = _draw_one_char_at_a_time;
  show_cursor = _show_cursor;
  auto image = game.engine.get_image(ImageName::UI);
  is_hidden = true;
  auto corners = vector<Sprite>();
  auto edges = vector<FixedSprite>();
  auto center = FixedSprite();
  bottom_arrow_w = 24;
  bottom_arrow = Sprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(0, 16, bottom_arrow_w, 12))),
      },
      100);
  continue_cursor =
      Sprite(game, image, Vec2(0, 0),
             vector<SpriteSrc>{
                 SpriteSrc(ImageLocation(image, Rect(24, 16, 15, 18))),
                 SpriteSrc(ImageLocation(image, Rect(39, 16, 15, 18))),
                 SpriteSrc(ImageLocation(image, Rect(54, 16, 15, 18))),
             },
             100);
  if (!show_cursor) {
    continue_cursor.is_hidden = true;
  }
  text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0), 150,
              TextAlignment::Left);
  if (draw_one_char_at_a_time) {
    text.draw_one_char_at_a_time = true;
  }

  expandable_sprite = ExpandableSprite(game);
}

void DialogueBox::update(Game &game, Unit &unit) {
  text.update(game);
  expandable_sprite.update(game);
  bottom_arrow.update(game);
  continue_cursor.update(game);
}

void DialogueBox::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  expandable_sprite.draw(game);
  bottom_arrow.draw(game);
  text.draw(game);
  continue_cursor.draw(game);
}

void DialogueBox::set_text_str(Game &game, Unit &unit, const string &_str) {
  text.str = _str;
  update_layout(game, unit);
}

void DialogueBox::update_layout(Game &game, Unit &unit) {
  assert(text.font_handle != -1);
  // text has been updated, can get text.dst.h now
  bottom_arrow.dst.set_xy(
      unit.sprite.dst.x + (unit.sprite.hitbox_dims_input_events.x / 2) -
          (bottom_arrow.dst.w / 2),
      unit.sprite.dst.y + unit.sprite.hitbox_dims_input_events.y);
  auto text_dims = text.measure_text(game, text.str);
  auto w = text.dst.w;
  if (w < bottom_arrow_w) {
    w = bottom_arrow_w + BACKGROUND_PADDING;
  }
  auto box_x = unit.sprite.dst.x - (w / 2) +
               (unit.sprite.hitbox_dims_input_events.x / 2);
  text.dst.set_xy(unit.sprite.dst.x - (text.dst.w / 2) +
                      (unit.sprite.hitbox_dims_input_events.x / 2),
                  unit.sprite.dst.y + unit.sprite.hitbox_dims_input_events.y +
                      text_dims.y + expandable_sprite.corner_size +
                      bottom_arrow.dst.h);
  if (draw_one_char_at_a_time) {
    text.start_draw_individual_chars(game);
  }
  text.update(game);

  auto &font = game.engine.fonts[text.font_handle];
  auto text_w = w;
  auto top_y = text.dst.y + expandable_sprite.corner_size - 1;
  auto bottom_y = top_y - text.dst.h - expandable_sprite.corner_size * 2;
  auto left_x = box_x - expandable_sprite.corner_size;
  // corner size - 1 feels like the right side needs a little more padding
  auto right_x = box_x + text_w + (expandable_sprite.corner_size);
  expandable_sprite.update_layout(
      game, Rect(left_x, bottom_y, right_x - left_x, top_y - bottom_y));
  continue_cursor.dst.set_xy(
      expandable_sprite.corners[3].dst.x - expandable_sprite.corner_size,
      expandable_sprite.corners[3].dst.y - expandable_sprite.corner_size * 2);

  // initial update is needed for some reason or there is a frame where it
  // is obviouslly in the previous state.
  bottom_arrow.update(game);
  continue_cursor.update(game);
}