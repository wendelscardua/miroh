#include "polyomino.hpp"
#include "bag.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "common.hpp"
#include "direction.hpp"
#include "ggsound.hpp"
#include "log.hpp"
#include "polyomino-defs.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_0.text.polyominos"
#pragma clang section rodata = ".prg_rom_0.rodata.polyominos"

// NOTE: source file defines indices [0, 4) as littleminos
static auto littleminos = Bag<u8, 4>(NULL);

// NOTE: source file defines indices [11, 28) as pentominos
static auto pentominos = Bag<u8, 17>(NULL);

// NOTE: source file defines indices [4, 11) as tetrominos
auto Polyomino::pieces = Bag<u8, 10>([](auto value) {
  if (value < 4) {
    return littleminos.take();
  }

  if (value >= 11) {
    return pentominos.take();
  }

  return value;
});

Polyomino::Polyomino(Board &board)
    : board(board), definition(NULL), state(State::Inactive) {

  // initialize littleminos bag
  littleminos.reset();
  for (u8 i = 0; i < 4; i++) {
    littleminos.insert(i);
  }

  // initialize pentominos bag
  pentominos.reset();
  for (u8 i = 11; i < 28; i++) {
    pentominos.insert(i);
  }

  // add all tetrominos to the pieces bag
  // NOTE: source file defines indices [4, 11) as tetrominos
  pieces.reset();
  for (u8 i = 4; i < 11; i++) {
    pieces.insert(i);
  }

  // also add two random "littleminos" (1,2, or 3 blocks)
  pieces.insert(littleminos.take());
  pieces.insert(littleminos.take());

  // ... and a random pentomino
  pieces.insert(pentominos.take());

  next = polyominos[pieces.take()];

  render_next();
}

void Polyomino::spawn() {
  state = State::Active;
  grounded_timer = 0;
  move_timer = 0;
  movement_direction = Direction::None;
  column = 4;
  row = 0;
  x = board.origin_x + (u8)(column << 4);
  y = board.origin_y + (u8)(row << 4);

  definition = next;
  next = polyominos[pieces.take()];

  render_next();

  for (u8 i = 0; i < 4; i++) {
    bitmask[i] = 0;
  }

  s8 max_delta = 0;
  for (auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) {
      max_delta = delta.delta_row;
    }
  }
  row -= (max_delta + 1);
  y -= (max_delta + 1) * 16;

  update_bitmask();
}

void Polyomino::update_bitmask() {
  for (u8 i = 0; i < 4; i++) {
    bitmask[i] = 0;
  }

  left_limit = 4;
  right_limit = 0;
  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    if (delta.delta_column > right_limit) {
      right_limit = (u8)delta.delta_column;
    }
    if (delta.delta_column < left_limit) {
      left_limit = (u8)delta.delta_column;
    }
    bitmask[(u8)(delta.delta_row)] |=
        Board::OCCUPIED_BITMASK[(u8)(column + delta.delta_column)];
  }
  update_shadow();
}

void Polyomino::update_shadow() {
  START_MESEN_WATCH(4);
  shadow_row = row;
  shadow_y = y;
  while (!collide(shadow_row + 1, (s8)column)) {
    shadow_row++;
    shadow_y += 16;
  }
  STOP_MESEN_WATCH(4);
}

inline u16 signed_shift(u16 value, s8 shift) {
  if (shift == 0) {
    return value;
  } else if (shift > 0) {
    return value << 1;
  } else {
    return value >> 1;
  }
}

bool Polyomino::collide(s8 new_row, s8 new_column) {
  START_MESEN_WATCH(31);
  if (left_limit + new_column < 0 || right_limit + new_column >= WIDTH) {
    STOP_MESEN_WATCH(31);
    return true;
  }
#pragma clang loop unroll(full)
  for (u8 i = 0; i < 4; i++) {
    s8 mod_row = (s8)(new_row + i);
    if (mod_row < 0) {
      continue;
    }
    if (bitmask[i] && (mod_row >= HEIGHT ||
                       (signed_shift(bitmask[i], (new_column - (s8)column)) &
                        board.occupied_bitset[(u8)(new_row + i)]))) {
      STOP_MESEN_WATCH(31);
      return true;
    }
  }
  STOP_MESEN_WATCH(31);
  return false;
}

