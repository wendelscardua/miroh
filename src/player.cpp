#include "player.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
  : facing(Direction::Right),
    moving(Direction::Right),
    hunger(0),
    hunger_timer(0),
    state(State::Idle),
    board(board),
    x(starting_x),
    y(starting_y) {}

void Player::update(InputMode input_mode, u8 pressed, u8 held) {
  hunger_timer++;
  if (hunger_timer >= HUNGER_TICKS) {
    hunger_timer = 0;
    if (hunger == MAX_HUNGER) {
      // TODO: player dies
    } else {
      hunger++;
      refresh_hunger_hud();
    }
  }
  switch (state) {
  case State::Idle: {
    auto current_row = y.whole >> 4;
    auto current_column = x.whole >> 4;
    auto current_cell = board.cell[current_row][current_column];
    if (input_mode != InputMode::Player) break;
    if (held & PAD_UP) {
      moving = Direction::Up;
      if (!current_cell.up_wall &&
          current_row > 0 &&
          !board.occupied((s8)(current_row - 1), (s8)current_column)) {
        target_x = x;
        target_y = y - GRID_SIZE;
        state = State::Moving;
      }
    }
    if (held & PAD_DOWN) {
      moving = Direction::Down;
      if (!current_cell.down_wall &&
          !board.occupied((s8)(current_row + 1), (s8)current_column)) {
        target_x = x;
        target_y = y + GRID_SIZE;
        state = State::Moving;
      }
    }
    if (held & PAD_LEFT) {
      facing = Direction::Left;
      moving = Direction::Left;
      if (!current_cell.left_wall &&
          !board.occupied((s8)current_row, (s8)(current_column - 1))) {
        target_x = x - GRID_SIZE;
        target_y = y;
        state = State::Moving;
      }
    }
    if (held & PAD_RIGHT) {
      facing = Direction::Right;
      moving = Direction::Right;
      if (!current_cell.right_wall &&
          !board.occupied((s8)current_row, (s8)(current_column + 1))) {
        target_x = x + GRID_SIZE;
        target_y = y;
        state = State::Moving;
      }
    }
  } break;
  case State::Moving:
    switch (moving) {
    case Direction::Up:
      y -= move_speed;
      if (y < target_y) {
        y = target_y;
        state = State::Idle;
      }
      break;
    case Direction::Right:
      x += move_speed;
      if (x > target_x) {
        x = target_x;
        state = State::Idle;
      }
      break;
    case Direction::Down:
      y += move_speed;
      if (y > target_y) {
        y = target_y;
        state = State::Idle;
      }
      break;
    case Direction::Left:
      x -= move_speed;
      if (x < target_x) {
        x = target_x;
        state = State::Idle;
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
    bool toggle = ((x.whole ^ y.whole) & 0b1000) != 0;
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

void Player::feed(u8 nutrition) {
  hunger_timer = 0;
  if (hunger > nutrition) {
    hunger -= nutrition;
  } else {
    hunger = 0;
  }
  refresh_hunger_hud();
}

void Player::refresh_hunger_hud() {
  // refresh hunger hud
  u8 hunger_bar[4];
  u8 temp = hunger;
  for(auto &hunger_cell : hunger_bar) {
    if (temp >= 8) {
      hunger_cell = HUNGER_BAR_BASE_TILE + 8;
      temp -= 8;
    } else {
      hunger_cell = HUNGER_BAR_BASE_TILE + temp;
      temp = 0;
    }
  }

  multi_vram_buffer_horz(hunger_bar, 4, NTADR_A(12, 27));
}
