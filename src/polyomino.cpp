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
  grounded_timer = 0;
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
  y -= GRID_SIZE * (u8)(max_delta + 1);
  target_x = x;
  target_y = y;
}

void Polyomino::update(InputMode input_mode, u8 pressed, u8 held) {
  if (!active)
    return;

  if (target_y == y) {
    target_y = y + GRID_SIZE;
    bool bumped = false;
    for(auto delta : definition->deltas) {
      if (board.occupied(x.whole + delta.delta_x(),
                         target_y.whole + delta.delta_y())) {
        bumped = true;
        break;
      }
    }
    if (bumped) {
      target_y = y;
      grounded_timer++;
    }
  }

  if (target_y > y) {
    y += DROP_SPEED;
    if (y > target_y) y = target_y;
  }
}

void Polyomino::render() {
  if (!active)
    return;

  for (u8 i = 0; i < definition->size; i++) {
    oam_meta_spr(board.origin_x +
                 (u8)(x.whole + definition->deltas[i].delta_x()),
                 board.origin_y +
                 (u8)(y.whole + definition->deltas[i].delta_y()),
                 metasprite_block);
  }
}
