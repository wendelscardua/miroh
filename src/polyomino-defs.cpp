#include "polyomino-defs.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "common.hpp"
#include "polyominos-metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

bool PolyominoDef::collide(Board &board, s8 row, s8 column) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    if (board.occupied(row + delta.delta_row,
                       (u8)(column + delta.delta_column))) {
      return true;
    }
  }
  return false;
}

void PolyominoDef::render(u8 x, int y) const {
  u8 index = this->index;
  if (get_frame_count() & 1) {
    index += (u8)PolyominoMetaspriteMain::Piece_Id::FlipStart;
  }

  u8 bank = current_stage == Stage::StarlitStables
                ? POLYOMINO_METASPRITE_MAIN_BANK
                : POLYOMINO_METASPRITE_ALT_BANK;
  auto ptr = banked_lambda(POLYOMINO_METASPRITE_MAIN_BANK, [index, bank] {
    return bank == POLYOMINO_METASPRITE_MAIN_BANK
               ? PolyominoMetaspriteMain::all_pieces[index]
               : PolyominoMetaspriteAlt::all_pieces[index];
  });
  banked_oam_meta_spr(bank, x, y, ptr);
  return;
}

static const u8 shadow_banks[] = {
    POLYOMINO_METASPRITE_SHADOW1_BANK, POLYOMINO_METASPRITE_SHADOW2_BANK,
    POLYOMINO_METASPRITE_SHADOW3_BANK, POLYOMINO_METASPRITE_SHADOW4_BANK,
    POLYOMINO_METASPRITE_SHADOW5_BANK};

// TODO: avoid overlap with render
void PolyominoDef::shadow(u8 x, int y, u8 dist) const {
  u8 index = this->index;
  if (get_frame_count() & 1) {
    index += (u8)PolyominoMetaspriteShadow1::Piece_Id::FlipStart;
  }

  u8 bank;
  switch (dist) {
  case 0:
    return;
  case 1:
  case 2:
  case 3:
  case 4:
    bank = shadow_banks[dist - 1];
    break;
  default:
    bank = shadow_banks[4];
    break;
  }
  auto ptr = banked_lambda(bank, [index, dist] {
    switch (dist) {
    case 1:
      return PolyominoMetaspriteShadow1::all_pieces[index];
    case 2:
      return PolyominoMetaspriteShadow2::all_pieces[index];
    case 3:
      return PolyominoMetaspriteShadow3::all_pieces[index];
    case 4:
      return PolyominoMetaspriteShadow4::all_pieces[index];
    default:
      return PolyominoMetaspriteShadow5::all_pieces[index];
    }
  });
  banked_oam_meta_spr(bank, x, y, ptr);
  return;
}

void PolyominoDef::chibi_render(u8 row, u8 column) const {
  multi_vram_buffer_horz(preview_tiles, 2, NTADR_A(column, row));
  multi_vram_buffer_horz(preview_tiles + 2, 2, NTADR_A(column, row + 1));
}

bool PolyominoDef::board_render(Board &board, s8 row, s8 column) const {
  bool it_fits = true;
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    s8 block_row = row + delta.delta_row;
    u8 block_column = (u8)(column + delta.delta_column);
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
