#include "skill_tree_window.h"
#include "game.h"
#include "unit.h"
#include "utils_ui.h"

SkillSlot::SkillSlot(Game &game, const Ability &_ability,
                     vector<AbilityName> _required_abilities, Vec2 _dst) {
  ability = _ability;
  required_abilities = _required_abilities;
  has_skill_slot = create_slot(game);
  does_not_have_skill_slot = create_unlearned_skill_slot(game);
  has_skill_slot.dst.set_xy(_dst);
  does_not_have_skill_slot.dst.set_xy(_dst);
}

void SkillSlot::update(Game &game, Unit &unit) {
  // does linear search over a vector, maybe change to hash table
  // later if this becomes an issue.
  auto has_ability = unit.has_ability(ability.ability_name);
  has_skill_slot.is_hidden =
      game.ui.skill_tree_window.is_hidden || !has_ability;
  does_not_have_skill_slot.is_hidden =
      game.ui.skill_tree_window.is_hidden || has_ability;

  does_not_have_skill_slot.input_events.is_disabled =
      !unit_can_get_ability(game, unit);

  has_skill_slot.update(game);
  does_not_have_skill_slot.update(game);
}

void SkillSlot::draw(Game &game, Unit &unit) {
  has_skill_slot.draw(game);
  does_not_have_skill_slot.draw(game);
  ability.draw_at_dst(game, false,
                      has_skill_slot.dst.get_xy() +
                          Vec2(SLOT_OFFSET_X, SLOT_OFFSET_Y));
}

bool SkillSlot::unit_can_get_ability(Game &game, Unit &unit) {
  if (required_abilities.size() == 0) {
    return true;
  }
  for (auto ability_name : required_abilities) {
    if (unit.has_ability(ability_name)) {
      return true;
    }
  }
  return false;
}

void SkillTreeWindow::update(Game &game, Unit &unit) {
  background.is_hidden = is_hidden;
  background.update(game);
  if (background.input_events.is_mouse_over) {
    game.ui.set_is_mouse_over_ui("skill tree window");
  }
  for (auto &connection : connections) {
    connection.update(game);
  }
  for (auto &corner_connection : corner_connections) {
    corner_connection.update(game);
  }
  for (auto &arrow : arrows) {
    arrow.update(game);
  }
  for (auto &skill_slot : skill_slots) {
    if (skill_slot.ability.ability_name == AbilityName::None) {
      continue;
    }
    skill_slot.update(game, unit);
    // prevent tooltip / drag and drop / adding skills if window is hidden
    if (!is_hidden && skill_slot.ability.ability_name != AbilityName::None) {
      if (skill_slot.has_skill_slot.input_events.is_mouse_over ||
          skill_slot.does_not_have_skill_slot.input_events.is_mouse_over) {
        auto tooltip_dst = skill_slot.has_skill_slot.dst;
        tooltip_dst.x += SLOT_DIM + 1;
        tooltip_dst.y += 1;
        game.ui.ability_tooltip.set_ability_and_dst(game, skill_slot.ability,
                                                    tooltip_dst);
      }
      if (!game.ui.drag_ghost.is_dragging &&
          skill_slot.has_skill_slot.input_events
              .was_mouse_down_when_mouse_over &&
          game.engine.mouse_point_game_rect_scaled !=
              skill_slot.has_skill_slot.input_events
                  .mouse_dst_when_mouse_down) {
        game.ui.drag_ghost.drop_callback.set_as_equip_ability(
            skill_slot.ability, unit.guid);
        game.ui.drag_ghost.start_drag(skill_slot.ability.portrait);
      }
      if (skill_slot.does_not_have_skill_slot.input_events.is_click) {
        unit.add_ability(skill_slot.ability);
      }
    }
  }
}

void SkillTreeWindow::draw(Game &game, Unit &unit) {
  if (is_hidden) {
    return;
  }
  game.engine.start_clipping(background.scaled_dst);
  background.draw(game);
  for (auto &connection : connections) {
    connection.draw(game);
  }
  for (auto &corner_connection : corner_connections) {
    corner_connection.draw(game);
  }
  for (auto &skill_slot : skill_slots) {
    skill_slot.draw(game, unit);
  }
  for (auto &arrow : arrows) {
    arrow.draw(game);
  }
  game.engine.end_clipping();
}

