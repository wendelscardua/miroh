#include "polyomino.hpp"
#include "bank-helper.hpp"
#include "input-mode.hpp"
#include "metasprites.hpp"
#include "polyominos.hpp"
#include <bank.h>
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Polyomino::Polyomino(Board &board, bool active)
    : board(board), definition(NULL), active(active) {}

void Polyomino::spawn() {
  active = true;
  grounded_timer = 0;
  column = 5;
  row = 0;
  u8 random_index;
  do {
    random_index = rand8() & 0x7f;
  } while (random_index >= NUM_POLYOMINOS);
  set_prg_bank(GET_BANK(polyominos));
  definition = polyominos[random_index];
  s8 max_delta = 0;
  for(auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) max_delta = delta.delta_row;
  }
  row -= (max_delta + 1);
}

void Polyomino::update(InputMode input_mode, u8 pressed, u8 held) {
  if (!active) {
    if (input_mode == InputMode::Polyomino) {
      input_mode = InputMode::Player;
    }
    return;
  }

  set_prg_bank(GET_BANK(polyominos));

  if ((input_mode == InputMode::Polyomino) && (held & PAD_DOWN)) {
    drop_timer = DROP_FRAMES;
  } else {
    drop_timer++;
  }

  if (drop_timer >= DROP_FRAMES) {
    drop_timer = 0;
    bool bumped = false;
    for(u8 i = 0; i < definition->size; i++) {
      auto& delta = definition->deltas[i];
      if (board.occupied(row + delta.delta_row + 1,
                         column + delta.delta_column)) {
        bumped = true;
        break;
      }
    }
    if (bumped) {
      grounded_timer++;
      if (grounded_timer >= MAX_GROUNDED_TIMER) {
        freeze_blocks();
        input_mode = InputMode::Player;
        return;
      }
    } else {
      row++;
      grounded_timer = 0;
    }
  }

  if (input_mode == InputMode::Polyomino) {
    if (pressed & PAD_LEFT) {
      bool bumped = false;
      for(u8 i = 0; i < definition->size; i++) {
        auto& delta = definition->deltas[i];
        if (board.occupied(row + delta.delta_row,
                           column - 1 + delta.delta_column)) {
          bumped = true;
          break;
        }
      }
      if (!bumped) {
        column--;
      }
    } else if (pressed & PAD_RIGHT) {
      bool bumped = false;
      for(u8 i = 0; i < definition->size; i++) {
        auto& delta = definition->deltas[i];
        if (board.occupied(row + delta.delta_row,
                           column + 1 + delta.delta_column)) {
          bumped = true;
          break;
        }
      }
      if (!bumped) {
        column++;
      }
    } else if (pressed & PAD_A) {
      definition = definition->right_rotation;
      for(auto kick : definition->right_kick->deltas) {
        s8 new_row = row + kick.delta_row;
        s8 new_column = column + kick.delta_column;

        bool bumped = false;
        for(u8 i = 0; i < definition->size; i++) {
          auto& delta = definition->deltas[i];
          if (board.occupied(new_row + delta.delta_row,
                             new_column + delta.delta_column)) {
            bumped = true;
            break;
          }
        }

        if (!bumped) {
          row = new_row;
          column = new_column;
          break;
        }
      }
    } else if (pressed & PAD_B) {
      definition = definition->left_rotation;
      for(auto kick : definition->left_kick->deltas) {
        s8 new_row = row + kick.delta_row;
        s8 new_column = column + kick.delta_column;

        bool bumped = false;
        for(u8 i = 0; i < definition->size; i++) {
          auto& delta = definition->deltas[i];
          if (board.occupied(new_row + delta.delta_row,
                             new_column + delta.delta_column)) {
            bumped = true;
            break;
          }
        }

        if (!bumped) {
          row = new_row;
          column = new_column;
          break;
        }
      }
    } else if (pressed & PAD_UP) {

    }
  }
}

void Polyomino::render() {
  if (!active)
    return;

  for (u8 i = 0; i < definition->size; i++) {
    auto& delta = definition->deltas[i];
    oam_meta_spr(board.origin_x +
                 (u8)((column + delta.delta_column) << 4),
                 board.origin_y +
                 (u8)((row + delta.delta_row) << 4),
                 metasprite_block);
  }
}

void Polyomino::freeze_blocks() {
  active = false;

  for (u8 i = 0; i < definition->size; i++) {
    auto& delta = definition->deltas[i];
    s8 block_row = row + delta.delta_row;
    s8 block_column = column + delta.delta_column;
    if (!board.occupied(block_row, block_column)) {
      if (block_row >= 0) board.cell[block_row][block_column].occupied = true;
      int position = NTADR_A((board.origin_x >> 3) + (block_column << 1), (board.origin_y >> 3) + (block_row << 1));
      multi_vram_buffer_horz((const u8[2]){0x04, 0x05}, 2, position);
      multi_vram_buffer_horz((const u8[2]){0x14, 0x15}, 2, position+0x20);
      // TODO set attribute
    }
  }
}
