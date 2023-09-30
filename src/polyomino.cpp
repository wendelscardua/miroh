#include "polyomino.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"
#include "log.hpp"
#include "metasprites.hpp"
#include "polyominos.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Polyomino::Polyomino(Board &board, bool active)
    : board(board), definition(NULL), active(active) {}

void Polyomino::spawn() {
  active = true;
  x = fixed_point(0x50, 0);
  y = fixed_point(0x00, 0);
  u8 random_index;
  do {
    random_index = rand8() & 0x7f;
  } while (random_index >= NUM_POLYOMINOS);
  definition = polyominos[random_index];
  s8 max_delta = 0;
  for(auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) max_delta = delta.delta_row;
  }
  y -= fixed_point(0x10 * max_delta + 0x10, 0);
}

void Polyomino::update(InputMode input_mode, u8 pressed, u8 held) {
  if (!active)
    return;
}

void Polyomino::render() {
  if (!active)
    return;

  for (u8 i = 0; i < definition->size; i++) {
    oam_meta_spr(board.origin_x +
                     (u8)(x.whole + definition->deltas[i].delta_column * 0x10),
                 board.origin_y +
                     (u8)(y.whole + definition->deltas[i].delta_row * 0x10),
                 metasprite_block);
  }
}
