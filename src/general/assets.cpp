#include "assets.h"
// linux only - couldn't get <filesystem> to compile
// need to change this as soon as possible as all users
// use this, it is not just for the editor.
#include <dirent.h>
#include <unordered_map>
using namespace std;

void Assets::start(Game &game) { update_all_assets_from_files(game); }

const Item &Assets::get_item(ItemName item_name) {
  auto idx = static_cast<int>(item_name);
  GAME_ASSERT(idx >= 0 && idx <= ITEM_NAME_LAST - 1);
  GAME_ASSERT(item_name == items[idx].item_name);
  return items[idx];
}

const Ability &Assets::get_ability(AbilityName ability_name) {
  auto idx = static_cast<int>(ability_name);
  GAME_ASSERT(idx >= 0 && idx <= ABILITY_NAME_LAST - 1);
  GAME_ASSERT(ability_name == abilities[idx].ability_name);
  return abilities[idx];
}

const Unit &Assets::get_unit(UnitName unit_name) {
  auto idx = static_cast<int>(unit_name);
  GAME_ASSERT(idx >= 0 && idx <= UNIT_NAME_LAST - 1);
  GAME_ASSERT(unit_name == units[idx].unit_name);
  return units[idx];
}

const StatusEffect &
Assets::get_status_effect(StatusEffectName status_effect_name) {
  auto idx = static_cast<int>(status_effect_name);
  GAME_ASSERT(idx >= 0 && idx <= STATUS_EFFECT_NAME_LAST - 1);
  GAME_ASSERT(status_effect_name == status_effects[idx].status_effect_name);
  return status_effects[idx];
}

void Assets::reserialize_all_assets(Game &game) {
  for (auto &entry : item_dict) {
    item_serialize_into_file(game, entry.second, entry.first.c_str());
  }
  for (auto &entry : ability_dict) {
    ability_serialize_into_file(game, entry.second, entry.first.c_str());
  }
  for (auto &entry : unit_dict) {
    unit_serialize_into_file(game, entry.second, entry.first.c_str());
  }
  for (auto &entry : treasure_chest_dict) {
    treasure_chest_serialize_into_file(game, entry.second, entry.first.c_str());
  }
  for (auto &entry : status_effect_dict) {
    status_effect_serialize_into_file(game, entry.second, entry.first.c_str());
  }
}

// if an asset with static data gets editted in the editor,
// can update all entities that use the static data (might
// need a map reload first though to deserialize them)
void Assets::update_all_assets_from_files(Game &game) {
  // clear previous asset data
  items.fill(Item());
  abilities.fill(Ability());
  units.fill(Unit());
  treasure_chests.fill(TreasureChest());
  status_effects.fill(StatusEffect());
  item_dict.clear();
  ability_dict.clear();
  unit_dict.clear();
  treasure_chest_dict.clear();
  status_effect_dict.clear();
  if (auto dir = opendir("../assets/prefabs/items")) {
    while (auto f = readdir(dir)) {
      if (!f->d_name || f->d_name[0] == '.') {
        continue; // Skip everything that starts with a dot
      }
      string prefab_file_path = "../assets/prefabs/items/";
      prefab_file_path += f->d_name;
      Item item =
          item_deserialize_from_file(game, prefab_file_path.c_str(), false);
      auto idx = static_cast<int>(item.item_name);
      items[idx] = item;
      item_dict[prefab_file_path] = item;
    }
    closedir(dir);
  }
  // status effects before abilities as abilities will get static data
  // from status effects
  if (auto dir = opendir("../assets/prefabs/statuseffects")) {
    while (auto f = readdir(dir)) {
      if (!f->d_name || f->d_name[0] == '.') {
        continue; // Skip everything that starts with a dot
      }
      string prefab_file_path = "../assets/prefabs/statuseffects/";
      prefab_file_path += f->d_name;
      StatusEffect status_effect = status_effect_deserialize_from_file(
          game, prefab_file_path.c_str(), false);
      auto idx = static_cast<int>(status_effect.status_effect_name);
      status_effects[idx] = status_effect;
      status_effect_dict[prefab_file_path] = status_effect;
    }
    closedir(dir);
  }
  if (auto dir = opendir("../assets/prefabs/abilities")) {
    while (auto f = readdir(dir)) {
      if (!f->d_name || f->d_name[0] == '.') {
        continue; // Skip everything that starts with a dot
      }
      string prefab_file_path = "../assets/prefabs/abilities/";
      prefab_file_path += f->d_name;
      Ability ability =
          ability_deserialize_from_file(game, prefab_file_path.c_str(), false);
      auto idx = static_cast<int>(ability.ability_name);
      abilities[idx] = ability;
      ability_dict[prefab_file_path] = ability;
    }
    closedir(dir);
  }
  if (auto dir = opendir("../assets/prefabs/units")) {
    while (auto f = readdir(dir)) {
      if (!f->d_name || f->d_name[0] == '.') {
        continue; // Skip everything that starts with a dot
      }
      string prefab_file_path = "../assets/prefabs/units/";
      prefab_file_path += f->d_name;
      Unit unit =
          unit_deserialize_from_file(game, prefab_file_path.c_str(), false);
      auto idx = static_cast<int>(unit.unit_name);
      units[idx] = unit;
      unit_dict[prefab_file_path] = unit;
    }
    closedir(dir);
  }
  if (auto dir = opendir("../assets/prefabs/treasurechests")) {
    while (auto f = readdir(dir)) {
      if (!f->d_name || f->d_name[0] == '.') {
        continue; // Skip everything that starts with a dot
      }
      string prefab_file_path = "../assets/prefabs/treasurechests/";
      prefab_file_path += f->d_name;
      TreasureChest treasure_chest = treasure_chest_deserialize_from_file(
          game, prefab_file_path.c_str(), false);
      auto idx = static_cast<int>(treasure_chest.treasure_chest_name);
      treasure_chests[idx] = treasure_chest;
      treasure_chest_dict[prefab_file_path] = treasure_chest;
    }
    closedir(dir);
  }

  check_if_all_asset_names_exist<Item, (size_t)ITEM_NAME_LAST, ItemName>(
      items, [&](const Item &item) -> ItemName { return item.item_name; },
      "items");
  check_if_all_asset_names_exist<Ability, (size_t)ABILITY_NAME_LAST,
                                 AbilityName>(
      abilities,
      [&](const Ability &ability) -> AbilityName {
        return ability.ability_name;
      },
      "abilities");
  check_if_all_asset_names_exist<Unit, (size_t)UNIT_NAME_LAST, UnitName>(
      units, [&](const Unit &unit) -> UnitName { return unit.unit_name; },
      "units");
  check_if_all_asset_names_exist<
      TreasureChest, (size_t)TREASURE_CHEST_NAME_LAST, TreasureChestName>(
      treasure_chests,
      [&](const TreasureChest &treasure_chest) -> TreasureChestName {
        return treasure_chest.treasure_chest_name;
      },
      "treasure chests");
  check_if_all_asset_names_exist<StatusEffect, (size_t)STATUS_EFFECT_NAME_LAST,
                                 StatusEffectName>(
      status_effects,
      [&](const StatusEffect &status_effect) -> StatusEffectName {
        return status_effect.status_effect_name;
      },
      "status effects");
}
