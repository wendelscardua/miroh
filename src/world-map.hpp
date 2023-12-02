#pragma once

#include "board.hpp"
#include "common.hpp"
#include <neslib.h>

class WorldMap {
public:
  static constexpr u8 BANK = 2;

  Board &board;

  bool available_stages[NUM_STAGES];

  u8 ending_frame_counter;

  WorldMap(Board &board);
  ~WorldMap();
  void render_sprites();
  void stage_change(Stage new_stage);
  void loop();
  void tick_ending();
};
