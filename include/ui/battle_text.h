#ifndef BATTLE_TEXT_H
#define BATTLE_TEXT_H

#include "constants.h"
#include "pool.h"
#include "text.h"
#include "utils.h"

struct Game;
struct Unit;

struct BattleText {
  int pool_handle;
  bool in_use_in_pool;
  Text damage_text;
  BattleText() = default;
  BattleText(Game &game, Unit &unit, string &_text);
  void update(Game &game, Unit &unit);
  void draw(Game &game);
};

struct BattleTexts {
  Pool<BattleText> battle_texts = Pool<BattleText>();
  BattleTexts() = default;
  void update(Game &game, Unit &unit);
  void draw(Game &game);
};

#endif // BATTLE_TEXT_H