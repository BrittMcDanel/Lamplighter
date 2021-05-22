#include "pathfinder.h"
#include "game.h"
#include "map.h"
#include "unit.h"

NodeF::NodeF() {
  p = Vec2(0, 0);
  f = 0;
}

NodeF::NodeF(Vec2 _p, int _f) {
  p = _p;
  f = _f;
}

PathFinder::PathFinder() {
  path = vector<Vec2>();
  pq = priority_queue<NodeF, vector<NodeF>, CompareNodeF>();
  closed_set = unordered_map<Vec2, bool, Vec2HashFunction>();
  came_from = unordered_map<Vec2, Vec2, Vec2HashFunction>();
  g_cost_map = unordered_map<Vec2, int, Vec2HashFunction>();
}

void PathFinder::clear_pq() {
  while (!pq.empty()) {
    pq.pop();
  }
}

// get_path uses Point instead of Vec2 so that I can make == and
// hash functions for the Point type without modifying Vec2. A
// vector of Vec2 is returned however.
void PathFinder::set_path(Game &game, Map &map, Unit &unit, Vec2 start,
                          Vec2 target,
                          bool allow_units_to_path_through_each_other) {
  path_found = false;
  int closest_point_dist = manhattan_distance(start, target);
  closest_point_to_target = start;
  path.clear();
  clear_pq();
  closed_set.clear();
  came_from.clear();
  g_cost_map.clear();
  auto iters = 0;
  auto max_iters = 5000;

  if (!game.map.point_in_bounds_move_grid(target)) {
    return;
  }

  auto start_point = Vec2(start.x, start.y);
  auto target_point = Vec2(target.x, target.y);
  pq.push(NodeF(start_point, manhattan_distance(start, target)));
  g_cost_map[start_point] = 0;
  while (pq.size() > 0 && iters < 5000) {
    iters += 1;

    auto current_pair = pq.top();
    pq.pop();
    auto current = current_pair.p;
    // auto current_height = tiles[current.x][current.y].height;

    // add current to closed set
    if (closed_set.find(current) == closed_set.end()) {
      closed_set[current] = true;
    } else {
      // node is in closed set and has already been considered, continue
      continue;
    }

    // keep track of closest point to the target. Used sometimes if the
    // path is not reached, and you want to move to the cloest point to
    // the target.
    auto dist = manhattan_distance(current, target);
    if (dist < closest_point_dist) {
      closest_point_dist = dist;
      closest_point_to_target = current;
    }

    if (current == target_point) {
      path_found = true;
      // insert final point in front first (will be at end after everything
      // else is inserted)
      path.insert(path.begin(), target);
      auto came_from_iter = came_from.find(current);
      while (came_from_iter != came_from.end()) {
        current = came_from[current];
        // skip start point
        if (current == start) {
          break;
        } else {
          path.insert(path.begin(), current);
          came_from_iter = came_from.find(current);
        }
      }
      return;
    }

    array<Vec2, 8> neighbors = {
        Vec2(current.x + 1, current.y),     Vec2(current.x - 1, current.y),
        Vec2(current.x, current.y + 1),     Vec2(current.x, current.y - 1),
        Vec2(current.x - 1, current.y - 1), Vec2(current.x + 1, current.y + 1),
        Vec2(current.x + 1, current.y - 1), Vec2(current.x - 1, current.y + 1),
    };
    for (auto n : neighbors) {
      auto dist = manhattan_distance(start, n);
      if (closed_set.find(n) != closed_set.end()) {
        continue;
      }
      // the hitbox of the unit if they were to go to this tile
      auto unit_tile_point_hit_box_cpy = unit.sprite.tile_point_hit_box;
      unit_tile_point_hit_box_cpy.x = n.x;
      unit_tile_point_hit_box_cpy.y = n.y;
      auto tile_point = move_grid_point_to_tile_point(n);
      // cout << n.x << " " << n.y << " " << tile_point.x << " " << tile_point.y
      // << "\n";
      auto tile_idx = twod_to_oned_idx(tile_point, game.map.rows);
      if (!game.map.point_in_bounds_move_grid(n)) {
        continue;
      } else if (game.map.tiles[tile_idx].is_obstacle) {
        continue;
      } else if (!allow_units_to_path_through_each_other &&
                 game.map.unit_occupies_tile_point_move_grid(
                     game, unit_tile_point_hit_box_cpy, unit.guid)) {
        continue;
      }
      auto new_g_cost = g_cost_map[current] + 1;
      auto g_cost_iter = g_cost_map.find(n);
      if (g_cost_iter == g_cost_map.end() || new_g_cost < g_cost_map[n]) {
        auto f = manhattan_distance(n, target);
        g_cost_map[n] = new_g_cost;
        pq.push(NodeF(n, f));
        came_from[n] = current;
      }
    }
  }
}