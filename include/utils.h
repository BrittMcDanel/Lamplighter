#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>

#include <boost/uuid/uuid.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

//#include <SDL_opengl.h>

#include "constants.h"

using namespace std;

#define TILE_SIZE 20
#define MOVE_GRID_TILE_SIZE 2
#define MOVE_GRID_RATIO (TILE_SIZE / MOVE_GRID_TILE_SIZE)

struct BoostUUIDHash {
  size_t operator()(const boost::uuids::uuid &uuid) const {
    return hash_value(uuid);
  }
};

struct Vec2 {
  int x = 0;
  int y = 0;
  Vec2();
  Vec2(int _x, int _y);
  void set(int _x, int _y);
  Vec2 operator+(const Vec2 &b) { return Vec2(x + b.x, y + b.y); }
};

struct Vec2HashFunction {
  size_t operator()(const Vec2 &p) const {
    return ((p.x + 10000) << 16) | ((p.y + 10000) & 0xFFFF);
  }
};

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  Rect();
  Rect(int _x, int _y, int _w, int _h);
  void set_xy(int _x, int _y);
  void set_xy(Vec2 _xy);
  Vec2 get_xy();
  Vec2 get_top_left_corner();
  Vec2 get_top_right_corner();
  Vec2 get_bottom_right_corner();
  Vec2 get_top_center();
  Vec2 get_center();
  Vec2 get_bottom_center();
};

class Range {
public:
  int lower_bound;
  int current;
  int current_before_status_effects;
  int max;
  int upper_bound;
  Range() = default;
  Range(int _current);
  void inc_current(int value);
  void dec_current(int value);
  void inc_max(int value);
  void set_current_to_max();
  void set_current_to_lower_bound();
  double get_current_max_pct();
  bool current_equals_lower_bound();
};

struct Stats {
  Range hp;
  Range action_points;
  Range damage;
  Range range;
  Range aoe;
  Range cast_time;
  Stats() = default;
  Stats(Range _hp, Range _action_points, Range _damage, Range _range,
        Range _aoe, Range _cast_time);
};

class DoublePoint {
public:
  double x;
  double y;
  DoublePoint();
  DoublePoint(double _x, double _y);
  DoublePoint operator+(const DoublePoint &b) {
    return DoublePoint(this->x + b.x, this->y + b.y);
  }
  DoublePoint &operator+=(const DoublePoint &b) {
    this->x += b.x;
    this->y += b.y;
    return *this;
  }
  Vec2 to_vec2();
};

class DoubleRect {
public:
  double x;
  double y;
  double w;
  double h;
  DoubleRect();
  DoubleRect(double _x, double _y, double _w, double _h);
  DoubleRect operator+(const DoubleRect &b) {
    return DoubleRect(this->x + b.x, this->y + b.y, this->w + b.w,
                      this->h + b.h);
  }
  DoubleRect &operator+=(const DoubleRect &b) {
    this->x += b.x;
    this->y += b.y;
    this->w += b.w;
    this->h += b.h;
    return *this;
  }
  Rect to_rect();
};

struct CharDst {
  char c;
  Rect dst;
  CharDst();
  CharDst(char _c, Rect _dst);
};

struct EntityInput {
  boost::uuids::uuid guid;
  bool is_mouse_over = false;
  bool clicked = false;
  bool right_clicked = false;
  EntityInput() = default;
  void clear();
};

bool operator==(const Vec2 &v1, const Vec2 &v2);
bool operator!=(const Vec2 &v1, const Vec2 &v2);
bool operator==(const Rect &r1, const Rect &r2);
bool operator!=(const Rect &r1, const Rect &r2);
void set_scaled_rect(Rect &rect, int scale);
void set_scaled_rect_with_camera(Rect &rect, Vec2 &camera_dst, int scale);
Vec2 tile_point_to_world_point(Vec2 idx);
Vec2 tile_point_to_world_point_move_grid(Vec2 idx);
Vec2 world_point_to_tile_point(Vec2 world_point);
Vec2 world_point_to_tile_point_move_grid(Vec2 world_point);
int manhattan_distance(Vec2 v1, Vec2 v2);
bool rect_contains_point(const Rect &r, const Vec2 &p);
bool rect_contains_rect(const Rect &r1, const Rect &r2);
Vec2 oned_to_twod_idx(int idx, int rows);
int twod_to_oned_idx(Vec2 tile_point, int rows);
Vec2 tile_point_to_tile_point_move_grid(Vec2 tile_point);
Vec2 move_grid_point_to_tile_point(Vec2 move_grid_tile_point);
void set_world_point_from_tile_point(Vec2 _tile_point, Rect &dst_to_set);
bool is_player_unit_guid(vector<boost::uuids::uuid> &unit_guids,
                         boost::uuids::uuid unit_guid);
bool is_player_controlled_unit_guid(vector<boost::uuids::uuid> &unit_guids,
                                    boost::uuids::uuid unit_guid);
int get_current_frame_idx(Uint32 current_time, Uint32 spawn_time, int size,
                          int anim_speed);
vector<string> string_split(const string &s, char delim);
void string_trim(string &s);
void string_replace_all(string &str, const string &from, const string &to);
Vec2 get_unit_directional_target(Rect &acting_unit_tile_point_hit_box,
                                 Rect &receiving_unit_tile_point_hit_box);
bool rect_contains_circle(Vec2 circle_center, int radius, Rect &rect);
Faction get_opposite_faction(Faction faction);
int get_path_ap_cost(size_t path_size, int move_indexes_this_turn,
                     int num_moves_this_turn);
int dist(Vec2 v1, Vec2 v2);
void get_bezier_curve(Vec2 cp1, Vec2 cp2, Vec2 cp3, Vec2 cp4,
                      vector<Vec2> &points_in);

#endif // UTILS_H
