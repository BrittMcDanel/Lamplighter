#include "utils.h"
#include <iostream>
#include <math.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <streambuf>
#include <string>

using namespace std;

Vec2::Vec2() {
  x = 0;
  y = 0;
}

Vec2::Vec2(int _x, int _y) {
  x = _x;
  y = _y;
}

void Vec2::set(int _x, int _y) {
  x = _x;
  y = _y;
}

bool operator==(const Vec2 &v1, const Vec2 &v2) {
  return v1.x == v2.x && v1.y == v2.y;
}
bool operator!=(const Vec2 &v1, const Vec2 &v2) {
  return !(v1.x == v2.x && v1.y == v2.y);
}

Rect::Rect() {
  x = 0;
  y = 0;
  w = 0;
  h = 0;
}

Rect::Rect(int _x, int _y, int _w, int _h) {
  x = _x;
  y = _y;
  w = _w;
  h = _h;
}

void Rect::set_xy(int _x, int _y) {
  x = _x;
  y = _y;
}

void Rect::set_xy(Vec2 _xy) {
  x = _xy.x;
  y = _xy.y;
}

Vec2 Rect::get_xy() { return Vec2(x, y); }

Vec2 Rect::get_top_left_corner() { return Vec2(x, y + h); }

Vec2 Rect::get_top_right_corner() { return Vec2(x + w, y + h); }

Vec2 Rect::get_bottom_right_corner() { return Vec2(x + w, y); }

Vec2 Rect::get_top_center() { return Vec2(x + (w / 2), y + h); }

Vec2 Rect::get_center() { return Vec2(x + (w / 2), y + (h / 2)); }

Vec2 Rect::get_bottom_center() { return Vec2(x + (w / 2), y); }

bool operator==(const Rect &r1, const Rect &r2) {
  return r1.x == r2.x && r1.y == r2.y && r1.w == r2.w && r1.h == r2.h;
}

bool operator!=(const Rect &r1, const Rect &r2) {
  return !(r1.x == r2.x && r1.y == r2.y && r1.w == r2.w && r1.h == r2.h);
}

Range::Range(int _current) {
  lower_bound = 0;
  current = _current;
  current_before_status_effects = _current;
  max = _current;
  upper_bound = _current;
}

void Range::inc_current(int value) {
  current += value;
  if (current > max) {
    current = max;
  }
}

void Range::dec_current(int value) {
  current -= value;
  if (current < lower_bound) {
    current = lower_bound;
  }
}

void Range::inc_max(int value) {
  max += value;
  if (max > upper_bound) {
    max = upper_bound;
  }
}

void Range::set_current_to_max() { current = max; }

void Range::set_current_to_lower_bound() { current = lower_bound; }

double Range::get_current_max_pct() { return (double)current / (double)max; }

bool Range::current_equals_lower_bound() { return current == lower_bound; }

Stats::Stats(Range _hp, Range _action_points, Range _damage, Range _range,
             Range _aoe, Range _cast_time) {
  hp = _hp;
  action_points = _action_points;
  damage = _damage;
  range = _range;
  aoe = _aoe;
  cast_time = _cast_time;

  // set upper bounds on all stats
  hp.upper_bound = HP_MAX;
  action_points.upper_bound = ACTION_POINT_MAX;
  damage.upper_bound = DAMAGE_STAT_MAX;
  range.upper_bound = RANGE_MAX;
  aoe.upper_bound = AOE_MAX;
  cast_time.upper_bound = CAST_TIME_MAX;
}

DoublePoint::DoublePoint() {
  x = 0.0;
  y = 0.0;
}

DoublePoint::DoublePoint(double _x, double _y) {
  x = _x;
  y = _y;
}

Vec2 DoublePoint::to_vec2() { return Vec2((int)x, (int)y); }

DoubleRect::DoubleRect() {
  x = 0.0;
  y = 0.0;
  w = 0.0;
  h = 0.0;
}

DoubleRect::DoubleRect(double _x, double _y, double _w, double _h) {
  x = _x;
  y = _y;
  w = _w;
  h = _h;
}

