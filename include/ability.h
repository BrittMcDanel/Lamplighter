#ifndef ABILITY_H
#define ABILITY_H
#include <SDL.h>

#include "constants.h"
#include "rapidjson/document.h"
#include "sprite.h"
#include "status_effect.h"
#include "tween.h"
#include "tweenable_sprite.h"
#include <boost/uuid/uuid.hpp>
#include <string>
#include <vector>
using namespace std;

struct Game;

struct Ability {
  boost::uuids::uuid guid;
  int pool_handle;
  bool in_use_in_pool;
  AbilityName ability_name = AbilityName::None;
  AbilityType ability_type;
  AbilityElement ability_element;
  string display_name;
  string description;
  Stats stats;
  vector<StatusEffectPct> status_effect_pcts;
  TweenableSprite portrait;
  TweenableSprite sprite;
  bool is_projectile;
  double projectile_speed = 1.0;
  Uint32 delay = 0;
  Tweens tweens;
  bool is_melee;
  Ability() = default;
  Ability(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void draw_at_dst(Game &game, bool _is_camera_rendered, Vec2 _dst_xy);
};

struct EquippedAbilities {
  Ability counter;
  EquippedAbilities() = default;
  EquippedAbilities(Game &game);
};

void ability_serialize(Game &game, Ability &ability);
void ability_serialize_into_file(Game &game, Ability &ability,
                                 const char *file_path);
Ability ability_deserialize_from_file(Game &game, const char *file_path,
                                      bool use_static_asset_data = true);
Ability ability_deserialize(Game &game, GenericObject<false, Value> &obj,
                            bool use_static_asset_data = true);

#endif // ABILITY_H