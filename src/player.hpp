#pragma once

#include "animation.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"
#include "metasprites.hpp"

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

  const soa::Array<AnimCell, 4> idleRightCells{
      {(void *)metasprite_UniRightIdle, 162},
      {(void *)metasprite_UniRightBlink, 12},
      {(void *)metasprite_UniRightIdle, 8},
      {(void *)metasprite_UniRightBlink, 12}};

  const soa::Array<AnimCell, 4> idleLeftCells{
      {(void *)metasprite_UniLeftIdle, 162},
      {(void *)metasprite_UniLeftBlink, 12},
      {(void *)metasprite_UniLeftIdle, 8},
      {(void *)metasprite_UniLeftBlink, 12}};

  const soa::Array<AnimCell, 4> movingRightCells{
      {(void *)metasprite_UniRightWalk1, 5},
      {(void *)metasprite_UniRightWalk2, 9},
      {(void *)metasprite_UniRightWalk3, 5},
      {(void *)metasprite_UniRightWalk4, 9}};

  const soa::Array<AnimCell, 4> movingLeftCells{
      {(void *)metasprite_UniLeftWalk1, 5},
      {(void *)metasprite_UniLeftWalk2, 9},
      {(void *)metasprite_UniLeftWalk3, 5},
      {(void *)metasprite_UniLeftWalk4, 9}};
  Animation<4> idleRightAnimation{&idleRightCells};

  Animation<4> idleLeftAnimation{&idleLeftCells};

  Animation<4> movingRightAnimation{&movingRightCells};

  Animation<4> movingLeftAnimation{&movingLeftCells};
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
