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

#define POLYOMINOS_TEXT ".prg_rom_0.text"

Polyomino::Polyomino(Board &board, bool active)
    : board(board), definition(NULL), active(active) {}

void Polyomino::spawn() {
  active = true;
  grounded_timer = 0;
  column = 5;
  row = 0;
  set_prg_bank(GET_BANK(polyominos));

  u8 random_weight = rand8();
  u8 random_index = 0;
  while(random_weight >= polyomino_weights[random_index]) {
    random_weight -= polyomino_weights[random_index];
    random_index++;
  }

  definition = polyominos[random_index];
  s8 max_delta = 0;
  for(auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) max_delta = delta.delta_row;
  }
  row -= (max_delta + 1);
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::update(InputMode &input_mode, u8 pressed, u8 held, bool &blocks_placed, bool &failed_to_place, u8 &lines_filled) {
  if (!active) {
    if (input_mode == InputMode::Polyomino) {
      input_mode = InputMode::Player;
    }
    return;
  }

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
        grounded_timer = 0;
        if (can_be_frozen()) {
          lines_filled = freeze_blocks();
          blocks_placed = true;
          input_mode = InputMode::Player;
          return;
        } else {
          failed_to_place = true;
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
        banked_lambda(GET_BANK(sfx_list),
                      []() {
                        GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);

                      });
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
        banked_lambda(GET_BANK(sfx_list),
                      []() {
                        GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
                      });
      } else {
        definition = definition->right_rotation; // undo rotation
      }
    } else if (pressed & PAD_UP) {

    }
  }
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::banked_render() {
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
}

void Polyomino::render() {
  if (!active)
    return;

  banked_lambda(GET_BANK(polyominos), [this]() {banked_render();});
}

bool Polyomino::can_be_frozen() {
  s8 min_y_delta = 2;

  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    if (delta.delta_row < min_y_delta) min_y_delta = delta.delta_row;
  }
  return row + min_y_delta >= 0;
}

u8 Polyomino::freeze_blocks() {
  banked_lambda(GET_BANK(sfx_list), []() {
    GGSound::play_sfx(SFX::Click, GGSound::SFXPriority::Two);
  });

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

  return filled_lines;
}
