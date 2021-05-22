#ifndef UI_MOVE_POINT_ICONS_H
#define UI_MOVE_POINT_ICONS_H

#include "item.h"
#include "sprite.h"
#include "text.h"
#include "tween.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

// shows the player what their path will be when moving in battle
struct MovePointIcons {
  vector<Sprite> icons = vector<Sprite>();
  MovePointIcons() = default;
  MovePointIcons(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void show_path(Game &game, const vector<Vec2> &path);
  void hide_all();
};

#endif // UI_MOVE_POINT_ICONS_H