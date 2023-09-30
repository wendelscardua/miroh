#include "polyomino.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"
#include "log.hpp"
#include "metasprites.hpp"
#include "polyominos.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Polyomino::Polyomino(Board &board, bool active)
    : board(board), definition(NULL), active(active) {}

void Polyomino::spawn() {
  active = true;
  grounded_timer = 0;
  x = fixed_point(0x50, 0);
  y = fixed_point(0x00, 0);
  u8 random_index;
  do {
    random_index = rand8() & 0x7f;
  } while (random_index >= NUM_POLYOMINOS);
  definition = polyominos[random_index];
  s8 max_delta = 0;
  for(auto delta : definition->deltas) {
    if (delta.delta_row > max_delta) max_delta = delta.delta_row;
  }
  y -= GRID_SIZE * (u8)(max_delta + 1);
  target_x = x;
  target_y = y;
}

void Polyomino::update(InputMode input_mode, u8 pressed, u8 held) {
  if (!active) {
    if (input_mode == InputMode::Polyomino) {
      input_mode = InputMode::Player;
    }
    return;
  }

  fixed_point drop_speed;

  if ((input_mode == InputMode::Polyomino) && (held & PAD_DOWN)) {
    drop_speed = DROP_SPEED * 4;
  } else {
    drop_speed = DROP_SPEED;
  }

  if (target_y > y) {
    grounded_timer = 0;
    y += drop_speed;
    if (y > target_y) y = target_y;
  }

  if (target_y == y) {
    target_y = y + GRID_SIZE;
    bool bumped = false;
    for(u8 i = 0; i < definition->size; i++) {
      auto& delta = definition->deltas[i];
      if (board.occupied(x.whole + delta.delta_x(),
                         target_y.whole + delta.delta_y()) ||
          board.occupied(x.whole + delta.delta_x() + 0x0f,
                         target_y.whole + delta.delta_y())) {
        bumped = true;
        break;
      }
    }
    if (bumped) {
      target_y = y;
      grounded_timer++;
    }
  }

  if (target_x == x) {
    if (grounded_timer >= MAX_GROUNDED_TIMER) {
      freeze_blocks();
      input_mode = InputMode::Player;
      return;
    }

    if (input_mode == InputMode::Polyomino) {
      if (pressed & PAD_LEFT) {
        target_x = x - GRID_SIZE;
      }
      if (pressed & PAD_RIGHT) {
        target_x = x + GRID_SIZE;
      }
      if (target_x != x) {
        bool bumped = false;
        for(u8 i = 0; i < definition->size; i++) {
          auto& delta = definition->deltas[i];
          if (board.occupied(target_x.whole + delta.delta_x(),
                             y.whole + delta.delta_y()) ||
              board.occupied(target_x.whole + delta.delta_x(),
                             y.whole + delta.delta_y() + 0x0f) ) {
            bumped = true;
            break;
          }
        }
        if (bumped) {
          target_x = x;
        }
      }
    }
  }

  if (target_x < x) {
    x -= HORIZONTAL_SPEED;
    if (x < target_x) x = target_x;
  }

  if (target_x > x) {
    x += HORIZONTAL_SPEED;
    if (x > target_x) x = target_x;
  }
}

void Polyomino::render() {
  if (!active)
    return;

  for (u8 i = 0; i < definition->size; i++) {
    auto& delta = definition->deltas[i];
    oam_meta_spr(board.origin_x +
                 (u8)(x.whole + delta.delta_x()),
                 board.origin_y +
                 (u8)(y.whole + delta.delta_y()),
                 metasprite_block);
  }
}

void Polyomino::freeze_blocks() {
  active = false;

  for (u8 i = 0; i < definition->size; i++) {
    auto& delta = definition->deltas[i];
    s16 block_x = x.whole + delta.delta_x();
    s16 block_y = y.whole + delta.delta_y();
    if (!board.occupied(block_x, block_y)) {
      board.get_cell((u8)block_x, (u8)block_y).occupied = true;
      int position = NTADR_A((board.origin_x + block_x) >> 3, (board.origin_y + block_y) >> 3);
      multi_vram_buffer_horz((const u8[2]){0x04, 0x05}, 2, position);
      multi_vram_buffer_horz((const u8[2]){0x14, 0x15}, 2, position+0x20);
      // TODO set attribute
    }
  }
}
