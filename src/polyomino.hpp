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

  enum class SpawnState {
    WaitToPushPreview,
    OpenToPushPreview,
    PreviewFliesUp,
    WaitToSpawn,
    SpawnAndPrepareToSpit,
    SpitNewPreview,
  };

  // For each of 4 speed tiers, tells how many frames each spawn state lasts;
  // usually spawn states do their main "thing" on their first frame
  static constexpr u8 spawn_state_frames[4][6] = {
      {12, 20, 4, 32, 20, 1},
      {9, 15, 3, 24, 15, 1},
      {6, 10, 2, 16, 10, 1},
      {3, 5, 1, 8, 5, 3},
  };

  u8 grounded_timer;
  State state;
  SpawnState spawn_state;
  u8 spawn_state_timer; // goes up to spawn_state_frames[tier][state]
  u8 spawn_speed_tier;  // goes from 0 to 3 as the game progresses

  Polyomino(Board &board);

  void spawn();

  void handle_input(u8 pressed, u8 held);

  void freezing_handler(bool &blocks_placed, bool &failed_to_place,
                        u8 &lines_cleared);

  void spawn_update();
  void update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
              u8 &lines_filled);

  void banked_render();

  void render(int y_scroll);

  void outside_render(int y_scroll);

  void render_next();

  bool collide(s8 row, s8 column);

  void update_bitmask();
  void update_shadow();

  // returns number of filled lines aftter blocks were frozen, or -1 if
  // polyomino didn't fit
  s8 freeze_blocks();
};
