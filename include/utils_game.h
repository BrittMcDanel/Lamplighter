#ifndef UTILS_GAME_H
#define UTILS_GAME_H

#include <SDL.h>

#include "engine.h"
#include "item.h"
#include "ui/ui_sprite.h"
#include "utils.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

//#include <SDL_opengl.h>

#include "constants.h"

using namespace std;

struct Game;
struct Unit;
struct Ability;

struct Equipment {
  Item primary_hand;
  Item secondary_hand;
  Item head;
  Item armor;
  Item boots;
  Equipment() = default;
  Equipment(Game &game);
};

void draw_sprite_at_dst(Game &game, Image image, vector<SpriteSrc> &srcs,
                        Uint32 spawn_time, Uint32 anim_speed,
                        bool is_camera_rendered, Vec2 dst_xy);
void draw_sprite_at_dst(Game &game, Image image, SpriteSrc &current_src,
                        bool is_camera_rendered, Vec2 dst_xy);
void draw_text_at_dst(Game &game, int font_handle, string &str,
                      TextAlignment alignment, int scaled_max_width,
                      bool is_camera_rendered, Vec2 dst_xy);
bool is_mouse_over_dst(Game &game, bool _is_camera_rendered, const Rect &dst);
Item get_coin_item(Game &game);
Item get_cash_item(Game &game);
const Ability &get_default_attack_ability(Game &game, Unit &unit);
bool is_ability_in_range(const Ability &ability, Vec2 start_dst,
                         Vec2 target_dst);

#endif // UTILS_GAME_H
