#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "engine.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <SDL.h>
#include <vector>

using namespace std;
using namespace rapidjson;

// forward declaring everything that gets serialized/deserialized,
// don't want any conflicts here.
struct Game;
struct Dialogue;
struct AIWalkPath;
struct TweenCallback;

struct Serializer {
  Document doc;
  StringBuffer sb;
  Writer<StringBuffer> writer;
  Serializer();
  void clear();
  void serialize_int(const char *key, int value);
  void serialize_uint(const char *key, Uint32 value);
  void serialize_double(const char *key, double value);
  void serialize_bool(const char *key, bool value);
  void serialize_string(const char *key, string &value);
  void serialize_string_val(const char *key, string value);
  void serialize_rect(const char *key, Rect &value);
  void serialize_vec2(const char *key, Vec2 &value);
  void serialize_double_point(const char *key, DoublePoint &value);
  void serialize_sprite_src_vec(const char *key, vector<SpriteSrc> &srcs);
  void serialize_dialogues(vector<Dialogue> &dialogues);
  void serialize_ai_walk_paths(vector<AIWalkPath> &ai_walk_paths);
  void serialize_tween_callback(TweenCallback &cb);
  void serialize_tile_point_hitbox(Vec2 &hitbox_dims, Rect &tile_point_hit_box);
  void serialize_range(const char *key, Range &range);
  void serialize_stats(const char *key, Stats &stats);
  void deserialize_rect(GenericObject<false, Value> &obj, const char *key,
                        Rect &value);
  void deserialize_vec2(GenericObject<false, Value> &obj, const char *key,
                        Vec2 &value);
  void deserialize_double_point(GenericObject<false, Value> &obj,
                                const char *key, DoublePoint &value);
  void deserialize_sprite_src_vec(GenericObject<false, Value> &obj,
                                  Image &image, const char *key,
                                  vector<SpriteSrc> &srcs);
  void deserialize_dialogues(GenericObject<false, Value> &obj,
                             vector<Dialogue> &dialogues);
  void deserialize_ai_walk_paths(GenericObject<false, Value> &obj,
                                 vector<AIWalkPath> &ai_walk_paths);
  void deserialize_tween_callback(Game &game, GenericObject<false, Value> &obj,
                                  TweenCallback &cb);
  void deserialize_tile_point_hitbox(GenericObject<false, Value> &obj,
                                     Vec2 &hitbox_dims,
                                     Rect &tile_point_hit_box);
  void deserialize_range(GenericObject<false, Value> &obj, const char *key,
                         Range &range);
  void deserialize_stats(GenericObject<false, Value> &obj, const char *key,
                         Stats &stats);
};

#endif // SERIALIZER_H