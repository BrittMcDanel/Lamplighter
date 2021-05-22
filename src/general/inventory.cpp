#include "inventory.h"
#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "utils.h"
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Inventory::Inventory(Game &game) {
  for (size_t i = 0; i < NUM_INVENTORY_SLOTS; i++) {
    items.push_back(Item());
  }
}

void Inventory::update(Game &game) {
  for (auto &item : items) {
    item.update(game);
  }
}

void Inventory::draw(Game &game) {
  for (auto &item : items) {
    item.draw(game);
  }
}

void Inventory::add_item(Item &_item) {
  if (_item.item_name == ItemName::None) {
    return;
  }
  for (auto &item : items) {
    // will push back the new item if it would be larger than item max quantity
    if (item.item_name == _item.item_name &&
        item.quantity + _item.quantity <= ITEM_MAX_QUANTITY) {
      item.quantity += _item.quantity;
      return;
    }
  }

  // item doesn't exist in inventory, look for an open slot
  for (auto &item : items) {
    if (item.item_name == ItemName::None) {
      item = _item;
      return;
    }
  }

  // no empty slots were found, inventory is full.
  // not pushing back right now, assuming a fixed size, not growable
  // inventory. If growable it would be items.push_back(item).
}

void Inventory::add_item_to_idx(Item &_item, int idx) {
  if (_item.item_name == ItemName::None) {
    return;
  }
  if (!(idx >= 0 || idx <= (int)items.size() - 1)) {
    return;
  }

  auto &item = items[idx];
  if (item.item_name == ItemName::None) {
    item = _item;
  } else if (item.item_name == _item.item_name) {
    item.quantity += _item.quantity;
    // quantity overflow, add the item with the quantity diff
    // using add_item
    if (item.quantity > ITEM_MAX_QUANTITY) {
      auto diff = item.quantity - ITEM_MAX_QUANTITY;
      auto item_cpy = _item;
      item_cpy.quantity = diff;
      add_item(item_cpy);
    }
  }
  // tried to add an item to a non empty slot, or a slot with a different item,
  // just add the item using add_item which will add it to an
  // empty slot
  else {
    add_item(_item);
  }
}

void Inventory::remove_item(Item &_item) {
  if (_item.item_name == ItemName::None) {
    return;
  }
  for (auto &item : items) {
    if (item.item_name == _item.item_name) {
      item.quantity -= _item.quantity;
      if (item.quantity <= 0) {
        item.item_name = ItemName::None;
        item.quantity = 0;
      }
      return;
    }
  }
}

void Inventory::dec_quantity_at_idx(int idx, int quantity) {
  if (!(idx >= 0 || idx <= (int)items.size() - 1)) {
    return;
  }
  items[idx].quantity -= quantity;
  if (items[idx].quantity <= 0) {
    items[idx] = Item();
  }
}

void Inventory::clear() {
  for (auto &item : items) {
    item = Item();
  }
}

void Inventory::swap(int idx1, int idx2) {
  if (!(idx1 >= 0 && idx1 <= (int)items.size() - 1)) {
    return;
  }
  if (!(idx2 >= 0 && idx2 <= (int)items.size() - 1)) {
    return;
  }
  if (idx1 == idx2) {
    return;
  }
  // swapped same item name, combine them
  if (items[idx1].item_name == items[idx2].item_name) {
    auto total_quantity = items[idx1].quantity + items[idx2].quantity;
    if (total_quantity > ITEM_MAX_QUANTITY) {
      auto diff = total_quantity - ITEM_MAX_QUANTITY;
      items[idx2].quantity = ITEM_MAX_QUANTITY;
      items[idx1].quantity = diff;
    } else {
      items[idx2].quantity = total_quantity;
      items[idx1] = Item();
    }
  } else {
    auto temp = items[idx1];
    items[idx1] = items[idx2];
    items[idx2] = temp;
  }
}

int Inventory::get_total_cost() {
  int total_cost = 0;
  for (auto &item : items) {
    if (item.item_name == ItemName::None || item.item_type == ItemType::Money) {
      continue;
    }
    total_cost += item.cost * item.quantity;
  }
  return total_cost;
}

int Inventory::get_total_money() {
  int total_money = 0;
  for (auto &item : items) {
    if (item.item_name == ItemName::None) {
      continue;
    }
    if (item.item_type == ItemType::Money) {
      total_money += item.quantity;
    }
  }
  return total_money;
}

int Inventory::size() {
  int s = 0;
  for (auto &item : items) {
    if (item.item_name != ItemName::None) {
      s += 1;
    }
  }
  return s;
}

void inventory_serialize(Game &game, Inventory &inventory) {
  game.serializer.writer.StartObject();
  game.serializer.writer.String("items");
  game.serializer.writer.StartArray();
  for (auto &item : inventory.items) {
    item_serialize(game, item);
  }
  game.serializer.writer.EndArray();
  game.serializer.writer.EndObject();
  // cout << "output " << game.serializer.sb.GetString() << "\n";
}

Inventory inventory_deserialize(Game &game, GenericObject<false, Value> &obj) {
  Inventory inventory = Inventory();
  auto items_array = obj["items"].GetArray();
  for (auto &item : items_array) {
    auto obj = item.GetObject();
    inventory.items.push_back(item_deserialize(game, obj));
  }
  return inventory;
}

void inventory_serialize_into_file(Game &game, Inventory &inventory,
                                   const char *file_path) {
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  inventory_serialize(game, inventory);
  std::ofstream file(file_path);
  if (!file.good()) {
    cout << "inventory_serialize_into_file. File error " << file_path << "\n";
    abort();
  }
  file.write(game.serializer.sb.GetString(), game.serializer.sb.GetSize());
  file.close();
}

Inventory inventory_deserialize_from_file(Game &game, const char *file_path) {
  ifstream file(file_path);
  if (!file.good()) {
    cout << "inventory_deserialize_from_file. File error " << file_path << "\n";
    abort();
  }
  stringstream buffer;
  buffer.clear();
  buffer << file.rdbuf();
  // cout << "buff " << buffer.str().c_str() << "\n";
  // clear as this is for individual units, not part of a nested struct (like
  // map)
  game.serializer.clear();
  // parse
  game.serializer.doc.Parse(buffer.str().c_str());
  // inventory object
  auto obj = game.serializer.doc.GetObject();
  // deserialize unit_obj into an Inventory
  return inventory_deserialize(game, obj);
}