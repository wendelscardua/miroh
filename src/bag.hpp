#pragma once

#include "common.hpp"
#include <array>
#include <cstddef>

template <typename T, size_t N> class Bag {
  std::array<T, N> items;
  u8 index;
  u8 end;
  void (*refill)(Bag *bag);

public:
  Bag(void (*refill)(Bag *bag));

  void insert(T value);

  T take();

  T peek();

  bool empty();
};

template <typename T, size_t N>
Bag<T, N>::Bag(void (*refill)(Bag *bag)) : index(0), end(0), refill(refill) {};

template <typename T, size_t N> void Bag<T, N>::insert(T value) {
  items[end] = value;
  end++;
  if (end == N) {
    end = 0;
  }
}

template <typename T, size_t N> T Bag<T, N>::take() {
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

template <typename T, size_t N> T Bag<T, N>::peek() {
  if (empty()) {
    refill(this);
  }
  return items[index];
}

template <typename T, size_t N> bool Bag<T, N>::empty() { return index == end; }
