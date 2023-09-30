#include "player.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

Player::Player(fixed_point starting_x, fixed_point starting_y) {
  this->x = starting_x;
  this->y = starting_y;
  this->facing = Direction::Right;
  this->state = State::Idle;
}

void Player::update(u8 pressed, u8 held) {
 restate:
  switch(state) {
  case State::Idle:
    if (pressed & PAD_UP) {
      facing = Direction::Up;
      target_x = x.whole;
      target_y = (y.whole - 0x10);
      state = State::Moving;
      goto restate;
    }
    if (pressed & PAD_DOWN) {
      facing = Direction::Down;
      target_x = x.whole;
      target_y = (y.whole + 0x10);
      state = State::Moving;
      goto restate;
    }
    if (pressed & PAD_LEFT) {
      facing = Direction::Left;
      target_x = (x.whole - 0x10);
      target_y = y.whole;
      state = State::Moving;
      goto restate;
    }
    if (pressed & PAD_RIGHT) {
      facing = Direction::Right;
      target_x = (x.whole + 0x10);
      target_y = y.whole;
      state = State::Moving;
      goto restate;
    }
    break;
  case State::Moving:
    switch(facing) {
    case Direction::Up:
      y -= move_speed;
      if (y.whole < target_y) {
        y = fixed_point(target_y, 0);
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Right:
      x += move_speed;
      if (x.whole > target_x) {
        x = fixed_point(target_x, 0);
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Down:
      y += move_speed;
      if (y.whole > target_y) {
        y = fixed_point(target_y, 0);
        state = State::Idle;
        goto restate;
      }
      break;
    case Direction::Left:
      x -= move_speed;
      if (x.whole < target_x) {
        x = fixed_point(target_x, 0);
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
  const u8* metasprite;
  switch(state) {
  case State::Idle:
    switch(facing) {
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
    switch(facing) {
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
  oam_meta_spr((u8)x.round(),
               (u8)y.round(),
               metasprite);
}
