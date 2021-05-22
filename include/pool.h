#ifndef POOL_H
#define POOL_H
#include "constants.h"
#include <SDL.h>
#include <deque>
#include <stdio.h>

#include "utils.h"
using namespace std;

template <class T> class Pool {
public:
  // deque so that adding to the end with push_back doesn't invalidate
  deque<T> items;
  Pool();
  int get_handle(T item_to_add);
  void release_handle(int handle);
  T &get_item(int handle);
  int num_in_use();
  void clear();
};

template <class T> Pool<T>::Pool() { items = deque<T>(); }

template <class T> int Pool<T>::get_handle(T item_to_add) {
  item_to_add.in_use_in_pool = true;
  for (int i = 0; i < (int)items.size(); i++) {
    auto &item = items[i];
    if (!item.in_use_in_pool) {
      item_to_add.pool_handle = i;
      item = item_to_add;
      return i;
    }
  }
  // all items are in use, add a new item to the Pool
  // and return the handle. This makes the Pool expand
  // when it is full.
  item_to_add.pool_handle = items.size();
  items.push_back(item_to_add);
  return items.size() - 1;
}

template <class T> void Pool<T>::release_handle(int handle) {
  GAME_ASSERT(handle >= 0 && handle <= (int)items.size() - 1);
  GAME_ASSERT(items[handle].in_use_in_pool);
  items[handle].in_use_in_pool = false;
}

template <class T> T &Pool<T>::get_item(int handle) {
  GAME_ASSERT(handle >= 0 && handle <= (int)items.size() - 1);
  GAME_ASSERT(items[handle].in_use_in_pool);
  return items[handle];
}

template <class T> int Pool<T>::num_in_use() {
  int num = 0;
  for (auto &item : items) {
    if (item.in_use_in_pool) {
      num += 1;
    }
  }
  return num;
}

template <class T> void Pool<T>::clear() {
  for (auto &item : items) {
    item.in_use_in_pool = false;
  }
}

#endif // POOL_H