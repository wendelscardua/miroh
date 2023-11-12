#pragma once

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
  union {
    u8 energy;
    u8 ghost_height;
  };
  s16 energy_timer;
  u8 original_energy;
  u8 sprite_offset;

  // fixes priority flags for bottom sprites
  void fix_uni_priority(bool left_wall, bool right_wall);

public:
  static constexpr u8 ENERGY_TICKS = 240;

  enum class State {
    Idle,
    Moving,
    Dying,
    Dead,
  };

  State state;
  Board &board;
  fixed_point x;
  fixed_point y;
  u16 score;
  u8 lines;

  u8 buffered_input;

  Player(Board &board, fixed_point starting_x, fixed_point starting_y);

  void update(InputMode input_mode, u8 pressed, u8 held);
  void render(int y_scroll, bool left_wall, bool right_wall);
  void feed(u8 nutrition);
  void energy_upkeep(s16 delta);
  void refresh_energy_hud(int y_scroll);
  void refresh_score_hud();
  const fixed_point &move_speed();
};
