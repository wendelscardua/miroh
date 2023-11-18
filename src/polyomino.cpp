#include "polyomino.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "direction.hpp"
#include "ggsound.hpp"
#include "polyomino-defs.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

#define POLYOMINOS_TEXT ".prg_rom_0.text"

static auto littleminos = Bag<u8, 5>([](auto *bag) {
  for (u8 i = 0; i < NUM_POLYOMINOS; i++) {
    if (polyominos[i]->size <= 3) {
      bag->insert(i);
    }
  }
});

static auto pentominos = Bag<u8, 18>([](auto *bag) {
  for (u8 i = 0; i < NUM_POLYOMINOS; i++) {
    if (polyominos[i]->size == 5) {
      bag->insert(i);
    }
  }
});

auto Polyomino::pieces = Bag<u8, NUM_POLYOMINOS>([](auto *bag) {
  // add all tetrominos to the bag
  for (u8 i = 0; i < NUM_POLYOMINOS; i++) {
    if (polyominos[i]->size == 4) {
      bag->insert(i);
    }
  }

  // also add two random pentominos
  bag->insert(pentominos.take());
  bag->insert(pentominos.take());

  // ... and a random 1-2-or-3-mino
  bag->insert(littleminos.take());
});

Polyomino::Polyomino(Board &board)
    : board(board), definition(NULL), next(polyominos[pieces.take()]),
      state(State::Inactive) {
  render_next();
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::spawn() {
  state = State::Active;
  grounded_timer = 0;
  move_timer = 0;
  movement_direction = Direction::None;
  column = 5;
  row = 0;

  definition = next;
  next = polyominos[pieces.take()];

  render_next();

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
Polyomino::handle_input(u8 pressed, u8 held) {
  if (state != State::Active) {
    return;
  }
  if (pressed & PAD_UP) {
    // just some high enough value for the drop to proceed until the end
    drop_timer = HEIGHT * 70;
    grounded_timer = MAX_GROUNDED_TIMER;
    movement_direction = Direction::Up;
  } else if (pressed & PAD_LEFT) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    movement_direction = Direction::Left;
  } else if (held & PAD_LEFT) {
    if (--move_timer <= 0) {
      move_timer = MOVEMENT_DELAY;
      movement_direction = Direction::Left;
    }
  } else if (pressed & PAD_RIGHT) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    movement_direction = Direction::Right;
  } else if (held & PAD_RIGHT) {
    if (--move_timer <= 0) {
      move_timer = MOVEMENT_DELAY;
      movement_direction = Direction::Right;
    }
  } else if (pressed & PAD_DOWN) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    movement_direction = Direction::Down;
  } else if (held & PAD_DOWN) {
    if (--move_timer <= 0) {
      move_timer = MOVEMENT_DELAY;
      movement_direction = Direction::Down;
    }
  }

  if (pressed & PAD_A) {
    definition = definition->right_rotation;

    if (able_to_kick(definition->right_kick->deltas)) {
      banked_play_sfx(SFX::Rotate, GGSound::SFXPriority::One);
    } else {
      definition = definition->left_rotation; // undo rotation
    }
  } else if (pressed & PAD_B) {
    definition = definition->left_rotation;

    if (able_to_kick(definition->left_kick->deltas)) {
      banked_play_sfx(SFX::Rotate, GGSound::SFXPriority::One);
    } else {
      definition = definition->right_rotation; // undo rotation
    }
  }
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::jiggling() {
  definition->board_render(board, row, column, grounded_timer & 0b1);
  jiggling_timer++;
  if (jiggling_timer == 8) {
    jiggling_timer = 0;
    grounded_timer++;
    if (grounded_timer == 3) {
      state = State::Inactive;
    }
  }
}

void Polyomino::freezing_handler(bool &blocks_placed, bool &failed_to_place,
                                 u8 &lines_cleared) {
  s8 lines = freeze_blocks();
  if (lines >= 0) {
    lines_cleared = (u8)lines;
    blocks_placed = true;
  } else {
    failed_to_place = true;
    banked_play_sfx(SFX::Blockoverflow, GGSound::SFXPriority::One);
  }
}
__attribute__((noinline, section(POLYOMINOS_TEXT))) void
Polyomino::update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
                  u8 &lines_cleared) {
  if (state == State::Settling) {
    jiggling();
    return;
  }
  if (state == State::Inactive) {
    return;
  }
  if (drop_timer++ >= drop_frames) {
    drop_timer -= drop_frames;
    if (definition->collide(board, row + 1, column)) {
      if (grounded_timer >= MAX_GROUNDED_TIMER) {
        grounded_timer = 0;
        drop_timer = 0;
        movement_direction = Direction::None;
        freezing_handler(blocks_placed, failed_to_place, lines_cleared);
      } else {
        grounded_timer++;
      }
    } else {
      row++;
      if (movement_direction != Direction::Up) {
        grounded_timer = 0;
      }
    }
  }

  switch (movement_direction) {
  case Direction::Left:
    if (!definition->collide(board, row, column - 1)) {
      column--;
    }
    movement_direction = Direction::None;
    break;
  case Direction::Right:
    if (!definition->collide(board, row, column + 1)) {
      column++;
    }
    movement_direction = Direction::None;
    break;
  case Direction::Down:
    if (definition->collide(board, row + 1, column)) {
      freezing_handler(blocks_placed, failed_to_place, lines_cleared);
    } else {
      row++;
      movement_direction = Direction::None;
    }
  default:
    break;
  }
}

void Polyomino::render(int y_scroll) {
  if (state != State::Active)
    return;
  ScopedBank bank(GET_BANK(polyominos));
  definition->render(board.origin_x + (u8)(column << 4),
                     (board.origin_y - y_scroll + (row << 4)));
}

void Polyomino::outside_render(int y_scroll) {
  ScopedBank bank(GET_BANK(polyominos));
  definition->outside_render(board.origin_x + (u8)(column << 4),
                             (board.origin_y - y_scroll + (row << 4)),
                             board.origin_y - y_scroll);
}

void Polyomino::render_next() {
  ScopedBank iban(GET_BANK(polyominos));
  next->chibi_render(3, 5);
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) s8
Polyomino::freeze_blocks() {
  banked_play_sfx(SFX::Blockplacement, GGSound::SFXPriority::One);

  state = State::Settling;
  grounded_timer = 0;
  jiggling_timer = 0;
  s8 filled_lines = 0;
  if (!definition->board_render(board, row, column, true)) {
    return -1;
  }

  // XXX: checking the range of possible rows that could have been filled by a
  // polyomino
  for (s8 delta_row = -1; delta_row <= 2; delta_row++) {
    s8 block_row = row + delta_row;
    if (board.row_filled(block_row)) {
      filled_lines++;
    }
  }

  return filled_lines;
}
