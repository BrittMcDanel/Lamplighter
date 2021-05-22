#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "engine.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct Dialogue {
  GameFlag game_flag_condition = GameFlag::Always;
  vector<string> strs = vector<string>();
};

#endif // DIALOGUE_H