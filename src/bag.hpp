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

public:
  Bag() : index(0), end(0) {};

  void reset() {
    index = 0;
    end = 0;
  }

  void insert(T value) {
    u8 random_point = RAND_UP_TO(end + 1);
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
    T value = items[index];
    index++;
    if (index == N) {
      index = 0;
    }
    return value;
  }

  T peek() { return items[index]; }
};
