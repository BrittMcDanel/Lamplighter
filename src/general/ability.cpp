#include "ability.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include "utils_game.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Ability::Ability(Game &game) {
  guid = game.engine.get_guid();
  auto image = game.engine.get_image(ImageName::Abilities);
  auto portrait_image = game.engine.get_image(ImageName::AbilityIcons);
  ability_name = AbilityName::None;
  sprite =
      TweenableSprite(game, image, Vec2(0, 0),
                      vector<SpriteSrc>{
                          SpriteSrc(ImageLocation(image, Rect(0, 0, 20, 20))),
                      },
                      100);
  portrait = TweenableSprite(
      game, portrait_image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(portrait_image, Rect(0, 0, 20, 20))),
      },
      100);
  tweens = Tweens();
  stats = Stats(Range(1), Range(1), Range(5), Range(1), Range(1), Range(0));
  is_projectile = false;
  projectile_speed = 1.0;
  sprite.spawn_time = game.engine.current_time;
  sprite.dst = Rect(0, 0, 0, 0);
  sprite.scaled_dst = Rect(0, 0, 0, 0);
  sprite.image = game.engine.get_image(ImageName::Abilities);
}

void Ability::update(Game &game) {
  portrait.update(game);
  sprite.update(game);
  tweens.update(game, sprite.dst);
}

void Ability::draw(Game &game) { sprite.draw(game); }

void Ability::draw_at_dst(Game &game, bool _is_camera_rendered, Vec2 _dst_xy) {
  if (ability_name == AbilityName::None) {
    return;
  }
  draw_sprite_at_dst(game, portrait.image,
                     portrait.srcs.at(portrait.current_frame_idx),
                     _is_camera_rendered, _dst_xy);
}

EquippedAbilities::EquippedAbilities(Game &game) {
  counter = game.assets.get_ability(AbilityName::CounterAttack);
}

void ability_serialize(Game &game, Ability &ability) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_string_val("guid", to_string(ability.guid));
  game.serializer.serialize_int("ability_name",
                                static_cast<int>(ability.ability_name));
  game.serializer.serialize_int("ability_type",
                                static_cast<int>(ability.ability_type));
  game.serializer.serialize_int("ability_element",
                                static_cast<int>(ability.ability_element));
  game.serializer.serialize_string("display_name", ability.display_name);
  game.serializer.serialize_string("description", ability.description);
  game.serializer.serialize_stats("stats", ability.stats);
  game.serializer.writer.String("sprite");
  tweenable_sprite_serialize(game, ability.sprite);
  game.serializer.writer.String("portrait");
  tweenable_sprite_serialize(game, ability.portrait);
  game.serializer.serialize_bool("is_projectile", ability.is_projectile);
  game.serializer.serialize_double("projectile_speed",
                                   ability.projectile_speed);
  game.serializer.serialize_uint("delay", ability.delay);
  game.serializer.writer.String("tweens");
  tweens_serialize(game, ability.sprite.tweens);
  game.serializer.serialize_bool("is_melee", ability.is_melee);
  game.serializer.writer.String("status_effect_pcts");
  game.serializer.writer.StartArray();
  for (auto &status_effect_pct : ability.status_effect_pcts) {
    status_effect_pct_serialize(game, status_effect_pct);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.EndObject();
}

Ability ability_deserialize(Game &game, GenericObject<false, Value> &obj,
                            bool use_static_asset_data) {
  auto ability = Ability(game);
  ability.guid = game.engine.string_gen(obj["guid"].GetString());
  ability.ability_name = static_cast<AbilityName>(obj["ability_name"].GetInt());

  if (use_static_asset_data) {
    auto &static_ability = game.assets.get_ability(ability.ability_name);
    ability = static_ability;
  } else {
    // called from assets::update_all_assets_from_files only - assets loads the
    // prefab ability files here and stores them as static assets. After that,
    // abilities can be retrieved statically from assets.
    ability.ability_type =
        static_cast<AbilityType>(obj["ability_type"].GetInt());
    ability.ability_element =
        static_cast<AbilityElement>(obj["ability_element"].GetInt());
    ability.display_name = obj["display_name"].GetString();
    ability.description = obj["description"].GetString();
    game.serializer.deserialize_stats(obj, "stats", ability.stats);
    auto sprite_obj = obj["sprite"].GetObject();
    auto portrait_obj = obj["portrait"].GetObject();
    ability.sprite = tweenable_sprite_deserialize(game, sprite_obj);
    ability.portrait = tweenable_sprite_deserialize(game, portrait_obj);
    ability.is_projectile = obj["is_projectile"].GetBool();
    ability.projectile_speed = obj["projectile_speed"].GetDouble();
    ability.delay = obj["delay"].GetUint();
    ability.is_melee = obj["is_melee"].GetBool();
    auto status_effect_pct_array = obj["status_effect_pcts"].GetArray();
    for (auto &status_effect_pct_element : status_effect_pct_array) {
      auto status_effect_pct_obj = status_effect_pct_element.GetObject();
      auto status_effect_pct = status_effect_pct_deserialize(
          game, status_effect_pct_obj, use_static_asset_data);
      // make sure all status effects in abilities are up to date with the
      // the static asset data from the prefabs/statuseffects folder. If not,
      // abilities could be using old status effect data (Later just serialize
      // the status effect name, not the whole thing so its more obvious).
      // status effects are loaded before abilities, so they are ready here
      // to be loaded from assets.
      auto &static_status_effect = game.assets.get_status_effect(
          status_effect_pct.status_effect.status_effect_name);
      status_effect_pct.status_effect = static_status_effect;
      ability.status_effect_pcts.push_back(status_effect_pct);
    }
  }
  auto tweens_obj = obj["tweens"].GetObject();
  ability.tweens = tweens_deserialize(game, tweens_obj);
  return ability;
}

void ability_serialize_into_file(Game &game, Ability &ability,
                                 const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  ability_serialize(game, ability);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "unit_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Ability ability_deserialize_from_file(Game &game, const char *file_path,
                                      bool use_static_asset_data) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "unit_deserialize_from_file. File error " << file_path << "\n";
    abort();
  }
  stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  // cout << "buff " << buffer.str().c_str() << "\n";
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  // parse
  game.serializer.doc.Parse(buffer.str().c_str());
  // unit object
  auto obj = game.serializer.doc.GetObject();
  // deserialize unit_obj into a Unit
  auto ability = ability_deserialize(game, obj, use_static_asset_data);
  // set a unique guid as this is a copy of the unit from the json file
  // and will have the guid in the json file.
  ability.guid = game.engine.get_guid();
  return ability;
}
