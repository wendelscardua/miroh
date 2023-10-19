#include "polyomino-defs.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

bool PolyominoDef::collide(Board &board, s8 row, s8 column) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    if (board.occupied(row + delta.delta_row, column + delta.delta_column)) {
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
    banked_oam_meta_spr((u8)block_x, (u8)block_y, metasprite_block);
  }
}

void PolyominoDef::chibi_render(u8 row, u8 column) const {
  multi_vram_buffer_horz(preview_tiles, 2, NTADR_A(column, row));
  multi_vram_buffer_horz(preview_tiles + 2, 2, NTADR_A(column, row + 1));
}

void PolyominoDef::board_render(Board &board, s8 row, s8 column,
                                bool jiggling) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    s8 block_row = row + delta.delta_row;
    s8 block_column = column + delta.delta_column;
    board.block_maze_cell(block_row, block_column, jiggling);
  }
}
