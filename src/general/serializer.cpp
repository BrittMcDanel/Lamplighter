#include "serializer.h"
#include "ai_walk_path.h"
#include "dialogue.h"
#include "game.h"
#include "tween.h"

Serializer::Serializer() {
  doc = Document();
  sb = StringBuffer();
  writer.Reset(sb);
}

void Serializer::clear() {
  sb.Clear();
  writer.Reset(sb);
}

void Serializer::serialize_int(const char *key, int value) {
  writer.String(key);
  writer.Int(value);
}

void Serializer::serialize_uint(const char *key, Uint32 value) {
  writer.String(key);
  writer.Uint(value);
}

void Serializer::serialize_double(const char *key, double value) {
  writer.String(key);
  writer.Double(value);
}

void Serializer::serialize_bool(const char *key, bool value) {
  writer.String(key);
  writer.Bool(value);
}

void Serializer::serialize_string(const char *key, string &value) {
  writer.String(key);
  writer.String(value.c_str(), value.size());
}

void Serializer::serialize_string_val(const char *key, string value) {
  writer.String(key);
  writer.String(value.c_str(), value.size());
}

void Serializer::serialize_double_point(const char *key, DoublePoint &value) {
  writer.String(key);
  writer.StartObject();
  serialize_double("x", value.x);
  serialize_double("y", value.y);
  writer.EndObject();
}

void Serializer::deserialize_double_point(GenericObject<false, Value> &obj,
                                          const char *key, DoublePoint &value) {
  auto point_obj = obj[key].GetObject();
  value.x = point_obj["x"].GetDouble();
  value.y = point_obj["y"].GetDouble();
}

void Serializer::serialize_vec2(const char *key, Vec2 &value) {
  writer.String(key);
  writer.StartObject();
  serialize_int("x", value.x);
  serialize_int("y", value.y);
  writer.EndObject();
}

void Serializer::deserialize_vec2(GenericObject<false, Value> &obj,
                                  const char *key, Vec2 &value) {
  auto vec_obj = obj[key].GetObject();
  value.x = vec_obj["x"].GetInt();
  value.y = vec_obj["y"].GetInt();
}

void Serializer::serialize_rect(const char *key, Rect &value) {
  writer.String(key);
  writer.StartObject();
  serialize_int("x", value.x);
  serialize_int("y", value.y);
  serialize_int("w", value.w);
  serialize_int("h", value.h);
  writer.EndObject();
}

void Serializer::deserialize_rect(GenericObject<false, Value> &obj,
                                  const char *key, Rect &value) {
  auto rect_obj = obj[key].GetObject();
  value.x = rect_obj["x"].GetInt();
  value.y = rect_obj["y"].GetInt();
  value.w = rect_obj["w"].GetInt();
  value.h = rect_obj["h"].GetInt();
}

void Serializer::serialize_sprite_src_vec(const char *key,
                                          vector<SpriteSrc> &srcs) {
  writer.String(key);
  writer.StartArray();
  for (auto &src : srcs) {
    writer.StartObject();
    writer.String("x");
    writer.Int(src.image_location.src.x);
    writer.String("y");
    writer.Int(src.image_location.src.y);
    writer.String("w");
    writer.Int(src.image_location.src.w);
    writer.String("h");
    writer.Int(src.image_location.src.h);
    writer.EndObject();
  }
  writer.EndArray();
}

void Serializer::deserialize_sprite_src_vec(GenericObject<false, Value> &obj,
                                            Image &image, const char *key,
                                            vector<SpriteSrc> &srcs) {
  srcs.clear();
  for (auto &rect : obj[key].GetArray()) {
    auto x = rect["x"].GetInt();
    auto y = rect["y"].GetInt();
    auto w = rect["w"].GetInt();
    auto h = rect["h"].GetInt();
    srcs.push_back(SpriteSrc(ImageLocation(image, Rect(x, y, w, h))));
  }
}

void Serializer::serialize_dialogues(vector<Dialogue> &dialogues) {
  writer.String("dialogues");
  writer.StartArray();
  for (auto &dialogue : dialogues) {
    writer.StartObject();
    serialize_int("game_flag_condition", (int)dialogue.game_flag_condition);
    writer.String("strs");
    writer.StartArray();
    for (auto &str : dialogue.strs) {
      writer.String(str.c_str(), str.size());
    }
    writer.EndArray();
    writer.EndObject();
  }
  writer.EndArray();
}

void Serializer::deserialize_dialogues(GenericObject<false, Value> &obj,
                                       vector<Dialogue> &dialogues) {
  // make sure json object has dialogues first
  if (obj.HasMember("dialogues")) {
    for (auto &dialogue_obj : obj["dialogues"].GetArray()) {
      Dialogue dialogue;
      auto game_flag_condition = dialogue_obj["game_flag_condition"].GetInt();
      dialogue.game_flag_condition = static_cast<GameFlag>(game_flag_condition);
      auto dialogue_strs_array = dialogue_obj["strs"].GetArray();
      for (auto &str_obj : dialogue_strs_array) {
        auto str = str_obj.GetString();
        dialogue.strs.push_back(str);
      }
      dialogues.push_back(dialogue);
    }
  }
}

