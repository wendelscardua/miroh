#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"

#define NUM_POLYOMINOS 28

#define CHIBI_TILE 0x1a

struct Coordinates {
  s8 delta_row : 8;
  s8 delta_column : 8;

  constexpr u8 delta_x() { return (u8)(delta_column) * 0x10; }
  constexpr u8 delta_y() { return (u8)(delta_row) * 0x10; }
};

struct Kick {
  std::array<const Coordinates, 5> deltas;
};

struct PolyominoDef {
  const u8 index;
  const PolyominoDef *const left_rotation;
  const PolyominoDef *const right_rotation;
  const Kick *const left_kick;
  const Kick *const right_kick;
  const u8 size;
  std::array<const Coordinates, 5> deltas;
  const char preview_tiles[4];

  bool collide(Board &board, s8 row, u8 column) const;
  void render(u8 x, int y) const;
  void shadow(u8 x, int y, u8 dist) const;
  void chibi_render(u8 row, u8 column) const;

  // draws the polyomino on the nametable/board
  // returns true if the whole polyomino fits
  bool board_render(Board &board, s8 row, u8 column) const;
};

extern "C" const soa::Array<PolyominoDef *, NUM_POLYOMINOS> polyominos;
