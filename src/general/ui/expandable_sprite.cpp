#include "expandable_sprite.h"
#include "game.h"

ExpandableSprite::ExpandableSprite(Game &game, int _corner_size,
                                   vector<TweenableSprite> &_corners,
                                   vector<FixedSprite> &_edges,
                                   FixedSprite &_center) {
  corner_size = _corner_size;
  corners = _corners;
  edges = _edges;
  center = _center;
}

void ExpandableSprite::update(Game &game) {
  for (auto &corner : corners) {
    corner.update(game);
  }
  for (auto &edge : edges) {
    edge.update(game);
  }
  center.update(game);
}

void ExpandableSprite::draw(Game &game) {
  if (is_hidden) {
    return;
  }
  for (auto &corner : corners) {
    corner.draw(game);
  }
  for (auto &edge : edges) {
    edge.draw(game);
  }
  center.draw(game);
}

// opengl bottom-left coordinates rect, so y + h is > y
void ExpandableSprite::update_layout(Game &game, Rect r) {
  assert(corners.size() == 4);
  assert(edges.size() == 4);

  r.h -= corner_size;
  r.w -= corner_size;
  auto top_y = r.y + r.h;
  auto bottom_y = r.y;
  auto left_x = r.x;
  auto right_x = r.x + r.w;
  corners[0].dst.set_xy(left_x, top_y);
  corners[1].dst.set_xy(right_x, top_y);
  corners[2].dst.set_xy(left_x, bottom_y);
  corners[3].dst.set_xy(right_x, bottom_y);
  edges[0].dst =
      Rect(corners[0].dst.x + corner_size, corners[0].dst.y,
           corners[1].dst.x - corners[0].dst.x - corner_size, corner_size);
  edges[1].dst =
      Rect(corners[0].dst.x + corner_size, corners[3].dst.y,
           corners[1].dst.x - corners[0].dst.x - corner_size, corner_size);
  edges[2].dst =
      Rect(corners[0].dst.x, corners[2].dst.y + corner_size, corner_size,
           corners[0].dst.y - corners[2].dst.y - corner_size);
  edges[3].dst =
      Rect(corners[1].dst.x, corners[2].dst.y + corner_size, corner_size,
           corners[0].dst.y - corners[2].dst.y - corner_size);
  center.dst =
      Rect(corners[0].dst.x + corner_size, corners[2].dst.y + corner_size,
           corners[1].dst.x - corners[0].dst.x - corner_size,
           corners[0].dst.y - corners[2].dst.y - corner_size);

  // initial update is needed for some reason or there is a frame where it
  // is obviouslly in the previous state.
  for (auto &corner : corners) {
    corner.update(game);
  }
  for (auto &edge : edges) {
    edge.update(game);
  }
  center.update(game);
}

ExpandableSprite::ExpandableSprite(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  corner_size = 4;
  corners = vector<TweenableSprite>();
  edges = vector<FixedSprite>();
  center = FixedSprite();
  is_hidden = false;

  // top left
  corners.push_back(TweenableSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(0, 0, corner_size, corner_size))),
      },
      100));
  // top right
  corners.push_back(TweenableSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(
              image, Rect(corner_size, 0, corner_size, corner_size))),
      },
      100));
  // bottom left
  corners.push_back(TweenableSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(
              image, Rect(0, corner_size, corner_size, corner_size))),
      },
      100));
  // bottom right
  corners.push_back(TweenableSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(
              image, Rect(corner_size, corner_size, corner_size, corner_size))),
      },
      100));
  // top
  edges.push_back(
      FixedSprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(
                          image, Rect(corner_size * 2, 0, 1, corner_size))),
                  },
                  100));
  // bottom
  edges.push_back(FixedSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(
              image, Rect(corner_size * 2, corner_size, 1, corner_size))),
      },
      100));
  // left
  edges.push_back(
      FixedSprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(
                          image, Rect(0, corner_size * 2, corner_size, 1))),
                  },
                  100));
  // right
  edges.push_back(FixedSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(
              image, Rect(corner_size, corner_size * 2, corner_size, 1))),
      },
      100));
  // center
  center =
      FixedSprite(game, image, Vec2(0, 0),
                  vector<SpriteSrc>{
                      SpriteSrc(ImageLocation(
                          image, Rect(corner_size * 2, corner_size * 2, 1, 1))),
                  },
                  100);
}

void ExpandableSprite::set_is_camera_rendered(bool _is_camera_rendered) {
  for (auto &corner : corners) {
    corner.is_camera_rendered = false;
  }
  for (auto &edge : edges) {
    edge.is_camera_rendered = false;
  }
  center.is_camera_rendered = false;
}