// constructor going down here because of its size
SkillTreeWindow::SkillTreeWindow(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  int background_w = 415;
  int background_h = 280;
  auto background_dst =
      Vec2((game.engine.base_resolution.x / 2) - (background_w / 2), 32);
  background = UISprite(
      game, image,
      Vec2((game.engine.base_resolution.x / 2) - (background_w / 2), 32),
      vector<SpriteSrc>{
          SpriteSrc(
              ImageLocation(image, Rect(208, 713, background_w, background_h))),
      },
      100);
  connections = vector<FixedSprite>();
  corner_connections = vector<UISprite>();
  arrows = vector<UISprite>();
  vertical_connections = unordered_map<int, vector<int>>();
  connection_size = 4;
  arrow_offset_1 = 3;
  arrow_offset_2 = 2;

  auto xs = array<int, 20>();
  auto ys = array<int, 20>();
  auto dst_x = background.dst.x + BACKGROUND_PADDING;
  auto dst_y = background.dst.y + background_h - SLOT_DIM - BACKGROUND_PADDING;
  for (size_t i = 0; i < xs.size(); i++) {
    xs[i] = dst_x;
    dst_x += SLOT_DIM + SLOT_MARGIN_RIGHT;
  }
  for (size_t i = 0; i < ys.size(); i++) {
    ys[i] = dst_y;
    dst_y -= SLOT_DIM + SLOT_MARGIN_BOTTOM;
  }
  skill_slots[static_cast<int>(AbilityName::Fire)] =
      SkillSlot(game, game.assets.get_ability(AbilityName::Fire),
                vector<AbilityName>(), Vec2(xs.at(0), ys.at(0)));
  skill_slots[static_cast<int>(AbilityName::Bullet)] =
      SkillSlot(game, game.assets.get_ability(AbilityName::Bullet),
                vector<AbilityName>(), Vec2(xs.at(8), ys.at(0)));
  skill_slots[static_cast<int>(AbilityName::FireBall)] =
      SkillSlot(game, game.assets.get_ability(AbilityName::FireBall),
                vector<AbilityName>(), Vec2(xs.at(7), ys.at(4)));
  skill_slots[static_cast<int>(AbilityName::SwordSlash)] =
      SkillSlot(game, game.assets.get_ability(AbilityName::SwordSlash),
                vector<AbilityName>(), Vec2(xs.at(2), ys.at(2)));
  skill_slots[static_cast<int>(AbilityName::CounterAttack)] = SkillSlot(
      game, game.assets.get_ability(AbilityName::CounterAttack),
      vector<AbilityName>{AbilityName::Bullet}, Vec2(xs.at(10), ys.at(3)));
  skill_slots[static_cast<int>(AbilityName::Resolve)] = SkillSlot(
      game, game.assets.get_ability(AbilityName::Resolve),
      vector<AbilityName>{AbilityName::Bullet, AbilityName::SwordSlash,
                          AbilityName::FireBall},
      Vec2(xs.at(5), ys.at(6)));

  // visually link all the skills up to their dependencies
  link_skills(game);
}

void SkillTreeWindow::link_skills(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  auto bezier_curve_dsts = vector<Vec2>();
  for (auto &skill_slot : skill_slots) {
    for (auto &required_ability_name : skill_slot.required_abilities) {
      auto &required_skill_slot =
          skill_slots[static_cast<int>(required_ability_name)];
      auto start = required_skill_slot.has_skill_slot.dst.get_xy();
      auto target = skill_slot.has_skill_slot.dst.get_xy();
      start.x += SLOT_DIM / 2 - connection_size / 2;
      start.y += SLOT_DIM / 2 - connection_size / 2;
      target.x += SLOT_DIM / 2 - connection_size / 2;
      target.y += SLOT_DIM / 2 - connection_size / 2;
      int dist_x = abs(start.x - target.x);
      int dist_y = abs(start.y - target.y);
      // same horizontal line
      if (start.y == target.y) {
        add_horizontal_connection(game, start, target);
        auto arrow =
            start.x >= target.x
                ? get_skill_tree_left_arrow(
                      game,
                      target + Vec2(SLOT_DIM - arrow_offset_2, -arrow_offset_2))
                : get_skill_tree_right_arrow(
                      game, target + Vec2(-arrow_offset_1, -arrow_offset_2));
        arrows.push_back(arrow);
      }
      // same vertical line
      else if (start.x == target.x) {
        auto arrow =
            start.y >= target.y
                ? get_skill_tree_down_arrow(game, target + Vec2(-2, -3))
                : get_skill_tree_up_arrow(game, target + Vec2(-2, -3));
        add_vertical_connection(game, start, target);
        arrows.push_back(arrow);
      }
      // not same horizontal or vertical line
      else {
        if (start.y >= target.y) {
          // bottom left
          if (start.x >= target.x) {
            add_left_bottom_connection(game, start, target);
          }
          // bottom right
          else {
            add_right_bottom_connection(game, start, target);
          }
        }
      }
    }
  }
}