Rect DoubleRect::to_rect() { return Rect((int)x, (int)y, (int)w, (int)h); }

CharDst::CharDst() {
  c = ' ';
  dst = Rect(0, 0, 0, 0);
}

CharDst::CharDst(char _c, Rect _dst) {
  c = _c;
  dst = _dst;
}

void EntityInput::clear() {
  is_mouse_over = false;
  clicked = false;
  right_clicked = false;
}

void set_scaled_rect(Rect &rect, int scale) {
  rect.x *= scale;
  rect.y *= scale;
  rect.w *= scale;
  rect.h *= scale;
}

void set_scaled_rect_with_camera(Rect &rect, Vec2 &camera_dst, int scale) {
  auto p = Vec2(rect.x, rect.y);
  p.x -= camera_dst.x;
  p.y -= camera_dst.y;
  p.x *= scale;
  p.y *= scale;
  rect.x = p.x;
  rect.y = p.y;
  rect.w *= scale;
  rect.h *= scale;
}

bool rect_contains_point(const Rect &r, const Vec2 &p) {
  return p.x >= r.x && p.x <= r.x + r.w && p.y >= r.y && p.y <= r.y + r.h;
}

bool rect_contains_rect(const Rect &r1, const Rect &r2) {
  return r1.x < r2.x + r2.w && r1.x + r1.w > r2.x && r1.y < r2.y + r2.h &&
         r1.y + r1.h > r2.y;
}

Vec2 tile_point_to_world_point_move_grid(Vec2 tile_point) {
  return Vec2(tile_point.x * MOVE_GRID_TILE_SIZE,
              tile_point.y * MOVE_GRID_TILE_SIZE);
}

Vec2 world_point_to_tile_point_move_grid(Vec2 world_point) {
  world_point.x /= MOVE_GRID_TILE_SIZE;
  world_point.y /= MOVE_GRID_TILE_SIZE;
  return world_point;
}

Vec2 tile_point_to_world_point(Vec2 tile_point) {
  return Vec2(tile_point.x * TILE_SIZE, tile_point.y * TILE_SIZE);
}

Vec2 world_point_to_tile_point(Vec2 world_point) {
  world_point.x /= TILE_SIZE;
  world_point.y /= TILE_SIZE;
  return world_point;
}

int manhattan_distance(Vec2 v1, Vec2 v2) {
  return abs(v1.x - v2.x) + abs(v1.y - v2.y);
}

Vec2 oned_to_twod_idx(int idx, int rows) {
  return Vec2(idx % rows, idx / rows);
}

int twod_to_oned_idx(Vec2 tile_point, int rows) {
  return (tile_point.x * rows) + tile_point.y;
}

Vec2 tile_point_to_tile_point_move_grid(Vec2 tile_point) {
  return Vec2(tile_point.x * MOVE_GRID_RATIO, tile_point.y * MOVE_GRID_RATIO);
}

Vec2 move_grid_point_to_tile_point(Vec2 move_grid_tile_point) {
  return Vec2(move_grid_tile_point.x / MOVE_GRID_RATIO,
              move_grid_tile_point.y / MOVE_GRID_RATIO);
}

// sets given dst to the world dst at tile point. Tile point is a
// regular tile point, not a move grid tile point.
void set_world_point_from_tile_point(Vec2 _tile_point, Rect &dst_to_set) {
  auto world_point = tile_point_to_world_point(_tile_point);
  dst_to_set.x = world_point.x;
  dst_to_set.y = world_point.y;
}

// is the unit guid the player's unit
bool is_player_unit_guid(vector<boost::uuids::uuid> &unit_guids,
                         boost::uuids::uuid unit_guid) {
  return unit_guids.size() > 0 && unit_guids[0] == unit_guid;
}

// is the unit_guid the player's unit or a unit the player controls
bool is_player_controlled_unit_guid(vector<boost::uuids::uuid> &unit_guids,
                                    boost::uuids::uuid unit_guid) {
  return find(unit_guids.begin(), unit_guids.end(), unit_guid) !=
         unit_guids.end();
}

