#ifndef AIWalkPath_H
#define AIWalkPath_H

#include <SDL.h>
#include "constants.h"
#include "engine.h"
#include <vector>
using namespace std;

struct Game;

struct AIWalkPath {
  Vec2 target_point = Vec2(0, 0);
  Uint32 delay = 0;
  AIWalkPath();
  AIWalkPath(Vec2 _target_point, Uint32 _delay);
};

#endif // AIWalkPath_H