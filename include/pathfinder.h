#ifndef PATHFINDER_H
#define PATHFINDER_H
#include "utils.h"
#include <SDL.h>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

struct Game;
struct Map;
struct Unit;

struct NodeF {
  Vec2 p;
  int f;
  NodeF();
  NodeF(Vec2 _p, int _f);
};

struct CompareNodeF {
  bool operator()(const NodeF &n1, const NodeF &n2) const {
    return n1.f > n2.f;
  }
};

class PathFinder {
public:
  bool path_found;
  Vec2 closest_point_to_target;
  vector<Vec2> path;
  priority_queue<NodeF, vector<NodeF>, CompareNodeF> pq;
  unordered_map<Vec2, bool, Vec2HashFunction> closed_set;
  unordered_map<Vec2, Vec2, Vec2HashFunction> came_from;
  unordered_map<Vec2, int, Vec2HashFunction> g_cost_map;
  PathFinder();
  void set_path(Game &game, Map &map, Unit &unit, Vec2 start, Vec2 target,
                bool allow_units_to_path_through_each_other = true);
  void clear_pq();
};

#endif // PATHFINDER_H
