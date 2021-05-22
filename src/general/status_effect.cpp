#include "status_effect.h"
#include "game.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

StatusEffect::StatusEffect(Game &game) {
  auto image = game.engine.get_image(ImageName::Abilities);
  auto portrait_image = game.engine.get_image(ImageName::AbilityIcons);
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
}

StatusEffectPct::StatusEffectPct(StatusEffect &_status_effect, double _pct) {
  status_effect = _status_effect;
  pct = _pct;
}

void status_effect_serialize(Game &game, StatusEffect &status_effect) {
  game.serializer.writer.StartObject();
  game.serializer.serialize_int(
      "status_effect_name", static_cast<int>(status_effect.status_effect_name));
  game.serializer.serialize_int(
      "status_effect_type", static_cast<int>(status_effect.status_effect_type));
  game.serializer.serialize_string("display_name", status_effect.display_name);
  game.serializer.serialize_string("description", status_effect.description);
  game.serializer.serialize_stats("stats", status_effect.stats);
  game.serializer.serialize_int("turns_remaining",
                                status_effect.turns_remaining);
  game.serializer.writer.String("sprite");
  tweenable_sprite_serialize(game, status_effect.sprite);
  game.serializer.writer.String("portrait");
  tweenable_sprite_serialize(game, status_effect.portrait);
  game.serializer.writer.EndObject();
}

StatusEffect status_effect_deserialize(Game &game,
                                       GenericObject<false, Value> &obj,
                                       bool use_static_asset_data) {
  auto status_effect = StatusEffect(game);
  status_effect.status_effect_name =
      static_cast<StatusEffectName>(obj["status_effect_name"].GetInt());

  if (use_static_asset_data) {
    auto &static_status_effect =
        game.assets.get_status_effect(status_effect.status_effect_name);
    status_effect = static_status_effect;
  } else {
    status_effect.status_effect_type =
        static_cast<StatusEffectType>(obj["status_effect_type"].GetInt());
    status_effect.display_name = obj["display_name"].GetString();
    status_effect.description = obj["description"].GetString();
    game.serializer.deserialize_stats(obj, "stats", status_effect.stats);
    status_effect.turns_remaining = obj["turns_remaining"].GetInt();
    auto sprite_obj = obj["sprite"].GetObject();
    auto portrait_obj = obj["portrait"].GetObject();
    status_effect.sprite = tweenable_sprite_deserialize(game, sprite_obj);
    status_effect.portrait = tweenable_sprite_deserialize(game, portrait_obj);
  }

  return status_effect;
}

void status_effect_serialize_into_file(Game &game, StatusEffect &status_effect,
                                       const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  status_effect_serialize(game, status_effect);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "status_effect_serialize_into_file. File error " << file_path
         << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

StatusEffect status_effect_deserialize_from_file(Game &game,
                                                 const char *file_path,
                                                 bool use_static_asset_data) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "status_effect_deserialize_from_file. File error " << file_path
         << "\n";
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
  auto status_effect =
      status_effect_deserialize(game, obj, use_static_asset_data);
  return status_effect;
}

// status effect pct

void status_effect_pct_serialize(Game &game,
                                 StatusEffectPct &status_effect_pct) {
  game.serializer.writer.StartObject();
  game.serializer.writer.String("status_effect");
  status_effect_serialize(game, status_effect_pct.status_effect);
  game.serializer.serialize_double("pct", status_effect_pct.pct);
  game.serializer.writer.EndObject();
}

StatusEffectPct status_effect_pct_deserialize(Game &game,
                                              GenericObject<false, Value> &obj,
                                              bool use_static_asset_data) {
  auto status_effect_obj = obj["status_effect"].GetObject();
  auto status_effect =
      status_effect_deserialize(game, status_effect_obj, use_static_asset_data);
  auto pct = obj["pct"].GetDouble();
  return StatusEffectPct(status_effect, pct);
}

void status_effect_pct_serialize_into_file(Game &game,
                                           StatusEffectPct &status_effect_pct,
                                           const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  status_effect_pct_serialize(game, status_effect_pct);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "status_effect_pct_serialize_into_file. File error " << file_path
         << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

StatusEffectPct
status_effect_pct_deserialize_from_file(Game &game, const char *file_path,
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
  auto status_effect_pct =
      status_effect_pct_deserialize(game, obj, use_static_asset_data);
  return status_effect_pct;
}
