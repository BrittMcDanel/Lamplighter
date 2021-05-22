#include "crosshairs.h"
#include "game.h"

Crosshairs::Crosshairs(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  auto crosshair_size = 7;
  top_left = Sprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 34, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 34, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 34, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 34, crosshair_size, crosshair_size))),
      },
      100);
  top_right =
      Sprite(game, image, Vec2(0, 0),
             vector<SpriteSrc>{
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 34, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 34, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 34, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 34, crosshair_size, crosshair_size))),
             },
             100);
  bottom_left = Sprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 47, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 47, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 47, crosshair_size, crosshair_size))),
          SpriteSrc(ImageLocation(image,
                                  Rect(0, 47, crosshair_size, crosshair_size))),
      },
      100);
  bottom_right =
      Sprite(game, image, Vec2(0, 0),
             vector<SpriteSrc>{
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 47, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 47, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 47, crosshair_size, crosshair_size))),
                 SpriteSrc(ImageLocation(
                     image, Rect(13, 47, crosshair_size, crosshair_size))),
             },
             100);
  // defaults to hidden initially
  set_is_hidden(true);
}

void Crosshairs::update(Game &game, Rect &dst) {
  // size - 1
  auto num_frames = 3;
  auto margin = 1 * num_frames;
  top_left.dst.set_xy(dst.x - margin, dst.y + dst.h);
  top_right.dst.set_xy(dst.x + dst.w - margin, dst.y + dst.h);
  bottom_left.dst.set_xy(dst.x - margin, dst.y - margin);
  bottom_right.dst.set_xy(dst.x + dst.w - margin, dst.y - margin);
  // easier just to change dst x y values than to animate subrects
  // moves the crosshair corner based on the anim current frame idx
  // all srcs are always the same for each corner, only used for the
  // current frame idx to shift it here
  top_left.dst.x += top_left.current_frame_idx;
  top_left.dst.y -= top_left.current_frame_idx;
  top_right.dst.x -= top_right.current_frame_idx;
  top_right.dst.y -= top_right.current_frame_idx;
  bottom_left.dst.x += bottom_left.current_frame_idx;
  bottom_left.dst.y += bottom_left.current_frame_idx;
  bottom_right.dst.x -= bottom_left.current_frame_idx;
  bottom_right.dst.y += bottom_left.current_frame_idx;
  top_left.update(game);
  top_right.update(game);
  bottom_left.update(game);
  bottom_right.update(game);
}

void Crosshairs::draw(Game &game) {
  top_left.draw(game);
  top_right.draw(game);
  bottom_left.draw(game);
  bottom_right.draw(game);
}

void Crosshairs::set_is_hidden(bool _is_hidden) {
  is_hidden = _is_hidden;
  top_left.is_hidden = _is_hidden;
  top_right.is_hidden = _is_hidden;
  bottom_left.is_hidden = _is_hidden;
  bottom_right.is_hidden = _is_hidden;
}