void Serializer::serialize_ai_walk_paths(vector<AIWalkPath> &ai_walk_paths) {
  writer.String("ai_walk_paths");
  writer.StartArray();
  for (auto &ai_walk_path : ai_walk_paths) {
    writer.StartObject();
    serialize_uint("delay", ai_walk_path.delay);
    serialize_vec2("target_point", ai_walk_path.target_point);
    writer.EndObject();
  }
  writer.EndArray();
}

void Serializer::deserialize_ai_walk_paths(GenericObject<false, Value> &obj,
                                           vector<AIWalkPath> &ai_walk_paths) {
  // make sure json object has ai_walk_paths first
  if (obj.HasMember("ai_walk_paths")) {
    auto ai_walk_path_array = obj["ai_walk_paths"].GetArray();
    for (auto &ai_walk_path_obj : ai_walk_path_array) {
      AIWalkPath ai_walk_path;
      ai_walk_path.delay = ai_walk_path_obj["delay"].GetUint();
      deserialize_vec2(obj, "target_point", ai_walk_path.target_point);
      ai_walk_paths.push_back(ai_walk_path);
    }
  }
}

void Serializer::serialize_tween_callback(TweenCallback &cb) {
  writer.StartObject();
  serialize_int("cb_type", static_cast<int>(cb.cb_type));
  serialize_string_val("unit_handle", to_string(cb.unit_guid));
  serialize_vec2("tile_point", cb.tile_point);
  serialize_bool("is_final_point_in_path", cb.is_final_point_in_path);
  writer.EndObject();
}

void Serializer::deserialize_tween_callback(Game &game,
                                            GenericObject<false, Value> &obj,
                                            TweenCallback &cb) {
  cb.cb_type = static_cast<TweenCallbackType>(obj["cb_type"].GetInt());
  cb.unit_guid = game.engine.string_gen(obj["unit_handle"].GetString());
  deserialize_vec2(obj, "tile_point", cb.tile_point);
  cb.is_final_point_in_path = obj["is_final_point_in_path"].GetBool();
}

void Serializer::serialize_tile_point_hitbox(Vec2 &hitbox_dims,
                                             Rect &tile_point_hit_box) {
  serialize_vec2("hitbox_dims", hitbox_dims);
  serialize_rect("tile_point_hit_box", tile_point_hit_box);
}

void Serializer::deserialize_tile_point_hitbox(GenericObject<false, Value> &obj,
                                               Vec2 &hitbox_dims,
                                               Rect &tile_point_hit_box) {
  deserialize_vec2(obj, "hitbox_dims", hitbox_dims);
  deserialize_rect(obj, "tile_point_hit_box", tile_point_hit_box);
}

void Serializer::serialize_range(const char *key, Range &range) {
  writer.String(key);
  writer.StartObject();
  serialize_int("lower_bound", range.lower_bound);
  serialize_int("current", range.current);
  serialize_int("current_before_status_effects",
                range.current_before_status_effects);
  serialize_int("max", range.max);
  serialize_int("upper_bound", range.upper_bound);
  writer.EndObject();
}

void Serializer::deserialize_range(GenericObject<false, Value> &obj,
                                   const char *key, Range &range) {
  auto range_obj = obj[key].GetObject();
  range.lower_bound = range_obj["lower_bound"].GetInt();
  range.current = range_obj["current"].GetInt();
  range.current_before_status_effects =
      range_obj["current_before_status_effects"].GetInt();
  range.max = range_obj["max"].GetInt();
  range.upper_bound = range_obj["upper_bound"].GetInt();
}

void Serializer::serialize_stats(const char *key, Stats &stats) {
  writer.String(key);
  writer.StartObject();
  serialize_range("hp", stats.hp);
  serialize_range("action_points", stats.action_points);
  serialize_range("damage", stats.damage);
  serialize_range("range", stats.range);
  serialize_range("aoe", stats.aoe);
  serialize_range("cast_time", stats.cast_time);
  writer.EndObject();
}

void Serializer::deserialize_stats(GenericObject<false, Value> &obj,
                                   const char *key, Stats &stats) {
  auto stats_obj = obj[key].GetObject();
  deserialize_range(stats_obj, "hp", stats.hp);
  deserialize_range(stats_obj, "action_points", stats.action_points);
  deserialize_range(stats_obj, "damage", stats.damage);
  deserialize_range(stats_obj, "range", stats.range);
  deserialize_range(stats_obj, "aoe", stats.aoe);
  deserialize_range(stats_obj, "cast_time", stats.cast_time);
}