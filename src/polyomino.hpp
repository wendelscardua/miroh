#pragma once

#include "bag.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "polyomino-defs.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define MAX_GROUNDED_TIMER 2
#define FROZEN_BLOCK_ATTRIBUTE 2

class Polyomino {
  static constexpr s8 MOVEMENT_INITIAL_DELAY = 16;
  static constexpr s8 MOVEMENT_DELAY = 6;

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
  s8 move_timer;
  Direction movement_direction;
  s8 shadow_row;
  u8 shadow_y;
  u8 left_limit;
  u8 right_limit;
  soa::Array<u16, 4> bitmask;

  __attribute__((noinline)) bool able_to_kick(const auto &kick_deltas);

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

  void freezing_handler(bool &blocks_placed, bool &failed_to_place,
                        u8 &lines_cleared);
  void update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
              u8 &lines_filled);

  void banked_render();

  void render(int y_scroll);

  void outside_render(int y_scroll);

  void render_next();

  bool collide(s8 row, s8 column);

  __attribute__((noinline)) void update_bitmask();

  __attribute__((noinline)) void update_shadow();

  // returns number of filled lines aftter blocks were frozen, or -1 if
  // polyomino didn't fit
  __attribute__((noinline)) s8 freeze_blocks();
};
