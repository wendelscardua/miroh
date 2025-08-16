#pragma once

#include "bag.hpp"
#include "board.hpp"
#include "polyomino-defs.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define MAX_GROUNDED_TIMER 30
#define FROZEN_BLOCK_ATTRIBUTE 2

class Polyomino {
  static constexpr s8 MOVEMENT_INITIAL_DELAY = 10 - 1;
  static constexpr s8 MOVEMENT_DELAY = 2 - 1;
  static constexpr s8 ROTATION_INITIAL_DELAY = 32 - 1;
  static constexpr s8 ROTATION_DELAY = 16 - 1;
  static constexpr u8 SPAWN_COLUMN = 4;

public:
  static constexpr u8 BANK = 14;
  enum class State {
    Inactive,
    Active,
  };
  u8 grounded_timer;
  State state;
  Polyomino(Board &board);

  __attribute__((noinline)) void init();
  __attribute__((noinline)) u8 take_piece();
  __attribute__((noinline)) void spawn();
  __attribute__((noinline)) void handle_input(u8 pressed, u8 held);
  void update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
              u8 &lines_filled);
  void render(int y_scroll);
  void outside_render(int y_scroll);

private:
  enum class Action {
    Idle = 0,
    MoveLeft = 1,
    MoveRight = 2,
    MoveDown = 4,
    Drop = 8,
    RotateLeft = 16,
    RotateRight = 32,
    MoveLeftAndRotateLeft = MoveLeft | RotateLeft,
    MoveLeftAndRotateRight = MoveLeft | RotateRight,
    MoveRightAndRotateLeft = MoveRight | RotateLeft,
    MoveRightAndRotateRight = MoveRight | RotateRight,
  };

  static Bag<u8, 10> pieces;
  static Bag<u8, 4> littleminos;
  static Bag<u8, 17> pentominos;

  Board &board;
  const PolyominoDef *definition;
  const PolyominoDef *next;
  s8 row;
  s8 column;
  u8 x;
  u8 y;
  u16 drop_timer;
  u8 move_timer;
  u8 rotate_timer;
  Action action;
  s8 shadow_row;
  u8 shadow_y;
  u8 left_limit;
  u8 right_limit;
  u8 top_limit;
  u8 bottom_limit;
  soa::Array<u16, 4> bitmask;

  bool able_to_kick(const auto &kick_deltas);
  void freezing_handler(bool &blocks_placed, bool &failed_to_place,
                        u8 &lines_cleared);
  void render_next();
  bool collide(s8 row, s8 column);
  void update_bitmask();
  void move_bitmask_left();
  void move_bitmask_right();
  void update_shadow();

  // returns number of filled lines aftter blocks were frozen, or -1 if
  // polyomino didn't fit
  s8 freeze_blocks();
};
