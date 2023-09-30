#include "player.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "gameplay.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
    : board(board) {
  this->x = starting_x;
  this->y = starting_y;
  this->facing = Direction::Right;
  this->state = State::Idle;
}

void Player::update(InputMode input_mode, u8 pressed, u8 held) {
restate:
  switch (state) {
  case State::Idle: {
    auto current_cell = board.cell[y.whole >> 4][x.whole >> 4];
    if (input_mode != InputMode::Player) break;
    if (pressed & PAD_UP) {
      facing = Direction::Up;
      if (!current_cell.up_wall && y.whole > 0) {
        target_x = x;
        target_y = y - GRID_SIZE;
        state = State::Moving;
        goto restate;
      }
    }
    if (pressed & PAD_DOWN) {
      facing = Direction::Down;
      if (!current_cell.down_wall && y.whole < 0x10 * (SIZE - 1)) {
        target_x = x;
        target_y = y + GRID_SIZE;
        state = State::Moving;
        goto restate;
      }
    }
    if (pressed & PAD_LEFT) {
      facing = Direction::Left;
      if (!current_cell.left_wall && x.whole > 0) {
        target_x = x - GRID_SIZE;
        target_y = y;
        state = State::Moving;
        goto restate;
      }
    }
    if (pressed & PAD_RIGHT) {
      facing = Direction::Right;
      if (!current_cell.right_wall && x.whole < 0x10 * (SIZE - 1)) {
        target_x = x + GRID_SIZE;
        target_y = y;
        state = State::Moving;
        goto restate;
      }
    }
  } break;
  case State::Moving:
    switch (facing) {
    case Direction::Up:
      y -= move_speed;
      if (y < target_y) {
        y = target_y;
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Right:
      x += move_speed;
      if (x > target_x) {
        x = target_x;
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Down:
      y += move_speed;
      if (y > target_y) {
        y = target_y;
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Left:
      x -= move_speed;
      if (x < target_x) {
        x = target_x;
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::None:
      break;
    }
    break;
  }
}

void Player::render() {
  const u8 *metasprite;
  switch (state) {
  case State::Idle:
    switch (facing) {
    case Direction::Up:
    case Direction::Right:
    case Direction::Down:
    case Direction::None:
      metasprite = metasprite_MinoRight1;
      break;
    case Direction::Left:
      metasprite = metasprite_MinoLeft1;
      break;
    }
    break;
  case State::Moving:
    bool toggle = (get_frame_count() & 0b1000) == 0;
    switch (facing) {
    case Direction::Up:
    case Direction::Right:
    case Direction::Down:
    case Direction::None:
      if (toggle) {
        metasprite = metasprite_MinoRight1;
      } else {
        metasprite = metasprite_MinoRight2;
      }
      break;
    case Direction::Left:
      if (toggle) {
        metasprite = metasprite_MinoLeft1;
      } else {
        metasprite = metasprite_MinoLeft2;
      }
      break;
    }
    break;
  }
  oam_meta_spr(board.origin_x + (u8)x.round(), board.origin_y + (u8)y.round(),
               metasprite);
}
