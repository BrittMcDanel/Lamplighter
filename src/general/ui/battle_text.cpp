#include "battle_text.h"
#include "game.h"
#include "unit.h"
#include "utils_game.h"

BattleText::BattleText(Game &game, Unit &unit, string &_text) {
  damage_text =
      Text(game, 10, FontColor::WhiteShadow, _text.c_str(),
           unit.sprite.dst.get_top_center(), 100, TextAlignment::Left);
  // update to avoid a frame of display delay
  damage_text.update(game);
}

void BattleText::update(Game &game, Unit &unit) {
  /*damage_text.max_width = 100;
  damage_text.dst.x = unit.sprite.dst.x - (damage_text.max_width / 2) +
                      (unit.sprite.hitbox_dims.x / 2);*/
  damage_text.update(game);
}

void BattleText::draw(Game &game) { damage_text.draw(game); }

void BattleTexts::update(Game &game, Unit &unit) {
  for (int i = battle_texts.items.size() - 1; i >= 0; i--) {
    auto &battle_text = battle_texts.items[i];
    if (battle_text.in_use_in_pool) {
      battle_text.update(game, unit);
    }
  }
}

void BattleTexts::draw(Game &game) {
  for (auto &battle_text : battle_texts.items) {
    if (battle_text.in_use_in_pool) {
      battle_text.draw(game);
    }
  }
}