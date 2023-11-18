#include "polyomino-defs.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "common.hpp"
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

void PolyominoDef::render(u8 x, int y) const {
  static u8 polyomino_start_index = 0;

  for (u8 j = 0; j < 5; j++) {
    u8 i = polyomino_start_index + j;
    if (i >= 5) {
      i -= 5;
    }
    if (i >= size) {
      continue;
    }

    auto delta = deltas[i];
    u8 block_x = (u8)(x + (delta.delta_column << 4));
    int block_y = y + (delta.delta_row << 4);
    banked_oam_meta_spr(block_x, block_y,
                        current_stage == Stage::StarlitStables
                            ? metasprite_block
                            : metasprite_BlockB);
  }

  polyomino_start_index += 2;
  if (polyomino_start_index >= 5) {
    polyomino_start_index -= 5;
  }
}

void PolyominoDef::outside_render(u8 x, int y, int cutting_point_y) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    u8 block_x = (u8)(x + (delta.delta_column << 4));
    int block_y = y + (delta.delta_row << 4);
    if (block_y >= cutting_point_y) {
      continue;
    }
    banked_oam_meta_spr(block_x, block_y,
                        current_stage == Stage::StarlitStables
                            ? metasprite_block
                            : metasprite_BlockB);
  }
}

void PolyominoDef::chibi_render(u8 row, u8 column) const {
  multi_vram_buffer_horz(preview_tiles, 2, NTADR_A(column, row));
  multi_vram_buffer_horz(preview_tiles + 2, 2, NTADR_A(column, row + 1));
}

bool PolyominoDef::board_render(Board &board, s8 row, s8 column,
                                bool jiggling) const {
  bool it_fits = true;
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    s8 block_row = row + delta.delta_row;
    s8 block_column = column + delta.delta_column;
    if (block_row >= 0) {
      board.set_maze_cell(block_row, block_column,
                          jiggling ? CellType::Jiggling
                                   : CellType::Marshmallow);
    } else {
      it_fits = false;
    }
  }
  return it_fits;
}
