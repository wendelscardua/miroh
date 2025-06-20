#include "polyomino-defs.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "common.hpp"
#include "log.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

bool PolyominoDef::collide(Board &board, s8 row, u8 column) const {
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

#pragma clang loop unroll(full)
  for (u8 j = 0; j < 5; j++) {
    u8 i = polyomino_start_index + j;
    if (i >= 5) {
      i -= 5;
    }
    if (i >= size) {
      continue;
    }

    auto delta = deltas[i];
    u8 block_x = (u8)(x + delta.delta_x());
    int block_y = y + delta.delta_y();
    banked_oam_meta_spr(block_x, block_y,
                        current_stage == Stage::StarlitStables
                            ? Metasprites::block
                            : Metasprites::BlockB);
  }

  polyomino_start_index += 2;
  if (polyomino_start_index >= 5) {
    polyomino_start_index -= 5;
  }
}

static const char *shadows[] = {
    (const char *)Metasprites::BlockShadow1,
    (const char *)Metasprites::BlockShadow2,
    (const char *)Metasprites::BlockShadow3,
    (const char *)Metasprites::BlockShadow4,
    (const char *)Metasprites::BlockShadow5,
};

// TODO: avoid overlap with render
void PolyominoDef::shadow(u8 x, int y, u8 dist) const {
  static u8 polyomino_start_index = 0;

  if (dist == 0) {
    return;
  }

  const char *metasprite =
      dist >= 5 ? (const char *)Metasprites::BlockShadow5 : shadows[dist - 1];

  for (u8 j = 0; j < 5; j++) {
    u8 i = polyomino_start_index + j;
    if (i >= 5) {
      i -= 5;
    }
    if (i >= size) {
      continue;
    }

    START_MESEN_WATCH(100);
    auto delta = deltas[i];
    u8 block_x = (u8)(x + delta.delta_x());
    int block_y = y + delta.delta_y();
    banked_oam_meta_spr(block_x, block_y, metasprite);
    STOP_MESEN_WATCH(100);
  }

  polyomino_start_index += 2;
  if (polyomino_start_index >= 5) {
    polyomino_start_index -= 5;
  }
}

void PolyominoDef::outside_render(u8 x, int y, int cutting_point_y) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    u8 block_x = (u8)(x + delta.delta_x());
    int block_y = y + delta.delta_y();
    if (block_y >= cutting_point_y) {
      continue;
    }
    banked_oam_meta_spr(block_x, block_y,
                        current_stage == Stage::StarlitStables
                            ? Metasprites::block
                            : Metasprites::BlockB);
  }
}

void PolyominoDef::chibi_render(u8 row, u8 column) const {
  multi_vram_buffer_horz(preview_tiles, 2, NTADR_A(column, row));
  multi_vram_buffer_horz(preview_tiles + 2, 2, NTADR_A(column, row + 1));
}

bool PolyominoDef::board_render(Board &board, s8 row, u8 column) const {
  bool it_fits = true;
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    s8 block_row = row + delta.delta_row;
    u8 block_column = column + delta.delta_column;
    if (block_row >= 0) {
      banked_lambda(Board::BANK, [&board, block_row, block_column]() {
        board.add_animation(BoardAnimation(&Board::block_jiggle,
                                           sizeof(Board::block_jiggle) /
                                               sizeof(Board::block_jiggle[0]),
                                           (u8)block_row, (u8)block_column));
        // XXX: just so line clears can be counted
        board.occupy(block_row, block_column);
      });
    } else {
      it_fits = false;
    }
  }
  return it_fits;
}
