#ifndef ABILITY_TARGETS_H
#define ABILITY_TARGETS_H

#include "ability.h"
#include "constants.h"
#include <SDL.h>
#include <vector>
using namespace std;

struct Game;

struct AbilityTarget {
  AbilityElement element;
  Vec2 dims;
  Sprite sprite;
  bool is_hidden;
  AbilityTarget() = default;
  AbilityTarget(Game &game, Vec2 _dims, AbilityElement _element,
                Sprite _sprite);
  void update(Game &game);
  void draw(Game &game);
  void set_dst(Game &game, Vec2 dst, Vec2 hitbox_dims = Vec2(0, 0));
};

struct AbilityTargets {
  vector<AbilityTarget> targets;
  AbilityTargets() = default;
  AbilityTargets(Game &game);
  const AbilityTarget &
  get_target(const Ability &ability,
             AbilityTargetDims target_dims = AbilityTargetDims::AOE);
};

#endif // ABILITY_TARGETS_H