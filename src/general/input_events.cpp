#include "input_events.h"
#include "game.h"

InputEvents::InputEvents() {
  mouse_dst_when_mouse_down = Vec2(-1, -1);
  is_disabled = false;
  is_mouse_enter = false;
  is_mouse_exit = false;
  is_mouse_over = false;
  is_mouse_down = false;
  is_mouse_held_down = false;
  is_mouse_up = false;
  is_right_mouse_down = false;
  is_right_mouse_held_down = false;
  is_right_mouse_up = false;
  is_click = false;
  is_right_click = false;
  was_mouse_down_when_mouse_over = false;
  was_right_mouse_down_when_mouse_over = false;
  on_mouse_enter = []() {};
  on_mouse_exit = []() {};
  on_mouse_over = []() {};
  on_mouse_down = []() {};
  on_mouse_held_down = []() {};
  on_mouse_up = []() {};
  on_right_mouse_down = []() {};
  on_right_mouse_held_down = []() {};
  on_right_mouse_up = []() {};
  on_click = []() {};
  on_right_click = []() {};
}

void InputEvents::clear_state(bool clear_mouse_down_dst) {
  if (clear_mouse_down_dst) {
    mouse_dst_when_mouse_down = Vec2(-1, -1);
  }
  is_mouse_over = false;
  is_mouse_enter = false;
  is_mouse_exit = false;
  is_mouse_down = false;
  is_mouse_held_down = false;
  is_mouse_up = false;
  is_right_mouse_down = false;
  is_right_mouse_held_down = false;
  is_right_mouse_up = false;
  is_click = false;
  is_right_click = false;
}

void InputEvents::update(Game &game, bool _is_mouse_over) {
  clear_state(false);

  if (_is_mouse_over) {
    // call onMouseEnter once by calling it before the isMouseOver flag is set
    if (!is_mouse_over) {
      is_mouse_enter = true;
      on_mouse_enter();
    }
    is_mouse_over = true;
    on_mouse_over();
  } else {
    // call only once before flag is set
    if (is_mouse_over) {
      is_mouse_exit = true;
      on_mouse_exit();
    }
    is_mouse_over = false;
  }

  // prevent mouse down up events if disabled
  if (is_disabled) {
    return;
  }

  // events that require mouse enter status
  // mouse down
  if (game.engine.is_mouse_down) {
    if (is_mouse_over) {
      was_mouse_down_when_mouse_over = true;
      is_mouse_down = true;
      on_mouse_down();
      mouse_dst_when_mouse_down = game.engine.mouse_point_game_rect_scaled;
      /*if (canDrag) {
        isDragging = true;
        SetDragGhostDst();
        onDragStart();
      }*/
    } else {
      was_mouse_down_when_mouse_over = false;
    }
  }
  // mouse up
  if (game.engine.is_mouse_up) {
    /*if (canDrag && isDragging) {
      isDragging = false;
      onDrop();
    }*/
    if (is_mouse_over) {
      is_mouse_up = true;
      on_mouse_up();
      if (was_mouse_down_when_mouse_over) {
        // isFocused = true;
        is_click = true;
        on_click();
        // onFocus();
      }
    } else {
      // isFocused = false;
      // onFocusLost();
    }
    // always set to false on mouse up
    was_mouse_down_when_mouse_over = false;
  }
  // mouse held down
  if (game.engine.is_mouse_held_down) {
    if (is_mouse_over) {
      is_mouse_held_down = true;
      on_mouse_held_down();
    }
  }
  // right mouse down
  if (game.engine.is_right_mouse_down) {
    if (is_mouse_over) {
      was_right_mouse_down_when_mouse_over = true;
      is_right_mouse_up = true;
      on_right_mouse_down();
    } else {
      was_right_mouse_down_when_mouse_over = false;
    }
  }
  // right mouse up
  if (game.engine.is_right_mouse_up) {
    if (is_mouse_over) {
      on_right_mouse_up();
      if (was_right_mouse_down_when_mouse_over) {
        is_right_click = true;
        on_right_click();
      }
    }
    // always set to false on mouse up
    was_right_mouse_down_when_mouse_over = false;
  }
  // right mouse held down
  if (game.engine.is_right_mouse_held_down) {
    if (is_mouse_over) {
      is_right_mouse_held_down = true;
      on_right_mouse_held_down();
    }
  }
}
