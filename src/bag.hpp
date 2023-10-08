#pragma once

#include "common.hpp"
#include <array>
#include <cstddef>
#include <neslib.h>

template <typename T, size_t N> class Bag {
  std::array<T, N> items;
  u8 index;
  u8 end;
  void (*refill)(Bag *bag);

public:
  Bag(void (*refill)(Bag *bag)) : index(0), end(0), refill(refill){};

  void insert(T value) {
    u8 random_point = index + (((u16)rand8() * (u16)(size() + 1)) >> 8);
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
    return items[old_index];
  }

  T peek() {
    if (empty()) {
      refill(this);
    }
    return items[index];
  }

  bool empty() { return index == end; }

  size_t size() {
    if (end >= index) {
      return end - index;
    } else {
      return end + N - index;
    }
  }
};
