#include "ui/unit_ui.h"
#include "game.h"

UnitUIBeforeUnit::UnitUIBeforeUnit(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  move_icon = Sprite(game, image, Vec2(0, 0),
                     vector<SpriteSrc>{
                         SpriteSrc(ImageLocation(image, Rect(0, 64, 30, 30))),
                         SpriteSrc(ImageLocation(image, Rect(30, 64, 30, 30))),
                         SpriteSrc(ImageLocation(image, Rect(60, 64, 30, 30))),
                         SpriteSrc(ImageLocation(image, Rect(30, 64, 30, 30))),
                     },
                     120);
  // hide the icon until the player moves
  move_icon.is_hidden = true;
}

void UnitUIBeforeUnit::update(Game &game, Unit &unit) {
  move_icon.update(game);
}

void UnitUIBeforeUnit::draw(Game &game) { move_icon.draw(game); }

UnitUIAfterUnit::UnitUIAfterUnit(Game &game) {
  auto image = game.engine.get_image(ImageName::UI);
  dialogue_box = DialogueBox(game);
  ability_box = DialogueBox(game, false, false);
  crosshairs = Crosshairs(game);
  active_arrow =
      Sprite(game, image, Vec2(0, 0),
             vector<SpriteSrc>{
                 SpriteSrc(ImageLocation(image, Rect(112, 64, 12, 12))),
                 SpriteSrc(ImageLocation(image, Rect(112, 64, 12, 13))),
                 SpriteSrc(ImageLocation(image, Rect(112, 64, 12, 14))),
             },
             120);
  turn_order_arrow =
      Sprite(game, image, Vec2(0, 0),
             vector<SpriteSrc>{
                 SpriteSrc(ImageLocation(image, Rect(124, 64, 12, 12))),
                 SpriteSrc(ImageLocation(image, Rect(124, 64, 12, 13))),
                 SpriteSrc(ImageLocation(image, Rect(124, 64, 12, 14))),
             },
             120);
  turn_order_arrow.is_hidden = true;
}

void UnitUIAfterUnit::update(Game &game, Unit &unit) {
  dialogue_box.update(game, unit);
  ability_box.update(game, unit);
  // always set to hidden at the start of the frame, other functions
  // can show it after unit has been updated
  crosshairs.set_is_hidden(true);
  crosshairs.update(game, unit.sprite.dst);
  active_arrow.dst = unit.sprite.dst;
  active_arrow.dst.x += 4;
  active_arrow.dst.y += unit.sprite.dst.h + 4;
  active_arrow.is_hidden = !unit.in_battle;
  turn_order_arrow.dst = active_arrow.dst;
  if (unit.in_battle) {
    auto &battle = game.map.battle_dict[unit.battle_guid];
    active_arrow.is_hidden = battle.acting_unit_guid != unit.guid;
  }
  active_arrow.update(game);
  turn_order_arrow.update(game);
}

void UnitUIAfterUnit::draw(Game &game) {
  dialogue_box.draw(game);
  active_arrow.draw(game);
  turn_order_arrow.draw(game);
  crosshairs.draw(game);
  ability_box.draw(game);
}

UnitUILastLayer::UnitUILastLayer(Game &game) { battle_texts = BattleTexts(); }

void UnitUILastLayer::update(Game &game, Unit &unit) {
  battle_texts.update(game, unit);
}

void UnitUILastLayer::draw(Game &game) { battle_texts.draw(game); }