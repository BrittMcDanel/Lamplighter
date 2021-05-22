#ifndef UI_SKILLTREEWINDOW_H
#define UI_SKILLTREEWINDOW_H

#include "ability.h"
#include "fixed_sprite.h"
#include "inventory.h"
#include "tween.h"
#include "ui/ui_sprite.h"
#include "ui/utils_ui.h"
#include "utils.h"
#include <SDL.h>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct Game;
struct Unit;

struct SkillSlot {
  Ability ability = Ability();
  vector<AbilityName> required_abilities = vector<AbilityName>();
  UISprite has_skill_slot = UISprite();
  UISprite does_not_have_skill_slot = UISprite();
  SkillSlot() = default;
  SkillSlot(Game &game, const Ability &_ability,
            vector<AbilityName> _required_abilities, Vec2 _dst);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
  bool unit_can_get_ability(Game &game, Unit &unit);
};

struct SkillTreeWindow {
  array<SkillSlot, ABILITY_NAME_LAST> skill_slots =
      array<SkillSlot, ABILITY_NAME_LAST>();
  UISprite background = UISprite();
  vector<FixedSprite> connections = vector<FixedSprite>();
  vector<UISprite> corner_connections = vector<UISprite>();
  vector<UISprite> arrows = vector<UISprite>();
  unordered_map<int, vector<int>> vertical_connections;
  int connection_size;
  int arrow_offset_1;
  int arrow_offset_2;
  bool is_hidden = true;
  SkillTreeWindow() = default;
  SkillTreeWindow(Game &game);
  void update(Game &game, Unit &unit);
  void draw(Game &game, Unit &unit);
  void add_horizontal_connection(Game &game, Vec2 start, Vec2 target);
  void add_vertical_connection(Game &game, Vec2 start, Vec2 target);
  void add_bottom_left_connection(Game &game, Vec2 start, Vec2 target);
  void add_left_bottom_connection(Game &game, Vec2 start, Vec2 target);
  void add_right_bottom_connection(Game &game, Vec2 start, Vec2 target);
  bool has_higher_vertical_connection(Game &game, Vec2 start, Vec2 target);
  void link_skills(Game &game);
};

#endif // UI_SKILLTREEWINDOW_H