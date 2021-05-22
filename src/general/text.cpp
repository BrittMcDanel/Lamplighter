#include "text.h"
#include "game.h"

WordsInLine::WordsInLine() {
  start_idx = 0;
  end_idx = 0;
  line_width = 0;
  line = 0;
}

WordsInLine::WordsInLine(int _start_idx, int _end_idx, int _line_width,
                         int _line) {
  start_idx = _start_idx;
  end_idx = _end_idx;
  line_width = _line_width;
  line = _line;
}

TextRenderer::TextRenderer() {
  words = vector<WordsInLine>();
  line_widths = vector<int>();
  str_buffer = string("");
}

Text::Text() {
  font_handle = 0;
  base_font_size = 10;
  font_size = 10;
  str = "";
  max_width = 0;
  scaled_max_width = 0;
  dst = Rect(0, 0, 0, 0);
  scaled_dst = Rect(0, 0, 0, 0);
  alignment = TextAlignment::Left;
  font_color = FontColor::WhiteShadow;
  char_idx_rect = Rect(0, 0, 0, 0);
  text_char_tweens = Tweens();
  draw_one_char_at_a_time = false;
  is_hidden = false;
}

Text::Text(Game &game, int _base_font_size, FontColor _font_color,
           const char *_str, Vec2 _dst, int _max_width,
           TextAlignment _alignment) {
  base_font_size = _base_font_size;
  font_size = _base_font_size * game.engine.scale;
  font_color = _font_color;
  font_handle = game.engine.get_font_handle(font_size, font_color);
  str = string(_str);
  max_width = _max_width;
  scaled_max_width = _max_width * game.engine.scale;
  dst = Rect(_dst.x, _dst.y, 0, 0);
  scaled_dst = Rect(_dst.x, _dst.y, 0, 0);
  alignment = _alignment;
  char_idx_rect = Rect(0, 0, 0, 0);
  text_char_tweens = Tweens();
  draw_one_char_at_a_time = false;
  is_hidden = false;
}

void Text::update(Game &game) {
  scaled_max_width = max_width * game.engine.scale;
  font_size = base_font_size * game.engine.scale;
  font_handle = game.engine.get_font_handle(font_size, font_color);
  auto &font = game.engine.fonts[font_handle];
  text_char_tweens.update(game, char_idx_rect);
  scaled_dst = dst;
  if (is_camera_rendered) {
    set_scaled_rect_with_camera(scaled_dst, game.engine.camera.dst,
                                game.engine.scale);
  } else {
    set_scaled_rect(scaled_dst, game.engine.scale);
  }
  // split after scaled_dst has been set to get the number of lines
  // that this text occupies
  dst.w = game.text_renderer.set_text_line_split_info(
      game, font_handle, scaled_dst, scaled_max_width, str, ' ');
  scaled_dst.w *= game.engine.scale;
  auto text_height =
      game.text_renderer.line_widths.size() * font.get_adjusted_line_height();
  // setting height here on both even though only scaled_dst.h is used I
  // think.
  dst.h = text_height / game.engine.scale;
  scaled_dst.h = text_height;
  tweens.update(game, dst);
}

void Text::set_font_color(Game &game, FontColor _font_color) {
  font_color = _font_color;
  font_handle = game.engine.get_font_handle(font_size, font_color);
}

Vec2 Text::measure_text(Game &game, string &_str_to_measure) {
  assert(font_handle != -1);
  update(game);
  font_handle = game.engine.get_font_handle(font_size, font_color);
  auto &font = game.engine.fonts[font_handle];
  // split after scaled_dst has been set to get the number of lines
  // that this text occupies
  int w = game.text_renderer.set_text_line_split_info(
      game, font_handle, scaled_dst, scaled_max_width, _str_to_measure, ' ');
  auto text_height =
      game.text_renderer.line_widths.size() * font.get_adjusted_line_height();
  // setting height here on both even though only scaled_dst.h is used I think.
  text_height = text_height / game.engine.scale;
  return Vec2(w, text_height);
}

// text draws from scaled_dst.y down, i.e. a top-left origin, not a bottom-left
// origin.
void Text::draw(Game &game) {
  if (font_handle == -1) {
    return;
  } else if (is_hidden) {
    return;
  }
  auto &font = game.engine.fonts[font_handle];
  if (draw_one_char_at_a_time) {
    game.engine.draw_char_dsts(font, char_dsts, char_idx_rect.x);
  } else {
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
      word_dims =
          game.engine.measure_string(font, game.text_renderer.str_buffer);
      text_dims.x += word_dims.x;

      game.text_renderer.str_buffer.clear();
    }

    text_dims.x = scaled_dst.w;
  }
}

void Text::set_draw_one_char_at_a_time_mode(Game &game) {
  draw_one_char_at_a_time = true;
  start_draw_individual_chars(game);
}

