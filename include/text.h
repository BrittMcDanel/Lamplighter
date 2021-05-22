#ifndef TEXT_H
#define TEXT_H

#include "tween.h"
#include "utils.h"
#include <SDL.h>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Font;

struct Text {
  int font_handle = -1;
  // before being multiplied by engine.scale
  int base_font_size = 16;
  int font_size = 16;
  FontColor font_color = FontColor::None;
  string str = "";
  int max_width = 0;
  int scaled_max_width = 0;
  Rect dst = Rect(0, 0, 0, 0);
  Rect scaled_dst = Rect(0, 0, 0, 0);
  TextAlignment alignment = TextAlignment::Left;
  vector<CharDst> char_dsts = vector<CharDst>();
  Rect char_idx_rect = Rect(0, 0, 0, 0);
  Tweens text_char_tweens = Tweens();
  Tweens tweens = Tweens();
  bool draw_one_char_at_a_time = false;
  bool is_camera_rendered = true;
  bool is_hidden = false;
  Text();
  Text(Game &game, int _base_font_size, FontColor _font_color, const char *_str,
       Vec2 _dst, int _max_width, TextAlignment _alignment);
  void update(Game &game);
  void draw(Game &game);
  Vec2 measure_text(Game &game, string &_str_to_measure);
  void set_char_dsts(Game &game);
  void set_draw_one_char_at_a_time_mode(Game &game);
  void start_draw_individual_chars(Game &game);
  void set_font_color(Game &game, FontColor _font_color);
};

struct WordsInLine {
  int start_idx;
  int end_idx;
  int line_width;
  int line;
  WordsInLine();
  WordsInLine(int _start_idx, int _end_idx, int _line_width, int _line);
};

struct TextRenderer {
  vector<WordsInLine> words;
  vector<int> line_widths;
  string str_buffer;
  TextRenderer();
  int set_text_line_split_info(Game &game, int font_handle, Rect scaled_dst,
                               int scaled_max_width, string &_str_to_split,
                               char delim);
};

#endif // TEXT_H