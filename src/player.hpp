#pragma once

#include "direction.hpp"
#include "fixed-point.hpp"

#define move_speed fixed_point(1, 0x40)

class Player {
  enum class State {
    Idle,
    Moving,
  };

private:
  Direction facing;
  State state;
  u8 target_x, target_y;
public:
  fixed_point x;
  fixed_point y;
  Player(fixed_point starting_x, fixed_point starting_y);

  void update(u8 pressed, u8 held);
  void render();
};
