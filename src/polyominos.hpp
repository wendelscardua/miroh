#pragma once

#include <array>

#include "common.hpp"

#define NUM_POLYOMINOS 66

struct Coordinates {
  s8 delta_row;
  s8 delta_column;

  constexpr s8 delta_x() { return delta_column * 0x10; }
  constexpr s8 delta_y() { return delta_row * 0x10; }
};

struct Kick {
  std::array<Coordinates, 5> deltas;
};

struct PolyominoDef {
  PolyominoDef* left_rotation;
  PolyominoDef* right_rotation;
  Kick* left_kick;
  Kick* right_kick;
  u8 size;
  std::array<Coordinates, 5> deltas;
};

extern "C" const std::array<PolyominoDef*, NUM_POLYOMINOS> polyominos;
