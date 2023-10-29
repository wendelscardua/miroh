#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"

#define NUM_POLYOMINOS 28

#define CHIBI_TILE 0x1a

struct Coordinates {
  s8 delta_row;
  s8 delta_column;

  constexpr s8 delta_x() { return delta_column * 0x10; }
  constexpr s8 delta_y() { return delta_row * 0x10; }
};

#define SOA_STRUCT Coordinates
#define SOA_MEMBERS                                                            \
  MEMBER(delta_row)                                                            \
  MEMBER(delta_column)

#include <soa-struct.inc>

struct Kick {
  soa::Array<const Coordinates, 5> deltas;
};

struct PolyominoDef {
  const PolyominoDef *const left_rotation;
  const PolyominoDef *const right_rotation;
  const Kick *const left_kick;
  const Kick *const right_kick;
  const u8 size;
  const soa::Array<const Coordinates, 5> deltas;
  const char preview_tiles[4];

  bool collide(Board &board, s8 row, s8 column) const;
  void render(int x, int y) const;
  void chibi_render(u8 row, u8 column) const;
  void board_render(Board &board, s8 row, s8 column, bool jiggling) const;
};

extern "C" const soa::Array<PolyominoDef *, NUM_POLYOMINOS> polyominos;
