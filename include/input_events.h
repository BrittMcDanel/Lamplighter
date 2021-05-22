#ifndef INPUTEVENTS_H
#define INPUTEVENTS_H
#include <SDL2/SDL.h>

#include "utils.h"
#include <functional>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

struct Game;

struct InputEvents {
  Vec2 mouse_dst_when_mouse_down = Vec2(-1, -1);
  bool is_disabled = false;
  bool is_mouse_enter;
  bool is_mouse_exit;
  bool is_mouse_over;
  bool is_mouse_down;
  bool is_mouse_held_down;
  bool is_mouse_up;
  bool is_right_mouse_down;
  bool is_right_mouse_held_down;
  bool is_right_mouse_up;
  bool is_click;
  bool is_right_click;
  bool was_mouse_down_when_mouse_over;
  bool was_right_mouse_down_when_mouse_over;
  function<void()> on_mouse_enter;
  function<void()> on_mouse_exit;
  function<void()> on_mouse_over;
  function<void()> on_mouse_down;
  function<void()> on_mouse_held_down;
  function<void()> on_mouse_up;
  function<void()> on_right_mouse_down;
  function<void()> on_right_mouse_held_down;
  function<void()> on_right_mouse_up;
  function<void()> on_click;
  function<void()> on_right_click;
  InputEvents();
  void update(Game &game, bool _is_mouse_over);
  void clear_state(bool clear_mouse_down_dst = true);
};

#endif //  INPUTEVENTS_H