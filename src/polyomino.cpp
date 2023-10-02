#include "polyomino.hpp"
#include "attributes.hpp"
#include "bank-helper.hpp"
#include "ggsound.hpp"
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
    random_index = rand8() & 0x1f;
  } while (random_index >= NUM_POLYOMINOS);
  set_prg_bank(GET_BANK(polyominos));
  definition = polyominos[random_index];
  s8 max_delta = 0;
  for(auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) max_delta = delta.delta_row;
  }
  row -= (max_delta + 1);
}

void Polyomino::update(InputMode &input_mode, u8 pressed, u8 held, bool &blocks_placed, u8 &lines_filled) {
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
      auto delta = definition->deltas[i];
      if (board.occupied(row + delta.delta_row + 1,
                         column + delta.delta_column)) {
        bumped = true;
        break;
      }
    }
    if (bumped) {
      if (grounded_timer >= MAX_GROUNDED_TIMER) {
        if (can_be_frozen()) {
          lines_filled = freeze_blocks();
          blocks_placed = true;
          input_mode = InputMode::Player;
          return;
        }
      } else {
        grounded_timer++;
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
        auto delta = definition->deltas[i];
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
        auto delta = definition->deltas[i];
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
      bool kicked = false;
      for(auto kick : definition->right_kick->deltas) {
        s8 new_row = row + kick.delta_row;
        s8 new_column = column + kick.delta_column;

        bool bumped = false;
        for(u8 i = 0; i < definition->size; i++) {
          auto delta = definition->deltas[i];
          if (board.occupied(new_row + delta.delta_row,
                             new_column + delta.delta_column)) {
            bumped = true;
            break;
          }
        }

        if (!bumped) {
          row = new_row;
          column = new_column;
          kicked = true;
          break;
        }
      }
      if (kicked) {
        u8 old_bank = get_prg_bank();
        set_prg_bank(GET_BANK(sfx_list));
        GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        set_prg_bank(old_bank);
      } else {
        definition = definition->left_rotation; // undo rotation
      }
    } else if (pressed & PAD_B) {
      definition = definition->left_rotation;
      bool kicked = false;
      for(auto kick : definition->left_kick->deltas) {
        s8 new_row = row + kick.delta_row;
        s8 new_column = column + kick.delta_column;

        bool bumped = false;
        for(u8 i = 0; i < definition->size; i++) {
          auto delta = definition->deltas[i];
          if (board.occupied(new_row + delta.delta_row,
                             new_column + delta.delta_column)) {
            bumped = true;
            break;
          }
        }

        if (!bumped) {
          row = new_row;
          column = new_column;
          kicked = true;
          break;
        }
      }
      if (kicked) {
        u8 old_bank = get_prg_bank();
        set_prg_bank(GET_BANK(sfx_list));
        GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
        set_prg_bank(old_bank);
      } else {
        definition = definition->right_rotation; // undo rotation
      }
    } else if (pressed & PAD_UP) {

    }
  }
}

void Polyomino::render() {
  if (!active)
    return;

  u8 old_bank = get_prg_bank();
  set_prg_bank(GET_BANK(polyominos));

  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    auto block_x =
        board.origin_x + ((column + delta.delta_column) << 4);
    auto block_y = board.origin_y + ((row + delta.delta_row) << 4);
    if (block_y >= 0) {
      if (row < 0) {
        block_y++;
      }
      oam_meta_spr((u8)block_x, (u8)block_y, metasprite_block);
    }
  }
  set_prg_bank(old_bank);
}

bool Polyomino::can_be_frozen() {
  s8 min_y_delta = 2;

  u8 old_bank = get_prg_bank();
  set_prg_bank(GET_BANK(polyominos));
  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    if (delta.delta_row < min_y_delta) min_y_delta = delta.delta_row;
  }
  set_prg_bank(old_bank);
  return row + min_y_delta >= 0;
}

u8 Polyomino::freeze_blocks() {
  u8 old_bank = get_prg_bank();

  set_prg_bank(GET_BANK(sfx_list));
  GGSound::play_sfx(SFX::Click, GGSound::SFXPriority::Two);

  set_prg_bank(GET_BANK(polyominos));

  active = false;
  u8 filled_lines = 0;
  Attributes::enable_vram_buffer();
  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    s8 block_row = row + delta.delta_row;
    s8 block_column = column + delta.delta_column;
    if (!board.occupied(block_row, block_column)) {
      if (block_row >= 0) {
        board.occupy(block_row, block_column);
        if (board.tally[block_row] == WIDTH) {
          filled_lines++;
        }
      }
      int position = NTADR_A((board.origin_x >> 3) + (block_column << 1), (board.origin_y >> 3) + (block_row << 1));
      multi_vram_buffer_horz((const u8[2]){0x74, 0x75}, 2, position);
      multi_vram_buffer_horz((const u8[2]){0x84, 0x85}, 2, position+0x20);
      Attributes::set((board.origin_x >> 4) +(u8)block_column, (board.origin_y >> 4) +(u8)block_row, FROZEN_BLOCK_ATTRIBUTE);
    }
  }
  Attributes::flush_vram_update();

  set_prg_bank(old_bank);

  return filled_lines;
}
