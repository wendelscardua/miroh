#pragma once

#include "common.hpp"
#include "utils.hpp"
#include <array>
#include <cstddef>
#include <neslib.h>

template <typename T, size_t N> class Bag {
  std::array<T, N> items;
  u8 index;
  u8 end;
  u8 size;
  void (*refill)(Bag *bag);

public:
  Bag(void (*refill)(Bag *bag)) : index(0), end(0), size(0), refill(refill) {};

  void insert(T value) {
    u8 random_point = index + RAND_UP_TO(size + 1);
    if (random_point >= N) {
      random_point -= N;
    }

    if (random_point == end) {
      items[end++] = value;
    } else {
      items[end++] = items[random_point];
      items[random_point] = value;
    }
    if (end == N) {
      end = 0;
    }
    size++;
  }

  T take() {
    if (empty()) {
      refill(this);
    }
    u8 old_index = index;
    index++;
    if (index == N) {
      index = 0;
    }
    size--;
    return items[old_index];
  }

  T peek() {
    if (empty()) {
      refill(this);
    }
    return items[index];
  }

  bool empty() { return size == 0; }
};