bool Polyomino::able_to_kick(const auto &kick_deltas) {
  for (auto kick : kick_deltas) {
    s8 new_row = row + kick.delta_row;
    s8 new_column = column + kick.delta_column;

    if (!definition->collide(board, new_row, new_column)) {
      row = new_row;
      column = new_column;
      x += 16 * kick.delta_column;
      y += 16 * kick.delta_row;
      return true;
    }
  }
  return false;
}

void Polyomino::handle_input(u8 pressed, u8 held) {
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
      update_bitmask();
    } else {
      definition = definition->left_rotation; // undo rotation
    }
  } else if (pressed & PAD_B) {
    definition = definition->left_rotation;

    if (able_to_kick(definition->left_kick->deltas)) {
      banked_play_sfx(SFX::Rotate, GGSound::SFXPriority::One);
      update_bitmask();
    } else {
      definition = definition->right_rotation; // undo rotation
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
  }
}

void Polyomino::update(u8 drop_frames, bool &blocks_placed,
                       bool &failed_to_place, u8 &lines_cleared) {
  if (state == State::Inactive) {
    return;
  }
  if (drop_timer++ >= drop_frames) {
    drop_timer -= drop_frames;
    if (collide(row + 1, (s8)column)) {
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
      y += 16;
      if (movement_direction != Direction::Up) {
        grounded_timer = 0;
      }
    }
    return;
  }

  switch (movement_direction) {
  case Direction::Left:
    if (!collide(row, column - 1)) {
      column--;
      x -= 16;
#pragma clang loop unroll(full)
      for (u8 i = 0; i < 4; i++) {
        bitmask[i] = bitmask[i] >> 1;
      }
      update_shadow();
    }
    movement_direction = Direction::None;
    break;
  case Direction::Right:
    if (!collide(row, column + 1)) {
      column++;
      x += 16;
#pragma clang loop unroll(full)
      for (u8 i = 0; i < 4; i++) {
        bitmask[i] = bitmask[i] << 1;
      }
      update_shadow();
    }
    movement_direction = Direction::None;
    break;
  case Direction::Down:
    if (collide(row + 1, (s8)column)) {
      freezing_handler(blocks_placed, failed_to_place, lines_cleared);
    } else {
      row++;
      y += 16;
      movement_direction = Direction::None;
    }
  case Direction::Up:
  case Direction::None:
    if (board.active_animations) {
      update_shadow();
    }
    break;
  }
}

void Polyomino::render(int y_scroll) {
  if (state != State::Active)
    return;
  START_MESEN_WATCH(5);
  definition->render(x, y - y_scroll);
  STOP_MESEN_WATCH(5);
  START_MESEN_WATCH(6);
  definition->shadow(x, shadow_y - y_scroll, (u8)(shadow_row - row));
  STOP_MESEN_WATCH(6);
}

void Polyomino::outside_render(int y_scroll) {
  definition->render(x, y - y_scroll);
}

void Polyomino::render_next() { next->chibi_render(3, 5); }

s8 Polyomino::freeze_blocks() {
  state = State::Inactive;
  grounded_timer = 0;
  s8 filled_lines = 0;
  if (!definition->board_render(board, row, column)) {
    return -1;
  }

// XXX: checking the range of possible rows that could have been filled by a
// polyomino
#pragma clang loop unroll(full)
  for (s8 delta_row = 0; delta_row <= 3; delta_row++) {
    s8 block_row = row + delta_row;
    if (board.row_filled(block_row)) {
      filled_lines++;
    }
  }

  return filled_lines;
}