void SkillTreeWindow::add_horizontal_connection(Game &game, Vec2 start,
                                                Vec2 target) {
  auto image = game.engine.get_image(ImageName::UI);
  int dist_x = abs(start.x - target.x);
  int dist_y = abs(start.y - target.y);
  if (start.x > target.x) {
    auto temp = start.x;
    start.x = target.x;
    target.x = temp;
  }
  auto fixed_sprite = FixedSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(16, 0, 1, connection_size))),
      },
      100);
  fixed_sprite.dst = Rect(start.x, start.y, dist_x, connection_size);
  connections.push_back(fixed_sprite);
}

void SkillTreeWindow::add_vertical_connection(Game &game, Vec2 start,
                                              Vec2 target) {
  if (vertical_connections.find(target.y) == vertical_connections.end()) {
    vertical_connections[target.y] = vector<int>();
  }
  vertical_connections[target.y].push_back(start.y);
  auto image = game.engine.get_image(ImageName::UI);
  int dist_x = abs(start.x - target.x);
  int dist_y = abs(start.y - target.y);
  if (start.y > target.y) {
    auto temp = start.y;
    start.y = target.y;
    target.y = temp;
  }
  auto fixed_sprite = FixedSprite(
      game, image, Vec2(0, 0),
      vector<SpriteSrc>{
          SpriteSrc(ImageLocation(image, Rect(17, 0, connection_size, 1))),
      },
      100);
  fixed_sprite.dst = Rect(target.x, start.y, connection_size, dist_y);
  connections.push_back(fixed_sprite);
}

/*
      | | start
       |
       |
 | |----  target
*/
void SkillTreeWindow::add_bottom_left_connection(Game &game, Vec2 start,
                                                 Vec2 target) {
  add_vertical_connection(game, start, target);
  add_horizontal_connection(game, target, start);
  corner_connections.push_back(
      get_skill_tree_bottom_left_connection(game, Vec2(start.x, target.y)));
  auto arrow = get_skill_tree_left_arrow(
      game, target + Vec2(SLOT_DIM / 2, -arrow_offset_2));
  arrows.push_back(arrow);
}

/*
  -----| | start
  |
  |
 | |       target
*/
void SkillTreeWindow::add_left_bottom_connection(Game &game, Vec2 start,
                                                 Vec2 target) {
  if (has_higher_vertical_connection(game, start, target)) {
    // add_vertical_connection(game, target, start);
    add_horizontal_connection(game, start,
                              target + Vec2(connection_size - 1, 0));
  } else {
    add_horizontal_connection(game, start, target);
    add_vertical_connection(game, start, target);
    corner_connections.push_back(
        get_skill_tree_top_right_connection(game, Vec2(target.x, start.y)));
    auto arrow = get_skill_tree_down_arrow(
        game, target + Vec2(-arrow_offset_2, SLOT_DIM / 2));
    arrows.push_back(arrow);
  }
}

/*
  | |---  start
       |
       |
      | | target
*/
void SkillTreeWindow::add_right_bottom_connection(Game &game, Vec2 start,
                                                  Vec2 target) {
  if (has_higher_vertical_connection(game, start, target)) {
    // add_vertical_connection(game, target, start);
    add_horizontal_connection(game, start, target + Vec2(1, 0));
  } else {
    add_horizontal_connection(game, start, target);
    add_vertical_connection(game, start, target);
    corner_connections.push_back(
        get_skill_tree_top_left_connection(game, Vec2(target.x, start.y)));
    auto arrow = get_skill_tree_down_arrow(
        game, target + Vec2(-arrow_offset_2, SLOT_DIM / 2));
    arrows.push_back(arrow);
  }
}

bool SkillTreeWindow::has_higher_vertical_connection(Game &game, Vec2 start,
                                                     Vec2 target) {
  if (vertical_connections.find(target.y) != vertical_connections.end()) {
    auto &vec = vertical_connections[target.y];
    for (auto vertical_connection : vec) {
      if (vertical_connection >= start.y) {
        return true;
      }
    }
  }
  return false;
}