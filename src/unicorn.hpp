#pragma once

#include "animation-defs.hpp"
#include "animation.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"

class Unicorn {
  static constexpr fixed_point DEFAULT_MOVE_SPEED = fixed_point(1, 0x25);
  static constexpr fixed_point TIRED_MOVE_SPEED = fixed_point(0, 0x92);
  static constexpr u8 MAX_ENERGY = 12;
  static constexpr u8 STARTING_ENERGY = 9;
  static constexpr u8 CHARGE_COST = 3;

private:
  Direction facing;
  Direction moving;
  fixed_point target_x, target_y;
  u8 energy;
  u8 energy_timer;
  u8 original_energy;
  u8 sprite_offset;

  // fixes priority flags for bottom sprites
  void fix_uni_priority(bool left_wall, bool right_wall);

public:
  static constexpr u8 BANK = 5;
  static constexpr u8 ENERGY_TICKS = 240;

  enum class State {
    Idle,
    Moving,
    Yawning,
    Sleeping,
    Trapped,
    Roll,
    Impact,
  };

  State state;
  Board &board;
  fixed_point x;
  fixed_point y;
  u8 row;
  u8 column;
  u8 roll_distance;
  bool roll_into_block;
  u16 score;

  u8 buffered_input;

  bool statue;

  Animation idle_left_animation{NULL};
  Animation idle_right_animation{NULL};
  Animation idle_left_tired_animation{NULL};
  Animation idle_right_tired_animation{NULL};
  Animation left_animation{NULL};
  Animation right_animation{NULL};
  Animation left_tired_animation{NULL};
  Animation right_tired_animation{NULL};

  Animation generic_animation{NULL};

  __attribute__((section(".prg_rom_fixed.text.unicorn")))
  Unicorn(Board &board, fixed_point starting_x, fixed_point starting_y);

  __attribute__((noinline)) void update(u8 pressed, u8 held,
                                        bool roll_disabled);
  void render(int y_scroll, bool left_wall, bool right_wall);
  void feed(u8 nutrition);
  void energy_upkeep();
  void refresh_energy_hud(int y_scroll);
  void refresh_score_hud();
  __attribute__((noinline)) void set_state(State new_state);
  const fixed_point &move_speed();
};
