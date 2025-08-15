#pragma once

#include "animation-defs.hpp"
#include "animation.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"

class Unicorn {
  static constexpr fixed_point DEFAULT_MOVE_SPEED = 1.14453125_fp;
  static constexpr fixed_point TIRED_MOVE_SPEED = 0.5703125_fp;
  static constexpr u8 MAX_ENERGY = 12;
  static constexpr u8 STARTING_ENERGY = 9;
  static constexpr u8 CHARGE_COST = 3;

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
  fixed_point x;
  fixed_point y;
  u8 row;
  u8 column;
  u16 score;
  bool left_wall, right_wall;

  bool statue;

  Animation generic_animation{NULL};

  __attribute__((section(".prg_rom_fixed.text.unicorn")))
  Unicorn(Board &board, fixed_point starting_x, fixed_point starting_y);

  __attribute__((noinline)) void update(u8 pressed, u8 held,
                                        bool roll_disabled);
  void render(int y_scroll);
  void feed(u8 nutrition);
  void refresh_energy_hud(int y_scroll);
  void refresh_score_hud();
  void add_score(u8 points);

private:
  Board &board;

  Direction facing;
  Direction moving;
  fixed_point target_x, target_y;
  u8 energy;
  u8 energy_timer;
  u8 original_energy;

  u8 roll_distance;
  bool roll_into_block;

  u8 buffered_input;

  Animation idle_left_animation{NULL};
  Animation idle_right_animation{NULL};
  Animation idle_left_tired_animation{NULL};
  Animation idle_right_tired_animation{NULL};
  Animation left_animation{NULL};
  Animation right_animation{NULL};
  Animation left_tired_animation{NULL};
  Animation right_tired_animation{NULL};

  // fixes priority flags for bottom sprites
  void fix_uni_priority(u8 sprite_offset, bool left_wall, bool right_wall);
  void energy_upkeep();
  void set_state(State new_state);
  const fixed_point &move_speed();
};
