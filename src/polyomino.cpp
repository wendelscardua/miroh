#include "polyomino.hpp"
#include "attributes.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "direction.hpp"
#include "ggsound.hpp"
#include "input-mode.hpp"
#include "polyomino-defs.hpp"
#include <bank.h>
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

#define POLYOMINOS_TEXT ".prg_rom_0.text"

static auto pentominos = Bag<u8, 32>([](auto *bag) {
  for (u8 i = 0; i < NUM_POLYOMINOS; i++) {
    if (polyominos[i]->size == 5) {
      bag->insert(i);
    }
  }
});

auto Polyomino::pieces = Bag<u8, 32>([](auto *bag) {
  for (u8 i = 0; i < NUM_POLYOMINOS; i++) {
    if (polyominos[i]->size != 5) {
      bag->insert(i);
    }
  }
  bag->insert(pentominos.take());
  bag->insert(pentominos.take());
});

Polyomino::Polyomino(Board &board)
    : board(board), definition(NULL), next(polyominos[pieces.take()]),
      second_next(polyominos[pieces.take()]), active(false) {}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::spawn() {
  active = true;
  grounded_timer = 0;
  move_timer = 0;
  sideways_direction = Direction::None;
  column = 5;
  row = 0;

  definition = next;
  next = second_next;
  second_next = polyominos[pieces.take()];

  s8 max_delta = 0;
  for (auto delta : definition->deltas) {
    if (delta.delta_row > max_delta)
      max_delta = delta.delta_row;
  }
  row -= (max_delta + 1);
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) bool
Polyomino::able_to_kick(auto kick_deltas) {
  for (auto kick : kick_deltas) {
    s8 new_row = row + kick.delta_row;
    s8 new_column = column + kick.delta_column;

    if (!definition->collide(board, new_row, new_column)) {
      row = new_row;
      column = new_column;
      return true;
    }
  }
  return false;
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void
Polyomino::handle_input(InputMode &input_mode, u8 pressed, u8 held) {
  if (!active) {
    if (input_mode == InputMode::Polyomino) {
      input_mode = InputMode::Player;
    }
  }
  if (input_mode != InputMode::Polyomino) {
    return;
  }

  if (held & PAD_UP) {
    drop_timer = 200; // XXX: any absurd-but-not-max number
  }

  if (pressed & PAD_LEFT) {
    move_timer = SIDEWAYS_INITIAL_DELAY;
    sideways_direction = Direction::Left;
  } else if (held & PAD_LEFT) {
    if (--move_timer <= 0) {
      move_timer = SIDEWAYS_DELAY;
      sideways_direction = Direction::Left;
    }
  } else if (pressed & PAD_RIGHT) {
    move_timer = SIDEWAYS_INITIAL_DELAY;
    sideways_direction = Direction::Right;
  } else if (held & PAD_RIGHT) {
    if (--move_timer <= 0) {
      move_timer = SIDEWAYS_DELAY;
      sideways_direction = Direction::Right;
    }
  } else if (pressed & PAD_DOWN) {
    move_timer = SIDEWAYS_INITIAL_DELAY;
    sideways_direction = Direction::Down;
  } else if (held & PAD_DOWN) {
    if (--move_timer <= 0) {
      move_timer = SIDEWAYS_DELAY;
      sideways_direction = Direction::Down;
    }
  }

  if (pressed & PAD_A) {
    definition = definition->right_rotation;

    if (able_to_kick(definition->right_kick->deltas)) {
      banked_lambda(GET_BANK(sfx_list), []() {
        GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
      });
    } else {
      definition = definition->left_rotation; // undo rotation
    }
  } else if (pressed & PAD_B) {
    definition = definition->left_rotation;

    if (able_to_kick(definition->left_kick->deltas)) {
      banked_lambda(GET_BANK(sfx_list), []() {
        GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
      });
    } else {
      definition = definition->right_rotation; // undo rotation
    }
  }
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void
Polyomino::update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
                  u8 &lines_filled) {
  if (!active)
    return;
  if (drop_timer++ >= drop_frames) {
    drop_timer = 0;
    if (definition->collide(board, row + 1, column)) {
      if (grounded_timer >= MAX_GROUNDED_TIMER) {
        grounded_timer = 0;
        if (can_be_frozen()) {
          lines_filled = freeze_blocks();
          blocks_placed = true;
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

  switch (sideways_direction) {
  case Direction::Left:
    if (!definition->collide(board, row, column - 1)) {
      column--;
    }
    sideways_direction = Direction::None;
    break;
  case Direction::Right:
    if (!definition->collide(board, row, column + 1)) {
      column++;
    }
    sideways_direction = Direction::None;
    break;
  case Direction::Down:
    if (definition->collide(board, row + 1, column)) {
      if (can_be_frozen()) {
        lines_filled = freeze_blocks();
        blocks_placed = true;
      } else {
        failed_to_place = true;
      }
    } else {
      row++;
      sideways_direction = Direction::None;
    }
  default:
    break;
  }
}

void Polyomino::render() {
  if (!active)
    return;

  banked_lambda(GET_BANK(polyominos), [this]() {
    definition->render(board.origin_x + (u8)(column << 4),
                       board.origin_y + (u8)(row << 4));
  });
}

void Polyomino::render_next() {
  banked_lambda(GET_BANK(polyominos), [this]() {
    u8 next_x = board.origin_x + 0x10 * (WIDTH / 2);
    u8 next_y = board.origin_y - 0x20;
    if (active && row < 0) {
      next->chibi_render(next_x + 0x40, next_y);
    } else {
      next->chibi_render(next_x, next_y);
      second_next->chibi_render(next_x + 0x40, next_y);
    }
  });
}

__attribute__((noinline, section(POLYOMINOS_TEXT)))
bool Polyomino::can_be_frozen() {
  s8 min_y_delta = 2;

  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    if (delta.delta_row < min_y_delta)
      min_y_delta = delta.delta_row;
  }
  return row + min_y_delta >= 0;
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) u8
Polyomino::freeze_blocks() {
  banked_lambda(GET_BANK(sfx_list), []() {
    GGSound::play_sfx(SFX::Click, GGSound::SFXPriority::Two);
  });

  active = false;
  u8 filled_lines = 0;
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
      int position = NTADR_A((board.origin_x >> 3) + (block_column << 1),
                             (board.origin_y >> 3) + (block_row << 1));
      multi_vram_buffer_horz((const u8[2]){0x74, 0x75}, 2, position);
      multi_vram_buffer_horz((const u8[2]){0x84, 0x85}, 2, position + 0x20);
      Attributes::set((board.origin_x >> 4) + (u8)block_column,
                      (board.origin_y >> 4) + (u8)block_row,
                      FROZEN_BLOCK_ATTRIBUTE);
    }
  }

  return filled_lines;
}