void Text::start_draw_individual_chars(Game &game) {
  // need the update to set scaled dst x and y
  update(game);
  set_char_dsts(game);
  /*cout << dst.x << " " << dst.y << " " << scaled_dst.x << " " << scaled_dst.y
       << "\n";
  for (int i = 0; i < char_dsts.size(); i++) {
    auto &char_dst = char_dsts[i];
    cout << i << " " << char_dst.c << " " << char_dst.dst.x << " "
         << char_dst.dst.y << " " << char_dst.dst.w << " " << char_dst.dst.h
         << "\n";
  }*/
  char_idx_rect = Rect(0, 0, 0, 0);
  text_char_tweens.clear();
  // tweening from char idx 0 to tex_str.size(), showing one char at a time
  // using the rect's x value.
  // speed per char
  auto text_tween_speed = 30;
  text_char_tweens.tween_xys.push_back(TweenXY(
      Rect(0, 0, 0, 0), Rect(str.size(), 0, 0, 0), game.engine.current_time,
      (Uint32)(text_tween_speed * str.size()), (Uint32)0, TweenCallback(),
      []() {}, []() {}));
}

void Text::set_char_dsts(Game &game) {
  if (font_handle == -1) {
    return;
  } else if (is_hidden) {
    return;
  }
  char_dsts.clear();
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

    word_dims = game.engine.measure_string(font, game.text_renderer.str_buffer);
    game.engine.set_char_dsts(font, game.text_renderer.str_buffer, char_dsts,
                              text_dims);
    text_dims.x += word_dims.x;

    game.text_renderer.str_buffer.clear();
  }

  text_dims.x = scaled_dst.w;
}

int TextRenderer::set_text_line_split_info(Game &game, int font_handle,
                                           Rect scaled_dst,
                                           int scaled_max_width,
                                           string &_str_to_split, char delim) {
  Vec2 word_dims = Vec2(0, 0);
  Vec2 text_dims = Vec2(0, 0);
  int word_start_idx = 0;
  int word_end_idx = 0;
  int current_line_width = 0;
  int words_in_line = 0;
  int line = 0;
  auto &font = game.engine.fonts[font_handle];
  int font_height = font.line_height;
  // max line width
  int max_line_width = 0;

  words.clear();
  line_widths.clear();
  str_buffer.clear();
  for (size_t i = 0; i < _str_to_split.size(); i++) {
    auto c = _str_to_split[i];
    /* second check is to skip empty strings */
    if (c == delim && str_buffer.size() > 0) {
      // push back space
      str_buffer.push_back(c);
      word_dims = game.engine.measure_string(font, str_buffer);
      text_dims.x += word_dims.x;
      current_line_width += word_dims.x;
      if (current_line_width > scaled_max_width) {
        auto line_width = current_line_width - word_dims.x;
        auto scaled_line_width = line_width / game.engine.scale;
        if (scaled_line_width > max_line_width) {
          max_line_width = scaled_line_width;
        }
        line_widths.push_back(line_width);
        current_line_width = word_dims.x;
        line += 1;
        words_in_line = 0;
        text_dims.x = scaled_dst.x;
        text_dims.y += font_height;
      }
      words.push_back(WordsInLine(word_start_idx, i, word_dims.x, line));
      word_start_idx = i + 1;
      words_in_line += 1;
      str_buffer.clear();
    } else {
      str_buffer.push_back(c);
    }
  }

  /* add any final string (if the last char isn't a delim it won't be added
   * above. */
  if (str_buffer.size() > 0) {
    // this is not in the original string, but it makes the final line width
    // correct for right alignment.
    str_buffer.push_back(delim);
    word_dims = game.engine.measure_string(font, str_buffer);
    text_dims.x += word_dims.x;
    current_line_width += word_dims.x;
    // add the now overflowed line width, the final word will be added below;
    if (current_line_width > scaled_max_width) {
      auto line_width = current_line_width - word_dims.x;
      auto scaled_line_width = line_width / game.engine.scale;
      if (scaled_line_width > max_line_width) {
        max_line_width = scaled_line_width;
      }
      line_widths.push_back(line_width);
      current_line_width = word_dims.x;
      line += 1;
      words_in_line = 0;
      text_dims.x = scaled_dst.x;
      text_dims.y += font_height;
    }
    // handle the case where there is only one line of text or one word,
    // need to set the text dst w in that case here.
    if (line_widths.size() == 0) {
      max_line_width = current_line_width / game.engine.scale;
    }
    // add the final width, either a line with one word or a whole line.
    line_widths.push_back(current_line_width);

    words.push_back(
        WordsInLine(word_start_idx, _str_to_split.size(), word_dims.x, line));
  }

  return max_line_width;
}
