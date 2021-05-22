#ifndef TWEEN_H
#define TWEEN_H
#include "rapidjson/document.h"
#include <SDL.h>
#include <boost/uuid/uuid.hpp>

#include <functional>
#include <vector>

#include "utils.h"
using namespace std;
using namespace rapidjson;

struct Game;

enum class TweenInterpType {
  Linear,
  QuadraticIn,
};

struct TweenCompletion {
  bool started;
  bool completed;
  TweenCompletion();
};

struct TweenCallback {
  TweenCallbackType cb_type = TweenCallbackType::None;
  boost::uuids::uuid unit_guid;
  Vec2 tile_point = Vec2(0, 0);
  bool is_final_point_in_path = false;
  boost::uuids::uuid item_guid;
  boost::uuids::uuid panel_guid;
  int idx = 0;
  int handle = 0;
  int damage = 1;
  PerformAbilityContext ability_context = PerformAbilityContext::Default;
  vector<boost::uuids::uuid> receiving_unit_guids =
      vector<boost::uuids::uuid>();
  Vec2 target_dst = Vec2(0, 0);
  AbilityName ability_name;
  void set_as_unit_move_callback(boost::uuids::uuid _unit_guid,
                                 Vec2 _tile_point,
                                 bool _is_final_point_in_path);
  void set_as_send_item_to_unit_callback(boost::uuids::uuid _unit_guid,
                                         boost::uuids::uuid _item_guid);
  void set_as_item_pickup_display_complete(boost::uuids::uuid _panel_guid);
  void set_as_battle_text(boost::uuids::uuid _unit_guid, int _handle);
  void set_as_use_ability(boost::uuids::uuid _acting_unit_guid,
                          vector<boost::uuids::uuid> _receiving_unit_guids,
                          int _ability_handle, int _damage, Vec2 _target_dst,
                          PerformAbilityContext _ability_context);
  void
  set_as_use_ability_timeout(boost::uuids::uuid _acting_unit_guid,
                             vector<boost::uuids::uuid> _receiving_unit_guids,
                             AbilityName _ability_name, int _damage,
                             Vec2 _target_dst,
                             PerformAbilityContext _ability_context);
};

struct TweenXY {
  TweenCallback callback;
  Rect start_val;
  Rect target_val;
  Uint32 spawn_time;
  Uint32 duration;
  Uint32 delay;
  Uint32 current_time_spawn_time_delta;
  DoublePoint double_point;
  bool has_started;
  std::function<void()> on_start;
  std::function<void()> on_complete;
  TweenXY() = default;
  // tweens from start_val to target_val in duration after delay
  TweenXY(Rect _start_val, Rect _target_val, Uint32 current_time,
          Uint32 _duration, Uint32 _delay, TweenCallback _callback,
          std::function<void()> _on_start, std::function<void()> _on_complete);
  TweenCompletion update(Game &game, Rect &val);
};

struct TweenXYConstantSpeed {
  TweenCallback callback;
  Rect start_val;
  Rect target_val;
  double speed;
  Uint32 spawn_time;
  Uint32 delay;
  Uint32 current_time_spawn_time_delta;
  DoublePoint double_point;
  bool has_started;
  std::function<void()> on_start;
  std::function<void()> on_complete;
  TweenXYConstantSpeed() = default;
  // tweens from start val to target val in a constant speed after delay
  TweenXYConstantSpeed(Rect _start_val, Rect _target_val, Uint32 current_time,
                       Uint32 _delay, double _speed, TweenCallback _callback,
                       std::function<void()> _on_start,
                       std::function<void()> _on_complete);
  TweenCompletion update(Game &game, Rect &val);
};

struct TweenXYSpeedMovingTarget {
  TweenCallback callback;
  TweenInterpType tween_interp_type;
  Rect start_val;
  Rect target_val;
  double speed;
  bool is_constant_speed;
  Uint32 spawn_time;
  Uint32 delay;
  Uint32 current_time_spawn_time_delta;
  DoublePoint double_point;
  bool has_started;
  TweenMovingTargetType moving_target_type = TweenMovingTargetType::None;
  boost::uuids::uuid moving_target_guid;
  std::function<void()> on_start;
  std::function<void()> on_complete;
  TweenXYSpeedMovingTarget();
  // tweens from start val to a moving target val in a constant speed after
  // delay
  TweenXYSpeedMovingTarget(Rect _start_val, Rect _target_val,
                           TweenInterpType _tween_interp_type,
                           Uint32 current_time, Uint32 _delay, double _speed,
                           TweenMovingTargetType _moving_target_type,
                           boost::uuids::uuid _moving_target_guid,
                           TweenCallback _callback,
                           std::function<void()> _on_start,
                           std::function<void()> _on_complete);
  TweenCompletion update(Game &game, Rect &val, Rect &_target_val);
};

struct Tweens {
  vector<TweenXY> tween_xys;
  vector<TweenXYConstantSpeed> tween_xys_constant_speed;
  vector<TweenXYSpeedMovingTarget> tween_xys_speed_moving_target;
  void update(Game &game, Rect &val);
  void clear();
  Tweens() = default;
};

void handle_tween_on_start(Game &game, TweenCallback &cb);
void handle_tween_on_complete(Game &game, TweenCallback &cb);

double tween_interp_update(double speed, TweenInterpType tween_interp_type);

void tweens_serialize(Game &game, Tweens &tweens);
void tweens_serialize_into_file(Game &game, Tweens &tweens,
                                const char *file_path);
Tweens tweens_deserialize_from_file(Game &game, const char *file_path);
Tweens tweens_deserialize(Game &game, GenericObject<false, Value> &obj);

#endif // TWEEN_H