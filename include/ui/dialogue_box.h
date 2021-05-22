#ifndef UI_DIALOGUEBOX_H
#define UI_DIALOGUEBOX_H

#include "fixed_sprite.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "ui/expandable_sprite.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;
struct Unit;

struct DialogueBox {
  ExpandableSprite expandable_sprite;
  Sprite bottom_arrow;
  Sprite continue_cursor;
  Text text;
  int bottom_arrow_w;
  bool draw_one_char_at_a_time;
  bool show_cursor;
  bool is_hidden;
  DialogueBox() = default;
  DialogueBox(Game &game, bool _draw_one_char_at_a_time = true,
              bool _show_cursor = true);
  void update(Game &game, Unit &unit);
  void draw(Game &game);
  void update_layout(Game &game, Unit &unit);
  void set_text_str(Game &game, Unit &unit, const string &_str);
};

#endif // UI_DIALOGUEBOX_H