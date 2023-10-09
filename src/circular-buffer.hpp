#pragma once

#include "common.hpp"
#include <array>
#include <cstddef>
#include <neslib.h>

template <typename T, size_t N> class CircularBuffer {
  std::array<T, N> items;
  u8 index;
  u8 end;

public:
  CircularBuffer() : index(0), end(0) {};

  void insert(T value) {
    items[end++] = value;
    if (end == N) {
      end = 0;
    }
  }

  T take() {
    u8 old_index = index;
    index++;
    if (index == N) {
      index = 0;
    }
    return items[old_index];
  }

  T peek() {
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
