#ifndef STATUSEFFECT_H
#define STATUSEFFECT_H
#include <SDL.h>

#include "constants.h"
#include "rapidjson/document.h"
#include "sprite.h"
#include "tween.h"
#include "tweenable_sprite.h"
#include <string>
#include <vector>
using namespace std;

struct Game;

struct StatusEffect {
  StatusEffectName status_effect_name = StatusEffectName::None;
  StatusEffectType status_effect_type = StatusEffectType::Buff;
  string display_name;
  string description;
  Stats stats;
  TweenableSprite portrait;
  TweenableSprite sprite;
  int turns_remaining;
  StatusEffect() = default;
  StatusEffect(Game &game);
};

struct StatusEffectPct {
  StatusEffect status_effect;
  double pct;
  StatusEffectPct() = default;
  StatusEffectPct(StatusEffect &_status_effect, double _pct);
};

void status_effect_serialize(Game &game, StatusEffect &status_effect);
void status_effect_serialize_into_file(Game &game, StatusEffect &status_effect,
                                       const char *file_path);
StatusEffect
status_effect_deserialize_from_file(Game &game, const char *file_path,
                                    bool use_static_asset_data = true);
StatusEffect status_effect_deserialize(Game &game,
                                       GenericObject<false, Value> &obj,
                                       bool use_static_asset_data = true);

void status_effect_pct_serialize(Game &game,
                                 StatusEffectPct &status_effect_pct);
void status_effect_pct_serialize_into_file(Game &game,
                                           StatusEffectPct &status_effect_pct,
                                           const char *file_path);
StatusEffectPct
status_effect_pct_deserialize_from_file(Game &game, const char *file_path,
                                        bool use_static_asset_data = true);
StatusEffectPct
status_effect_pct_deserialize(Game &game, GenericObject<false, Value> &obj,
                              bool use_static_asset_data = true);

#endif // STATUSEFFECT_H