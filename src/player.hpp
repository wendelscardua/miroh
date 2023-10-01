#pragma once

#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"

#define move_speed fixed_point(1, 0x2000)
#define HUNGER_TICKS 90
#define MAX_HUNGER 16
#define HUNGER_BAR_BASE_TILE 0x92

class Player {
private:
  Direction facing;
  fixed_point target_x, target_y;
  u8 hunger;
  s16 hunger_timer;
public:
  enum class State {
    Idle,
    Moving,
  };

  State state;
  Board& board;
  fixed_point x;
  fixed_point y;
  Player(Board& board, fixed_point starting_x, fixed_point starting_y);

  void update(InputMode input_mode, u8 pressed, u8 held);
  void render();
  void feed(u8 nutrition);
  void refresh_hunger_hud();
};
