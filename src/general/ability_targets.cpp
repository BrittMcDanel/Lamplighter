#include "ability_targets.h"
#include "game.h"

AbilityTarget::AbilityTarget(Game &game, Vec2 _dims, AbilityElement _element,
                             Sprite _sprite) {
  dims = _dims;
  element = _element;
  sprite = _sprite;
}

void AbilityTarget::update(Game &game) { sprite.update(game); }

void AbilityTarget::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  sprite.draw(game);
}

void AbilityTarget::set_dst(Game &game, Vec2 dst, Vec2 hitbox_dims) {
  sprite.dst.set_xy(dst);
  sprite.dst.x += hitbox_dims.x / 2;
  sprite.dst.y += hitbox_dims.y / 2;
  sprite.dst.x -= dims.x / 2;
  sprite.dst.y -= dims.y / 2;
}

AbilityTargets::AbilityTargets(Game &game) {
  auto image = game.engine.get_image(ImageName::AbilityTargets);
  targets.push_back(
      AbilityTarget(game, Vec2(60, 60), AbilityElement::Fire,
                    Sprite(game, image, Vec2(0, 0),
                           vector<SpriteSrc>{SpriteSrc(
                               ImageLocation(image, Rect(0, 0, 60, 60)))},
                           100)));
  targets.push_back(
      AbilityTarget(game, Vec2(300, 300), AbilityElement::Physical,
                    Sprite(game, image, Vec2(0, 0),
                           vector<SpriteSrc>{SpriteSrc(
                               ImageLocation(image, Rect(0, 64, 300, 300)))},
                           100)));
  targets.push_back(
      AbilityTarget(game, Vec2(600, 600), AbilityElement::Physical,
                    Sprite(game, image, Vec2(0, 0),
                           vector<SpriteSrc>{SpriteSrc(
                               ImageLocation(image, Rect(0, 424, 600, 600)))},
                           100)));
}

const AbilityTarget &AbilityTargets::get_target(const Ability &ability,
                                                AbilityTargetDims target_dims) {
  Vec2 dims = Vec2(ability.stats.aoe.current, ability.stats.aoe.current);
  if (target_dims == AbilityTargetDims::Range) {
    dims = Vec2(ability.stats.range.current, ability.stats.range.current);
  }
  for (auto &target : targets) {
    // got rid of the element check for now, don't want to make many separate
    // images until the system is more solidified
    if (target.dims == dims) {
      return target;
    }
  }
  cout << "AbilityTargets::get_target. Target not found. Dims " << dims.x << " "
       << dims.y << " " << static_cast<int>(ability.ability_element) << "\n";
  abort();
  return targets.at(0);
}