int get_current_frame_idx(Uint32 current_time, Uint32 spawn_time, int size,
                          int anim_speed) {
  auto alive_time = current_time - spawn_time;
  auto current_frame = 0;
  if (size > 0) {
    current_frame = (alive_time / anim_speed) % size;
  }
  return current_frame;
}

Vec2 get_unit_directional_target(Rect &acting_unit_tile_point_hit_box,
                                 Rect &receiving_unit_tile_point_hit_box) {
  if (acting_unit_tile_point_hit_box.x <= receiving_unit_tile_point_hit_box.x) {
    if (acting_unit_tile_point_hit_box.y <=
        receiving_unit_tile_point_hit_box.y) {
      return receiving_unit_tile_point_hit_box.get_xy();
    } else {
      return receiving_unit_tile_point_hit_box.get_top_left_corner();
    }
  } else {
    if (acting_unit_tile_point_hit_box.y <=
        receiving_unit_tile_point_hit_box.y) {
      return receiving_unit_tile_point_hit_box.get_bottom_right_corner();
    } else {
      return receiving_unit_tile_point_hit_box.get_top_right_corner();
    }
  }

  return receiving_unit_tile_point_hit_box.get_xy();
}

// return true if the rectangle and circle are colliding
bool rect_contains_circle(Vec2 circle_center, int radius, Rect &rect) {
  auto distX = abs(circle_center.x - rect.x - rect.w / 2);
  auto distY = abs(circle_center.y - rect.y - rect.h / 2);

  if (distX > (rect.w / 2 + radius)) {
    return false;
  }
  if (distY > (rect.h / 2 + radius)) {
    return false;
  }

  if (distX <= (rect.w / 2)) {
    return true;
  }
  if (distY <= (rect.h / 2)) {
    return true;
  }

  auto dx = distX - rect.w / 2;
  auto dy = distY - rect.h / 2;
  return (dx * dx + dy * dy <= (radius * radius));
}

vector<string> string_split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> elems;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(move(item)); // if C++11 (based on comment from
    // @mchiasson)
  }
  return elems;
}

void string_trim(string &s) {
  // leading spaces
  while (!s.empty() && std::isspace(*s.begin()))
    s.erase(s.begin());
  // trailing spaces
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

void string_replace_all(string &str, const string &from, const string &to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

Faction get_opposite_faction(Faction faction) {
  if (faction == Faction::Ally) {
    return Faction::Enemy;
  } else {
    return Faction::Ally;
  }
}

int get_path_ap_cost(size_t path_size, int move_indexes_this_turn,
                     int num_moves_this_turn) {
  int ap_cost =
      (move_indexes_this_turn + (int)path_size) / MOVE_INDEXES_PER_ACTION_POINT;
  if (ap_cost == 0 && num_moves_this_turn == 0) {
    ap_cost = 1;
  }
  return ap_cost;
}

int dist(Vec2 v1, Vec2 v2) {
  double delta_x = v1.x - v2.x;
  double delta_y = v1.y - v2.y;
  return (int)sqrt(delta_x * delta_x + delta_y * delta_y);
}

void get_bezier_curve(Vec2 cp1, Vec2 cp2, Vec2 cp3, Vec2 cp4,
                      vector<Vec2> &points_in) {
  points_in.clear();
  for (double u = 0.0; u <= 1.0; u += 0.01) {
    auto xu = pow(1 - u, 3) * cp1.x + 3 * u * pow(1 - u, 2) * cp2.x +
              3 * pow(u, 2) * (1 - u) * cp3.x + pow(u, 3) * cp4.x;
    auto yu = pow(1 - u, 3) * cp1.y + 3 * u * pow(1 - u, 2) * cp2.y +
              3 * pow(u, 2) * (1 - u) * cp3.y + pow(u, 3) * cp4.y;
    // cout << (int)xu << " " << (int)yu << "\n";
    points_in.push_back(Vec2(xu, yu));
  }
}