#pragma once

#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"

#define move_speed fixed_point(1, 0x40)

class Player {
  enum class State {
    Idle,
    Moving,
  };

private:
  Direction facing;
  State state;
  fixed_point target_x, target_y;
public:
  Board& board;
  fixed_point x;
  fixed_point y;
  Player(Board& board, fixed_point starting_x, fixed_point starting_y);

  void update(InputMode input_mode, u8 pressed, u8 held);
  void render();
};
