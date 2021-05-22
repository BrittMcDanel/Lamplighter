#ifndef INVENTORY_H
#define INVENTORY_H
#include <SDL.h>

#include "constants.h"
#include "item.h"
#include "rapidjson/document.h"
#include "tweenable_sprite.h"
#include <vector>
using namespace std;
using namespace rapidjson;

struct Game;

struct Inventory {
  vector<Item> items = vector<Item>();
  Inventory() = default;
  Inventory(Game &game);
  void update(Game &game);
  void draw(Game &game);
  void add_item(Item &_item);
  void add_item_to_idx(Item &_item, int idx);
  void remove_item(Item &item);
  void dec_quantity_at_idx(int idx, int quantity);
  void clear();
  void swap(int idx1, int idx2);
  int get_total_cost();
  int get_total_money();
  int size();
};

void inventory_serialize(Game &game, Inventory &inventory);
void inventory_serialize_into_file(Game &game, Inventory &inventory,
                                   const char *file_path);
Inventory inventory_deserialize_from_file(Game &game, const char *file_path);
Inventory inventory_deserialize(Game &game, GenericObject<false, Value> &obj);

#endif // INVENTORY_H