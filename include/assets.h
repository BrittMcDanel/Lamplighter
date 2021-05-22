#ifndef ASSETS_H
#define ASSETS_H

#include "ability.h"
#include "constants.h"
#include "item.h"
#include "status_effect.h"
#include "treasure_chest.h"
#include "unit.h"
#include <SDL.h>
#include <array>
#include <functional>
#include <unordered_map>
#include <vector>
using namespace std;

struct Game;

class Assets {
private:
  array<Item, ITEM_NAME_LAST> items;
  array<Ability, ABILITY_NAME_LAST> abilities;
  array<Unit, UNIT_NAME_LAST> units;
  array<TreasureChest, TREASURE_CHEST_NAME_LAST> treasure_chests;
  array<StatusEffect, STATUS_EFFECT_NAME_LAST> status_effects;
  // used for reserializing all assets if a new property
  // was added to an asset. For example, is_melee added
  // to ability, don't want to manually serialize all abilities again.
  // can just reserialize everything all at once with the new property.
  // key is the file name.
  // don't want to put the fila names in the assets themselves.
  unordered_map<string, Item> item_dict;
  unordered_map<string, Ability> ability_dict;
  unordered_map<string, Unit> unit_dict;
  unordered_map<string, TreasureChest> treasure_chest_dict;
  unordered_map<string, StatusEffect> status_effect_dict;

public:
  Assets() = default;
  void start(Game &game);
  void reserialize_all_assets(Game &game);
  void update_all_assets_from_files(Game &game);
  const Item &get_item(ItemName item_name);
  const Ability &get_ability(AbilityName ability_name);
  const Unit &get_unit(UnitName unit_name);
  const StatusEffect &get_status_effect(StatusEffectName status_effect_name);
};

template <typename T, size_t N, typename AssetName>
void check_if_all_asset_names_exist(
    array<T, N> asset_array, function<AssetName(const T &)> get_asset_name,
    const char *asset_display_name) {
  auto asset_debug_map = unordered_map<int, bool>();
  for (size_t i = 0; i < asset_array.size(); i++) {
    auto &asset = asset_array.at(i);
    // get the treasure_chest at assets index i, sometimes the enum
    // can get messed up so we check that all are correct.
    auto key = static_cast<int>(get_asset_name(asset));
    asset_debug_map[key] = true;
  }
  auto asset_name_error = false;
  for (size_t i = 0; i < asset_array.size(); i++) {
    // this time use the index, not the treasure_chest to see if all the
    // treasure_chestnames are in the treasure_chests array
    if (asset_debug_map.find(i) == asset_debug_map.end()) {
      asset_name_error = true;
      cout << "Assets - " << asset_display_name << " name not found " << i
           << "\n";
    }
  }
  // don't abort, just fix it in the editor
  if (asset_name_error) {
    cout << "Assets::update_all_assets_from_files - " << asset_display_name
         << " name not "
            "found.\n";
  }
}

#endif // ASSETS_H