#pragma once

#include "animation-defs.hpp"
#include "animation.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"

#define PLAYER_BANK 0
#define PLAYER_TEXT_SECTION ".prg_rom_0.text.player"

class Unicorn {
  static constexpr fixed_point DEFAULT_MOVE_SPEED = fixed_point(1, 0x25);
  static constexpr fixed_point TIRED_MOVE_SPEED = fixed_point(0, 0x92);
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

  Animation idle_right_animation{idle_right_cells,
                                 sizeof(idle_right_cells) / sizeof(AnimCell)};
  Animation idle_left_animation{idle_left_cells,
                                sizeof(idle_left_cells) / sizeof(AnimCell)};

  Animation tired_right_animation{tired_right_cells,
                                  sizeof(tired_right_cells) / sizeof(AnimCell)};
  Animation tired_left_animation{tired_left_cells,
                                 sizeof(tired_left_cells) / sizeof(AnimCell)};

  Animation yawn_right_animation{yawn_right_cells,
                                 sizeof(yawn_right_cells) / sizeof(AnimCell)};
  Animation yawn_left_animation{yawn_left_cells,
                                sizeof(yawn_left_cells) / sizeof(AnimCell)};

  Animation sleep_right_animation{sleep_right_cells,
                                  sizeof(sleep_right_cells) / sizeof(AnimCell)};
  Animation sleep_left_animation{sleep_left_cells,
                                 sizeof(sleep_left_cells) / sizeof(AnimCell)};

  Animation moving_right_animation{
      moving_right_cells, sizeof(moving_right_cells) / sizeof(AnimCell)};
  Animation moving_left_animation{moving_left_cells,
                                  sizeof(moving_left_cells) / sizeof(AnimCell)};

  Animation trudging_right_animation{
      trudging_right_cells, sizeof(trudging_right_cells) / sizeof(AnimCell)};
  Animation trudging_left_animation{
      trudging_left_cells, sizeof(trudging_left_cells) / sizeof(AnimCell)};

  // fixes priority flags for bottom sprites
  void fix_uni_priority(bool left_wall, bool right_wall);

public:
  static constexpr u8 ENERGY_TICKS = 240;

  enum class State {
    Idle,
    Moving,
    Yawning,
    Sleeping,
    Trapped,
  };

  State state;
  Board &board;
  fixed_point x;
  fixed_point y;
  u16 score;

  u8 buffered_input;

  Animation trapped_animation{trapped_cells,
                              sizeof(trapped_cells) / sizeof(AnimCell)};

  Unicorn(Board &board, fixed_point starting_x, fixed_point starting_y);

  void update(u8 pressed, u8 held);
  void render(int y_scroll, bool left_wall, bool right_wall);
  void feed(u8 nutrition);
  void energy_upkeep();
  void refresh_energy_hud(int y_scroll);
  void refresh_score_hud();
  void set_state(State new_state);
  const fixed_point &move_speed();
};
