#include "player.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "metasprites.hpp"
#include <neslib.h>

Player::Player(fixed_point starting_x, fixed_point starting_y) {
  this->x = starting_x;
  this->y = starting_y;
  this->facing = Direction::Right;
  this->state = State::Idle;
}

void Player::update(u8 pressed, u8 held) {}

void Player::render() {
  oam_meta_spr((u8)x.round(),
               (u8)y.round(),
               metasprite_MinoRight1);
}
