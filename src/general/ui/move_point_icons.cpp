#include "ui/move_point_icons.h"
#include "game.h"

MovePointIcons::MovePointIcons(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  icons = vector<Sprite>();
  for (size_t i = 0; i < 500; i++) {
    icons.push_back(
        Sprite(game, image, Vec2(0, 0),
               vector<SpriteSrc>{
                   SpriteSrc(ImageLocation(image, Rect(80, 50, 8, 8))),
               },
               100));
  }
}

void MovePointIcons::update(Game &game) {
  for (auto &icon : icons) {
    icon.update(game);
  }
}

void MovePointIcons::draw(Game &game) {
  for (auto &icon : icons) {
    icon.draw(game);
  }
}

void MovePointIcons::show_path(Game &game, const vector<Vec2> &path) {
  hide_all();
  for (size_t i = 0; i < path.size(); i++) {
    auto &p = path[i];
    if (i < icons.size() && i != 0 && i % 6 == 0) {
      auto &icon = icons[i];
      auto world_point = tile_point_to_world_point_move_grid(p);
      icon.dst.set_xy(world_point.x, world_point.y);
      icon.is_hidden = false;
    }
  }
  // update after setting positions so there isn't a frame of delay
  // (it looks strange if you don't update here)
  update(game);
}

void MovePointIcons::hide_all() {
  for (auto &icon : icons) {
    icon.is_hidden = true;
  }
}