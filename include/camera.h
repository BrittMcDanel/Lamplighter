#ifndef CAMERA_H
#define CAMERA_H

#include "constants.h"
#include "utils.h"

struct Game;

struct Camera {
  Vec2 dst = Vec2(0, 0);
  CameraMode camera_mode = CameraMode::FollowPlayer;
  void center_on_rect(const Rect &rect, Vec2 &hitbox_dims,
                      const Vec2 &base_resolution);
  void keep_in_map_bounds(Game &game, int rows, int cols);
};

#endif // CAMERA_H