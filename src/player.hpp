#pragma once

#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"

#define HUNGER_TICKS 90
#define MAX_HUNGER 32
#define HUNGER_BAR_BASE_TILE 0x92

#define PLAYER_BANK 1
#define PLAYER_TEXT_SECTION ".prg_rom_1.text.player"

class Player {
  static constexpr fixed_point DEFAULT_MOVE_SPEED = fixed_point(1, 0x2000);
  static constexpr fixed_point FAST_MOVE_SPEED = fixed_point(2, 0x0000);
private:
  Direction facing;
  Direction moving;
  fixed_point target_x, target_y;
  union {
    u8 hunger;
    u8 ghost_height;
  };
  s16 hunger_timer;
public:
  enum class State {
    Idle,
    Moving,
    Dying,
    Dead,
  };

  State state;
  Board& board;
  fixed_point x;
  fixed_point y;
  u16 score;

  u8 buffered_input;

  Player(Board& board, fixed_point starting_x, fixed_point starting_y);

  void update(InputMode input_mode, u8 pressed, u8 held);
  void render();
  void feed(u8 nutrition);
  void hunger_upkeep(s16 delta);
  void refresh_hunger_hud();
  void refresh_score_hud();
  const fixed_point& move_speed();
};
