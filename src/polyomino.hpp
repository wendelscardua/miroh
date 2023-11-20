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

  static Bag<u8, NUM_POLYOMINOS> pieces;

  Board &board;
  const PolyominoDef *definition;
  const PolyominoDef *next;
  s8 row;
  s8 column;
  u16 drop_timer;
  s8 move_timer;
  Direction movement_direction;
  s8 shadow_row;
  s8 left_limit;
  s8 right_limit;
  std::array<u16, 4> bitmask;

  bool able_to_kick(auto kick_deltas);

public:
  enum class State {
    Inactive,
    Active,
  };
  u8 grounded_timer;
  State state;
  Polyomino(Board &board);

  void spawn();

  void handle_input(u8 pressed, u8 held);

  void freezing_handler(bool &blocks_placed, bool &failed_to_place,
                        u8 &lines_cleared);
  void update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
              u8 &lines_filled);

  void banked_render();

  void render(int y_scroll);

  void outside_render(int y_scroll);

  void render_next();

  bool collide(Board &board, s8 row);
  bool collide(Board &board, s8 row, s8 column);

  void update_bitmask();
  void update_shadow();

  // returns number of filled lines aftter blocks were frozen, or -1 if
  // polyomino didn't fit
  s8 freeze_blocks();
};
