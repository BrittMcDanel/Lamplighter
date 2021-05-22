#include "camera.h"
#include "game.h"

void Camera::center_on_rect(const Rect &rect, Vec2 &hitbox_dims, const Vec2 &base_resolution) {
  dst.set(rect.x, rect.y);
  // rect w, h can change as animations play, hitbox dims stay the same regardless
  dst.x -= ((base_resolution.x / 2) - (hitbox_dims.x / 2));
  dst.y -= ((base_resolution.y / 2) - (hitbox_dims.y / 2));
}

void Camera::keep_in_map_bounds(Game &game, int rows, int cols) {
  auto camera_min_x = 0;
  auto camera_max_x =
      (rows * TILE_SIZE) - ((game.engine.base_resolution.x / TILE_SIZE) * TILE_SIZE);
  auto camera_min_y = 0;
  auto camera_max_y =
      (cols * TILE_SIZE) - ((game.engine.base_resolution.y / TILE_SIZE) * TILE_SIZE);
  if (game.engine.camera.dst.x < camera_min_x) {
    game.engine.camera.dst.x = camera_min_x;
  }
  if (game.engine.camera.dst.x > camera_max_x) {
    game.engine.camera.dst.x = camera_max_x;
  }
  if (game.engine.camera.dst.y < camera_min_y) {
    game.engine.camera.dst.y = camera_min_y;
  }
  if (game.engine.camera.dst.y > camera_max_y) {
    game.engine.camera.dst.y = camera_max_y;
  }
}
