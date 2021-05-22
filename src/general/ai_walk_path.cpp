#include "ai_walk_path.h"

AIWalkPath::AIWalkPath() {
  target_point = Vec2(0, 0);
  delay = 0;
}

AIWalkPath::AIWalkPath(Vec2 _target_point, Uint32 _delay) {
  target_point = _target_point;
  delay = _delay;
}