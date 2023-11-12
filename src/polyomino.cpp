#include "polyomino.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "ggsound.hpp"
#include "input-mode.hpp"
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
Polyomino::handle_input(InputMode input_mode) {
  u8 pressed, held;

  switch (current_controller_scheme) {
  case ControllerScheme::OnePlayer:
    if (input_mode == InputMode::Polyomino) {
      pressed = get_pad_new(0);
      held = pad_state(0);
    } else {
      pressed = 0;
      held = 0;
    }
    break;
  case ControllerScheme::TwoPlayers:
    pressed = get_pad_new(0);
    held = pad_state(0);
    break;
  }

  if (pressed & PAD_UP) {
    // just some high enough value for the drop to proceed until the end
    drop_timer = HEIGHT * 60;
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
      banked_play_sfx(SFX::Number3, GGSound::SFXPriority::One);
    } else {
      definition = definition->left_rotation; // undo rotation
    }
  } else if (pressed & PAD_B) {
    definition = definition->left_rotation;

    if (able_to_kick(definition->left_kick->deltas)) {
      banked_play_sfx(SFX::Number3, GGSound::SFXPriority::One);
    } else {
      definition = definition->right_rotation; // undo rotation
    }
  }
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void Polyomino::jiggling() {
  CORO_INIT;

  for (grounded_timer = 0; grounded_timer < 3; grounded_timer++) {
    for (jiggling_timer = 0; jiggling_timer < 8; jiggling_timer++) {
      CORO_YIELD();
    }
    definition->board_render(board, row, column, grounded_timer & 0b1);
    CORO_YIELD();
  }
  state = State::Inactive;

  CORO_FINISH();
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) void
Polyomino::update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
                  u8 &lines_filled) {
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
      if (can_be_frozen()) {
        lines_filled = freeze_blocks();
        blocks_placed = true;
      } else {
        failed_to_place = true;
      }
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

  banked_lambda(GET_BANK(polyominos), [this, y_scroll]() {
    definition->render(board.origin_x + (u8)(column << 4),
                       (board.origin_y - y_scroll + (row << 4)));
  });
}

void Polyomino::render_next() {
  banked_lambda(GET_BANK(polyominos), [this]() { next->chibi_render(3, 5); });
}

__attribute__((noinline, section(POLYOMINOS_TEXT))) bool
Polyomino::can_be_frozen() {
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
  banked_play_sfx(SFX::Number4, GGSound::SFXPriority::Two);

  state = State::Settling;
  jiggling_timer = 0;
  u8 filled_lines = 0;
  definition->board_render(board, row, column, true);
  for (u8 i = 0; i < definition->size; i++) {
    auto delta = definition->deltas[i];
    s8 block_row = row + delta.delta_row;
    if (board.row_filled(block_row)) {
      filled_lines++;
    }
  }

  return filled_lines;
}
