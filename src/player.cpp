#include "player.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"

Player::Player(fixed_point starting_x, fixed_point starting_y) {
  this->x = starting_x;
  this->y = starting_y;
  this->facing = Direction::Right;
  this->state = State::Idle;
}

void Player::update(u8 pressed, u8 held) {}

void Player::render() {}
