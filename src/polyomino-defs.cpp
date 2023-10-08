#include "polyomino-defs.hpp"
#include "metasprites.hpp"
#include <neslib.h>

bool PolyominoDef::collide(Board& board, s8 row, s8 column) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    if (board.occupied(row + delta.delta_row,
                       column + delta.delta_column)) {
      return true;
    }
  }
  return false;
}

void PolyominoDef::render(u8 x, u8 y) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    u8 block_x = x + (u8)(delta.delta_column << 4);
    u8 block_y = y + (u8)(delta.delta_row << 4);
    if (block_y == 0) {
      block_y++;
    }
    oam_meta_spr((u8)block_x, (u8)block_y, metasprite_block);
  }
}
