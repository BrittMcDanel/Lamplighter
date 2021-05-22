#include "ui/item_pickup_list.h"
#include "game.h"

ItemPickupPanel::ItemPickupPanel(Game &game, Item &_item, int _idx) {
  guid = game.engine.get_guid();
  idx = _idx;
  margin_from_window = Vec2(8, 34);
  item_dim = 20;
  padding = 4;
  margin_top = 2;
  height = item_dim + (padding * 2);
  item = _item;
  // clear any tweens the item may have had
  item.sprite.tweens.clear();
  auto dst_xy = get_dst(game);
  Rect dst = Rect(dst_xy.x, dst_xy.y, 0, 0);
  background = ExpandableSprite(game);
  display_name_text = Text(game, 10, FontColor::WhiteShadow, "", Vec2(0, 0),
                           150, TextAlignment::Left);
  item.set_is_camera_rendered(false);
  background.set_is_camera_rendered(false);
  display_name_text.is_camera_rendered = false;

  tweens = Tweens();
  set_complete_state_tween(game);

  // call an update so that everything is positioned before a draw
  // or there is a frame of badly positioned elements
  update(game);
}

void ItemPickupPanel::update(Game &game) {
  display_name_text.str = item.display_name;
  auto display_name_dims =
      display_name_text.measure_text(game, display_name_text.str);
  dst.w = item_dim + display_name_dims.x + padding * 2 + 5;
  dst.h = height;
  background.update_layout(game, dst);
  item.sprite.dst.set_xy(dst.x + padding, dst.y + padding);
  item_dst = Vec2(dst.x + padding, dst.y + padding);
  display_name_text.dst.set_xy(dst.x + item_dim + (padding * 2),
                               dst.y + height - (padding * 2));
  background.update(game);
  // item.update(game); // causes a tween bug from the previous item
  display_name_text.update(game);
  tweens.update(game, dst);
}

void ItemPickupPanel::draw(Game &game) {
  background.draw(game);
  item.draw_at_dst(game, false, item_dst);
  display_name_text.draw(game);
}

Vec2 ItemPickupPanel::get_dst(Game &game) {
  return Vec2(margin_from_window.x,
              margin_from_window.y + (height * idx) + (margin_top * idx));
}

// animation tween to show the panel being cleared
void ItemPickupPanel::set_complete_state_tween(Game &game) {
  // clear any previous complete tweens
  tweens.clear();
  // other panels can take this panel's spot while its tweening away
  // is_in_completed_state = true;
  auto dst_xy = get_dst(game);
  auto start_rect = Rect(dst_xy.x, dst_xy.y, 0, 0);
  auto target_rect = start_rect;
  target_rect.x -= 150;
  dst = start_rect;
  remove_time = game.engine.current_time + 3000;
  /*
  // this tween is causing segfaults on complete, fix later
  auto callback = TweenCallback();
  callback.set_as_item_pickup_display_complete(guid);
  tweens.tween_xys.push_back(TweenXY(
      start_rect, target_rect, game.engine.current_time, 150, 3000, callback,
      []() {}, []() {}));*/
}

void ItemPickupList::update(Game &game) {
  // update can modify the item pickup panels vector through a tween on
  // complete callback. So iterate backwards.
  for (int i = (int)item_pickup_panels.size() - 1; i >= 0; i--) {
    auto &panel = item_pickup_panels[i];
    panel.update(game);
    if (game.engine.current_time > panel.remove_time) {
      item_pickup_panels.erase(item_pickup_panels.begin() + i);
    }
  }
}

void ItemPickupList::draw(Game &game) {
  for (auto &panel : item_pickup_panels) {
    panel.draw(game);
  }
}

void ItemPickupList::remove_panel_with_guid(Game &game,
                                            boost::uuids::uuid guid) {
  auto rm_idx = -1;
  for (size_t i = 0; i < item_pickup_panels.size(); i++) {
    if (item_pickup_panels[i].guid == guid) {
      rm_idx = i;
      break;
    }
  }
  GAME_ASSERT(rm_idx != -1);
  if (rm_idx != -1) {
    item_pickup_panels.erase(item_pickup_panels.begin() + rm_idx);
  }
}

ItemPickupPanel &ItemPickupList::get_panel_with_guid(Game &game,
                                                     boost::uuids::uuid guid) {
  for (auto &panel : item_pickup_panels) {
    if (panel.guid == guid) {
      return panel;
    }
  }

  cout << "ItemPickupList::get_panel_with_guid. No panel had the guid " << guid
       << ".\n";
  abort();
}

void ItemPickupList::add_item(Game &game, Item &_item) {
  for (auto &panel : item_pickup_panels) {
    if (panel.item.item_name == _item.item_name) {
      panel.item.quantity += _item.quantity;
      // extend panel completion time
      panel.set_complete_state_tween(game);
      return;
    }
  }
  // item not being displayed already, so add it to the display
  // find an empty slot or add to the back of the vec
  auto earliest_free_idx = 0;
  for (size_t i = 0; i < item_pickup_panels.size(); i++) {
    auto &panel = item_pickup_panels[i];
    if (panel.idx == earliest_free_idx) {
      earliest_free_idx += 1;
    }
  }

  if (earliest_free_idx > (int)item_pickup_panels.size() - 1) {
    earliest_free_idx = item_pickup_panels.size();
  }

  // pass in the idx so that the panel knows where to go vertically,
  // they are removed by guid because when erased the idx would not be correct
  // due to a shift. i.e idx passed in is just for positioning, not for
  // locating the panel.
  item_pickup_panels.push_back(ItemPickupPanel(game, _item, earliest_free_idx));
}