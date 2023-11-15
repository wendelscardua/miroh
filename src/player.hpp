#pragma once

#include "animation-defs.hpp"
#include "animation.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"

#define PLAYER_BANK 0
#define PLAYER_TEXT_SECTION ".prg_rom_0.text.player"

class Player {
  static constexpr fixed_point DEFAULT_MOVE_SPEED = fixed_point(1, 0x25);
  static constexpr u8 MAX_ENERGY = 12;
  static constexpr u8 STARTING_ENERGY = 9;

private:
  Direction facing;
  Direction moving;
  fixed_point target_x, target_y;
  u8 energy;
  s16 energy_timer;
  u8 original_energy;
  u8 sprite_offset;

  Animation<4> idleRightAnimation{&idle_right_cells};

  Animation<4> idleLeftAnimation{&idle_left_cells};

  Animation<4> movingRightAnimation{&moving_right_cells};

  Animation<4> movingLeftAnimation{&moving_left_cells};
  // fixes priority flags for bottom sprites
  void fix_uni_priority(bool left_wall, bool right_wall);

public:
  static constexpr u8 ENERGY_TICKS = 240;

  enum class State {
    Idle,
    Moving,
  };

  State state;
  Board &board;
  fixed_point x;
  fixed_point y;
  u16 score;
  u8 lines;

  u8 buffered_input;

  Player(Board &board, fixed_point starting_x, fixed_point starting_y);

  void update(InputMode input_mode);
  void render(int y_scroll, bool left_wall, bool right_wall);
  void feed(u8 nutrition);
  void energy_upkeep(s16 delta);
  void refresh_energy_hud(int y_scroll);
  void refresh_score_hud();
  const fixed_point &move_speed();
};
